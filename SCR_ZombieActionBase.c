class SCR_ZombieActionBase : AIActionBase
{
    [Attribute("2.0", UIWidgets.Auto, "Movement speed when wandering")]
    protected float m_fWanderSpeed;
    
    [Attribute("3.5", UIWidgets.Auto, "Movement speed when chasing")]
    protected float m_fChaseSpeed;
    
    [Attribute("15.0", UIWidgets.Auto, "Attack damage")]
    protected float m_fAttackDamage;
    
    [Attribute("1.5", UIWidgets.Auto, "Attack cooldown in seconds")]
    protected float m_fAttackCooldown;
    
    [Attribute("30.0", UIWidgets.Auto, "Detection range in meters")]
    protected float m_fDetectionRange;
    
    [Attribute("10.0", UIWidgets.Auto, "How often zombie should wander to a new position (seconds)")]
    protected float m_fWanderTime;
    
    protected float m_fLastAttackTime;
    protected float m_fLastWanderTime;
    protected IEntity m_TargetEntity;
    protected bool m_bIsChasing;
    protected bool m_bHasLostTarget;
    protected vector m_vLastKnownTargetPosition;
    protected float m_fLostTargetTime;
    
    // Initial target entity (optional)
    IEntity m_InitialTarget;
    
    override void OnActivate(AIAgent agent)
    {
        super.OnActivate(agent);
        
        // Set initial state to wandering
        m_bIsChasing = false;
        m_bHasLostTarget = false;
        m_fLastAttackTime = 0;
        m_fLastWanderTime = 0;
        m_fLostTargetTime = 0;
        
        // If we have an initial target, set it
        if (m_InitialTarget)
        {
            m_TargetEntity = m_InitialTarget;
            m_bIsChasing = true;
            agent.SetMaxSpeed(m_fChaseSpeed);
        }
        
        // Start zombie behavior
        GetGame().GetCallqueue().CallLater(ZombieUpdate, 500, true, agent);
    }
    
    override void OnDeactivate(AIAgent agent)
    {
        super.OnDeactivate(agent);
        GetGame().GetCallqueue().Remove(ZombieUpdate);
    }
    
    protected void ZombieUpdate(AIAgent agent)
    {
        IEntity entity = agent.GetControlledEntity();
        if (!entity)
            return;
            
        // Check if our current target is still valid
        if (m_TargetEntity && !m_bHasLostTarget)
        {
            if (!IsEntityValid(m_TargetEntity))
            {
                // Target no longer valid, switch to wandering
                m_TargetEntity = null;
                m_bIsChasing = false;
                agent.SetMaxSpeed(m_fWanderSpeed);
            }
            else
            {
                // Check if we still have line of sight
                if (!HasLineOfSight(entity, m_TargetEntity))
                {
                    // Lost line of sight, but remember the position
                    if (!m_bHasLostTarget)
                    {
                        m_bHasLostTarget = true;
                        m_vLastKnownTargetPosition = m_TargetEntity.GetOrigin();
                        m_fLostTargetTime = GetGame().GetWorld().GetWorldTime();
                    }
                }
                else
                {
                    // We have line of sight, update last known position
                    m_bHasLostTarget = false;
                    m_vLastKnownTargetPosition = m_TargetEntity.GetOrigin();
                }
            }
        }
        
        // If we've lost our target, look for a new one
        if (!m_TargetEntity)
        {
            // Check for nearby players
            m_TargetEntity = FindNearestPlayer(entity);
            
            if (m_TargetEntity)
            {
                // Player found - chase mode
                if (!m_bIsChasing)
                {
                    // Transition to chase mode
                    m_bIsChasing = true;
                    m_bHasLostTarget = false;
                    agent.SetMaxSpeed(m_fChaseSpeed);
                    
                    // Play alert sound
                    AudioComponent audio = AudioComponent.Cast(entity.FindComponent(AudioComponent));
                    if (audio)
                        audio.PlaySound("SOUND_ZOMBIE_ALERT");
                }
                
                // Move toward target
                NavigateToEntity(agent, m_TargetEntity);
            }
            else if (m_bIsChasing)
            {
                // Lost target completely - return to wandering
                m_bIsChasing = false;
                m_bHasLostTarget = false;
                agent.SetMaxSpeed(m_fWanderSpeed);
            }
        }
        
        // Handle chase behavior
        if (m_bIsChasing)
        {
            // If we've lost sight but remember the position
            if (m_bHasLostTarget)
            {
                // Move to last known position
                NavigateToPosition(agent, m_vLastKnownTargetPosition);
                
                // Check if we've reached the last known position
                float distToLastPos = vector.Distance(entity.GetOrigin(), m_vLastKnownTargetPosition);
                if (distToLastPos < 2.0)
                {
                    // We've reached the last known position, give up after a time
                    float timeSinceLost = GetGame().GetWorld().GetWorldTime() - m_fLostTargetTime;
                    if (timeSinceLost > 15.0)
                    {
                        // Give up and return to wandering
                        m_TargetEntity = null;
                        m_bIsChasing = false;
                        m_bHasLostTarget = false;
                        agent.SetMaxSpeed(m_fWanderSpeed);
                    }
                }
            }
            else if (m_TargetEntity)
            {
                // Move toward target
                NavigateToEntity(agent, m_TargetEntity);
                
                // Check if close enough to attack
                float distance = vector.Distance(entity.GetOrigin(), m_TargetEntity.GetOrigin());
                if (distance < 2.0) // Attack range
                {
                    float currentTime = GetGame().GetWorld().GetWorldTime();
                    if (currentTime - m_fLastAttackTime > m_fAttackCooldown)
                    {
                        // Perform attack
                        AttackTarget(agent, m_TargetEntity);
                        m_fLastAttackTime = currentTime;
                    }
                }
            }
        }
        else
        {
            // Wandering behavior
            float currentTime = GetGame().GetWorld().GetWorldTime();
            if (currentTime - m_fLastWanderTime > m_fWanderTime || !agent.IsNavigating())
            {
                // Pick new wander point
                vector wanderPoint = GetRandomWanderPoint(entity);
                NavigateToPosition(agent, wanderPoint);
                m_fLastWanderTime = currentTime;
            }
        }
    }
    
    protected IEntity FindNearestPlayer(IEntity zombieEntity)
    {
        // Find closest player within detection range
        vector zombiePos = zombieEntity.GetOrigin();
        float closestDist = m_fDetectionRange;
        IEntity closestPlayer = null;
        
        array<IEntity> players = new array<IEntity>();
        GetGame().GetPlayerManager().GetPlayers(players);
        
        foreach (IEntity player : players)
        {
            if (!IsEntityValid(player))
                continue;
                
            float dist = vector.Distance(zombiePos, player.GetOrigin());
            if (dist < closestDist)
            {
                // Line of sight check
                if (HasLineOfSight(zombieEntity, player))
                {
                    closestDist = dist;
                    closestPlayer = player;
                }
            }
        }
        
        return closestPlayer;
    }
    
    protected bool HasLineOfSight(IEntity source, IEntity target)
    {
        vector sourcePos = source.GetOrigin();
        sourcePos[1] = sourcePos[1] + 1.7; // Eye height
        
        vector targetPos = target.GetOrigin();
        targetPos[1] = targetPos[1] + 1.7; // Eye height
        
        // Perform raytrace between entities
        auto world = GetGame().GetWorld();
        TraceParam trace = new TraceParam();
        trace.Start = sourcePos;
        trace.End = targetPos;
        trace.LayerMask = EPhysicsLayerMask.BUILDINGS | EPhysicsLayerMask.TERRAIN;
        trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
        
        float fraction;
        if (world.TraceMove(trace, fraction) && fraction < 1.0)
            return false; // Something blocking the line of sight
            
        return true;
    }
    
    protected void AttackTarget(AIAgent agent, IEntity target)
    {
        IEntity entity = agent.GetControlledEntity();
        if (!entity || !target)
            return;
            
        // Play attack animation
        AnimationComponent anim = AnimationComponent.Cast(entity.FindComponent(AnimationComponent));
        if (anim)
            anim.PlayAnimation("ZombieAttack");
        
        // Apply damage to target
        DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(target.FindComponent(DamageManagerComponent));
        if (dmgMgr)
        {
            DamageParams params = new DamageParams();
            params.Damage = m_fAttackDamage;
            params.DamageType = EDamageType.MELEE;
            params.Instigator = entity;
            
            dmgMgr.InflictDamage(params);
            
            // Apply infection status if implemented
            SCR_CharacterControllerComponent targetCtrl = SCR_CharacterControllerComponent.Cast(target.FindComponent(SCR_CharacterControllerComponent));
            if (targetCtrl)
            {
                // infection system,
                // ApplyInfectionStatus(targetCtrl);
            }
        }
        
        // Play attack sound
        AudioComponent audio = AudioComponent.Cast(entity.FindComponent(AudioComponent));
        if (audio)
            audio.PlaySound("SOUND_ZOMBIE_ATTACK");
    }
    
    protected vector GetRandomWanderPoint(IEntity entity)
    {
        // Generate random point within reasonable distance
        vector currentPos = entity.GetOrigin();
        float angle = Math.RandomFloat(0, Math.PI2);
        float distance = Math.RandomFloat(5, 15);
        
        vector offset = Vector(Math.Cos(angle) * distance, 0, Math.Sin(angle) * distance);
        vector targetPos = currentPos + offset;
        
        // Get proper terrain position
        WorldEntity world = GetGame().GetWorld();
        if (world)
        {
            vector start = targetPos;
            start[1] = start[1] + 50.0;
            
            vector end = targetPos;
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
                targetPos = hitPos;
                targetPos[1] = targetPos[1] + 0.1;
            }
        }
        
        return targetPos;
    }
    
    protected void NavigateToPosition(AIAgent agent, vector position)
    {
        AINavigationComponent navComp = AINavigationComponent.Cast(agent.GetControlledEntity().FindComponent(AINavigationComponent));
        if (navComp)
        {
            navComp.SetTarget(position);
        }
    }
    
    protected void NavigateToEntity(AIAgent agent, IEntity target)
    {
        if (!target)
            return;
            
        NavigateToPosition(agent, target.GetOrigin());
    }
    
    protected bool IsEntityValid(IEntity entity)
    {
        if (!entity)
            return false;
            
        DamageManagerComponent dmgComp = DamageManagerComponent.Cast(entity.FindComponent(DamageManagerComponent));
        if (dmgComp && dmgComp.GetState() == EDamageState.DESTROYED)
            return false;
            
        return true;
    }
}
