class ZombieManager : ScriptComponent
{
    [Attribute("20", UIWidgets.Auto, "Maximum number of zombies to spawn per player")]
    protected int m_iMaxZombiesPerPlayer;
    
    [Attribute("150", UIWidgets.Auto, "Spawn radius around player in meters")]
    protected float m_fSpawnRadius;
    
    [Attribute("50", UIWidgets.Auto, "Minimum spawn distance from player in meters")]
    protected float m_fMinSpawnDistance;
    
    [Attribute("200", UIWidgets.Auto, "Despawn distance from player in meters")]
    protected float m_fDespawnDistance;
    
    [Attribute("$ZOMBIE_MODEL_PATH", UIWidgets.ResourceNamePicker, "Prefab resource for zombie entity")]
    protected ResourceName m_ZombiePrefab;
    
    [Attribute("10", UIWidgets.Auto, "Spawn attempt interval in seconds")]
    protected float m_fSpawnInterval;
    
    [Attribute("5", UIWidgets.Auto, "Maximum spawn attempts per interval")]
    protected int m_iMaxSpawnAttempts;
    
    [Attribute("30", UIWidgets.Auto, "Management update interval in seconds")]
    protected float m_fManagementInterval;
    
    protected ref array<IEntity> m_aSpawnedZombies = new array<IEntity>();
    protected ref map<IEntity, IEntity> m_mZombieTargets = new map<IEntity, IEntity>();
    
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        
        GetGame().GetCallqueue().CallLater(SpawnZombieUpdate, m_fSpawnInterval * 1000, true);
        GetGame().GetCallqueue().CallLater(ManagementUpdate, m_fManagementInterval * 1000, true);
    }
    
    override void OnDelete(IEntity owner)
    {
        GetGame().GetCallqueue().Remove(SpawnZombieUpdate);
        GetGame().GetCallqueue().Remove(ManagementUpdate);
        
        ClearAllZombies();
        super.OnDelete(owner);
    }
    
    void SpawnZombieUpdate()
    {
        array<IEntity> players = new array<IEntity>();
        GetGame().GetPlayerManager().GetPlayers(players);
        
        if (players.Count() == 0)
            return;
            
        int totalMaxZombies = players.Count() * m_iMaxZombiesPerPlayer;
        
        if (m_aSpawnedZombies.Count() >= totalMaxZombies)
            return;
            
        int zombiesToSpawn = Math.Min(m_iMaxSpawnAttempts, totalMaxZombies - m_aSpawnedZombies.Count());
        
        for (int i = 0; i < zombiesToSpawn; i++)
        {
            IEntity randomPlayer = players.GetRandomElement();
            if (!randomPlayer)
                continue;
                
            vector spawnPos = FindZombieSpawnPosition(randomPlayer);
            if (!spawnPos[0] && !spawnPos[1] && !spawnPos[2])
                continue;
                
            SpawnZombie(spawnPos, randomPlayer);
        }
    }
    
    void ManagementUpdate()
    {
        array<IEntity> players = new array<IEntity>();
        GetGame().GetPlayerManager().GetPlayers(players);
        
        if (players.Count() == 0 || m_aSpawnedZombies.Count() == 0)
            return;
        
        for (int i = m_aSpawnedZombies.Count() - 1; i >= 0; i--)
        {
            IEntity zombie = m_aSpawnedZombies[i];
            if (!zombie)
            {
                m_aSpawnedZombies.Remove(i);
                continue;
            }
            
            bool shouldDespawn = true;
            vector zombiePos = zombie.GetOrigin();
            
            foreach (IEntity player : players)
            {
                float distance = vector.Distance(zombiePos, player.GetOrigin());
                if (distance < m_fDespawnDistance)
                {
                    shouldDespawn = false;
                    break;
                }
            }
            
            if (shouldDespawn)
            {
                DeleteZombie(zombie);
                m_aSpawnedZombies.Remove(i);
            }
        }
    }
    
    vector FindZombieSpawnPosition(IEntity nearPlayer)
    {
        vector playerPos = nearPlayer.GetOrigin();
        float angle = Math.RandomFloat(0, Math.PI2);
        float distance = Math.RandomFloat(m_fMinSpawnDistance, m_fSpawnRadius);
        
        vector offset = Vector(Math.Cos(angle) * distance, 0, Math.Sin(angle) * distance);
        vector candidatePos = playerPos + offset;
        
        return DoTerrainCheck(candidatePos);
    }
    
    vector DoTerrainCheck(vector position)
    {
        WorldEntity world = GetGame().GetWorld();
        if (!world)
            return position;
        
        vector start = position;
        start[1] = start[1] + 50.0;
        
        vector end = position;
        end[1] = end[1] - 10.0;
        
        TraceParam trace = new TraceParam();
        trace.Start = start;
        trace.End = end;
        trace.LayerMask = EPhysicsLayerMask.TERRAIN;
        trace.Flags = TraceFlags.WORLD;
        
        float fraction;
        vector hitPos, hitNormal;
        
        if (world.TraceMove(trace, fraction, hitPos, hitNormal))
        {
            position = hitPos;
            position[1] = position[1] + 0.1;
            
            // Check if position is in water
            if (world.GetWaterDepth(position) > 0.3)
            {
                return vector.Zero;
            }
            
            // Check if position is too steep
            if (hitNormal[1] < 0.7) // Roughly 45 degrees
            {
                return vector.Zero;
            }
            
            // Check if position is in a building
            trace = new TraceParam();
            trace.Start = position;
            trace.End = position;
            trace.End[1] = position[1] + 3.0;
            trace.LayerMask = EPhysicsLayerMask.BUILDINGS;
            trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
            
            if (world.TraceMove(trace, fraction))
            {
                return vector.Zero;
            }
            
            return position;
        }
        
        return vector.Zero;
    }
    
    IEntity SpawnZombie(vector position, IEntity nearPlayer)
    {
        Resource zombieRes = Resource.Load(m_ZombiePrefab);
        if (!zombieRes)
            return null;
            
        EntitySpawnParams params = new EntitySpawnParams();
        params.TransformMode = ETransformMode.WORLD;
        params.Transform[3] = position;
        
        IEntity zombieEntity = GetGame().SpawnEntityPrefab(zombieRes, null, params);
        if (!zombieEntity)
            return null;
            
        // Configure zombie AI and components
        ConfigureZombieEntity(zombieEntity, nearPlayer);
        
        // Add to tracking arrays
        m_aSpawnedZombies.Insert(zombieEntity);
        m_mZombieTargets.Insert(zombieEntity, nearPlayer);
        
        return zombieEntity;
    }
    
    void ConfigureZombieEntity(IEntity zombieEntity, IEntity initialTarget)
    {
        // Configure AI components
        AIControlComponent aiControl = AIControlComponent.Cast(zombieEntity.FindComponent(AIControlComponent));
        if (aiControl)
        {
            ConfigureZombieAI(aiControl, initialTarget);
        }
        
        // Configure Character Controller for zombie movement
        CharacterControllerComponent controller = CharacterControllerComponent.Cast(zombieEntity.FindComponent(CharacterControllerComponent));
        if (controller)
        {
            ConfigureZombieMovement(controller);
        }
        
        // Configure animations
        AnimationComponent anim = AnimationComponent.Cast(zombieEntity.FindComponent(AnimationComponent));
        if (anim)
        {
            ConfigureZombieAnimations(anim);
        }
        
        // Configure audio
        AudioComponent audio = AudioComponent.Cast(zombieEntity.FindComponent(AudioComponent));
        if (audio)
        {
            ConfigureZombieAudio(audio);
        }
        
        // Configure damage handling
        DamageManagerComponent damageManager = DamageManagerComponent.Cast(zombieEntity.FindComponent(DamageManagerComponent));
        if (damageManager)
        {
            ConfigureZombieDamageHandling(damageManager);
        }
    }
    
    void ConfigureZombieAI(AIControlComponent aiControl, IEntity initialTarget)
    {
        // Create the zombie AI action
        SCR_ZombieActionBase zombieAction = new SCR_ZombieActionBase();
        zombieAction.m_fWanderSpeed = 1.8;
        zombieAction.m_fChaseSpeed = 3.5;
        zombieAction.m_fAttackDamage = 15.0;
        zombieAction.m_fAttackCooldown = 1.5;
        zombieAction.m_fDetectionRange = 35.0;
        zombieAction.m_InitialTarget = initialTarget;
        
        // Disable standard soldier behavior
        AIAgent agent = aiControl.GetAIAgent();
        if (agent)
        {
            // Get all actions and deactivate them
            array<AIActionBase> existingActions = new array<AIActionBase>();
            agent.GetActions(existingActions);
            
            foreach (AIActionBase action : existingActions)
            {
                agent.DeactivateAction(action);
            }
            
            // Add our custom zombie action
            agent.AddAction(zombieAction);
            agent.ActivateAction(zombieAction);
        }
    }
    
    void ConfigureZombieMovement(CharacterControllerComponent controller)
    {
        // Modify character controller settings for zombie-like movement
        controller.SetWalkSpeed(1.8);
        controller.SetRunSpeed(3.5);
        controller.SetSprintSpeed(4.5);
        controller.SetWalkAimingSpeed(1.8);
        controller.SetRunAimingSpeed(3.5);
        controller.SetSprintAimingSpeed(4.5);
        controller.SetTurnSpeed(100.0); // Slower turning for zombie-like movement
    }
    
    void ConfigureZombieAnimations(AnimationComponent anim)
    {
        // Set zombie-specific animation settings
        anim.SetFloat("movementSpeed", 0.8); // Slower animation speed
        anim.SetBool("isZombie", true);
        
        // Override default animation states
        AnimationSyncedVariablesComponent animSync = AnimationSyncedVariablesComponent.Cast(anim.GetOwner().FindComponent(AnimationSyncedVariablesComponent));
        if (animSync)
        {
            animSync.SetVariable("stance", "zombie");
        }
    }
    
    void ConfigureZombieAudio(AudioComponent audio)
    {
        // Set up zombie sounds
        audio.SetSoundEvent("SOUND_ZOMBIE_IDLE", "ZombieIdle");
        audio.SetSoundEvent("SOUND_ZOMBIE_ALERT", "ZombieAlert");
        audio.SetSoundEvent("SOUND_ZOMBIE_ATTACK", "ZombieAttack");
        audio.SetSoundEvent("SOUND_ZOMBIE_DEATH", "ZombieDeath");
        
        // Start ambient zombie sounds
        GetGame().GetCallqueue().CallLater(PlayRandomZombieSound, Math.RandomFloat(5000, 15000), false, audio);
    }
    
    void PlayRandomZombieSound(AudioComponent audio)
    {
        if (!audio || !audio.GetOwner())
            return;
        
        // Select a random zombie sound
        int soundType = Math.RandomInt(0, 3);
        string soundEvent;
        
        switch (soundType)
        {
            case 0:
                soundEvent = "SOUND_ZOMBIE_IDLE";
                break;
            case 1:
                soundEvent = "SOUND_ZOMBIE_ALERT";
                break;
            case 2:
                soundEvent = "SOUND_ZOMBIE_IDLE";
                break;
        }
        
        audio.PlaySound(soundEvent);
        
        // Schedule next sound
        GetGame().GetCallqueue().CallLater(PlayRandomZombieSound, Math.RandomFloat(8000, 20000), false, audio);
    }
    
    void ConfigureZombieDamageHandling(DamageManagerComponent damageManager)
    {
        // Configure zombie health and damage handling
        damageManager.SetHealthMax(100);
        damageManager.SetHealth(100);
        
        // Increase resistance to certain damage types
        damageManager.SetDamageMultiplier(EDamageType.BULLET, 0.5); // Resistant to bullets
        damageManager.SetDamageMultiplier(EDamageType.EXPLOSION, 0.7); // Resistant to explosions
        
        // Make headshots more effective
        damageManager.SetBodyPartDamageMultiplier("Head", 2.0);
    }
    
    void DeleteZombie(IEntity zombie)
    {
        if (!zombie)
            return;
            
        m_mZombieTargets.Remove(zombie);
        SCR_EntityHelper.DeleteEntityAndChildren(zombie);
    }
    
    void ClearAllZombies()
    {
        foreach (IEntity zombie : m_aSpawnedZombies)
        {
            if (zombie)
                SCR_EntityHelper.DeleteEntityAndChildren(zombie);
        }
        
        m_aSpawnedZombies.Clear();
        m_mZombieTargets.Clear();
    }
}
