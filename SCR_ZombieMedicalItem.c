class SCR_ZombieMedicalItem : SCR_ConsumableItemComponent
{
    [Attribute("25.0", UIWidgets.Auto, "Amount of health restored")]
    protected float m_fHealthValue;
    
    [Attribute("0", UIWidgets.CheckBox, "Whether this item cures infection")]
    protected bool m_bCuresInfection;
    
    [Attribute("0.0", UIWidgets.Auto, "Amount of infection reduced (0-100)")]
    protected float m_fInfectionReduction;
    
    [Attribute("3.0", UIWidgets.Auto, "Time in seconds to use this item")]
    protected float m_fUsageTime;
    
    [Attribute("", UIWidgets.Auto, "Sound to play when using")]
    protected string m_sUsageSound;
    
    [Attribute("0", UIWidgets.CheckBox, "Whether this item heals bleeding")]
    protected bool m_bHealsBleedng;
    
    [Attribute("0", UIWidgets.CheckBox, "Whether this item provides pain relief")]
    protected bool m_bProvidesPainRelief;
    
    [Attribute("60.0", UIWidgets.Auto, "Duration of pain relief in seconds")]
    protected float m_fPainReliefDuration;
    
    [Attribute("5.0", UIWidgets.Auto, "Stamina restoration amount")]
    protected float m_fStaminaRestoration;
    
    protected bool m_bIsUsing;
    protected float m_fUsageStartTime;
    
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        m_bIsUsing = false;
    }
    
    override bool CanConsume(IEntity user)
    {
        if (!super.CanConsume(user))
            return false;
            
        // Check if not already using
        if (m_bIsUsing)
            return false;
            
        // Check if user actually needs medical attention
        DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(user.FindComponent(DamageManagerComponent));
        if (!dmgMgr)
            return false;
            
        // Allow use if health is not full or if there's infection
        bool needsHealing = dmgMgr.GetHealth() < dmgMgr.GetHealthMax();
        bool hasInfection = false;
        
        SCR_ZombieInfectionComponent infectionComp = SCR_ZombieInfectionComponent.Cast(user.FindComponent(SCR_ZombieInfectionComponent));
        if (infectionComp)
        {
            hasInfection = infectionComp.GetInfectionLevel() > 0;
        }
        
        return needsHealing || hasInfection;
    }
    
    override void StartConsumption(IEntity user)
    {
        if (!CanConsume(user))
            return;
            
        m_bIsUsing = true;
        m_fUsageStartTime = GetGame().GetWorld().GetWorldTime();
        
        // Play usage sound
        if (m_sUsageSound != "")
        {
            AudioComponent audio = AudioComponent.Cast(user.FindComponent(AudioComponent));
            if (audio)
                audio.PlaySound(m_sUsageSound);
        }
        
        // Start usage timer
        GetGame().GetCallqueue().CallLater(CompleteUsage, m_fUsageTime * 1000, false, user);
        
        // Show usage UI/animation if needed
        ShowUsageFeedback(user);
    }
    
    protected void CompleteUsage(IEntity user)
    {
        if (!m_bIsUsing)
            return;
            
        OnConsume(user);
        m_bIsUsing = false;
    }
    
    override void OnConsume(IEntity user)
    {
        super.OnConsume(user);
        
        // Apply health restoration
        if (m_fHealthValue > 0)
        {
            DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(user.FindComponent(DamageManagerComponent));
            if (dmgMgr)
            {
                float currentHealth = dmgMgr.GetHealth();
                float newHealth = Math.Min(dmgMgr.GetHealthMax(), currentHealth + m_fHealthValue);
                dmgMgr.SetHealth(newHealth);
                
                ShowUsageResult(user, "Health restored: +" + m_fHealthValue.ToString());
            }
        }
        
        // Handle infection effects
        SCR_ZombieInfectionComponent infectionComp = SCR_ZombieInfectionComponent.Cast(user.FindComponent(SCR_ZombieInfectionComponent));
        if (infectionComp)
        {
            if (m_bCuresInfection)
            {
                infectionComp.Cure();
                ShowUsageResult(user, "Infection cured!");
            }
            else if (m_fInfectionReduction > 0)
            {
                float currentInfection = infectionComp.GetInfectionLevel();
                infectionComp.AddInfection(-m_fInfectionReduction); // Negative to reduce
                ShowUsageResult(user, "Infection reduced: -" + m_fInfectionReduction.ToString());
            }
        }
        
        // Handle bleeding
        if (m_bHealsBleedng)
        {
            // Stop bleeding effects
            DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(user.FindComponent(DamageManagerComponent));
            if (dmgMgr)
            {
                // Clear bleeding damage over time effects
                dmgMgr.ClearDamageOverTime(EDamageType.BLEEDING);
                ShowUsageResult(user, "Bleeding stopped!");
            }
        }
        
        // Handle pain relief
        if (m_bProvidesPainRelief)
        {
            ApplyPainRelief(user);
            ShowUsageResult(user, "Pain relief applied for " + (m_fPainReliefDuration / 60).ToString() + " minutes");
        }
        
        // Restore stamina
        if (m_fStaminaRestoration > 0)
        {
            SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(user.FindComponent(SCR_CharacterControllerComponent));
            if (controller)
            {
                float currentStamina = controller.GetStamina();
                float maxStamina = controller.GetMaxStamina();
                float newStamina = Math.Min(maxStamina, currentStamina + m_fStaminaRestoration);
                controller.SetStamina(newStamina);
                
                ShowUsageResult(user, "Stamina restored: +" + m_fStaminaRestoration.ToString());
            }
        }
        
        // Remove the item after use
        SCR_InventoryItemComponent itemComp = SCR_InventoryItemComponent.Cast(GetOwner().FindComponent(SCR_InventoryItemComponent));
        if (itemComp)
        {
            InventoryItemComponent inventoryComp = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
            if (inventoryComp)
            {
                InventoryStorageManagerComponent storageManager = inventoryComp.GetStorageManager();
                if (storageManager)
                {
                    storageManager.TryRemoveItemFromInventory(GetOwner());
                }
            }
        }
    }
    
    protected void ApplyPainRelief(IEntity user)
    {
        // Create a temporary pain relief component
        SCR_ZombiePainReliefComponent painRelief = SCR_ZombiePainReliefComponent.Cast(user.FindComponent(SCR_ZombiePainReliefComponent));
        if (!painRelief)
        {
            // Add pain relief component if it doesn't exist
            painRelief = new SCR_ZombiePainReliefComponent();
            user.AddComponent(painRelief);
        }
        
        // Apply pain relief effect
        painRelief.ApplyPainRelief(m_fPainReliefDuration);
    }
    
    protected void ShowUsageFeedback(IEntity user)
    {
        // Show usage in progress feedback
        PlayerController playerController = GetGame().GetPlayerController();
        if (playerController && playerController.GetControlledEntity() == user)
        {
            SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
            if (hintManager)
            {
                hintManager.ShowCustomHint("Using medical item...", "USING_MEDICAL", 3.0);
            }
        }
    }
    
    protected void ShowUsageResult(IEntity user, string message)
    {
        // Show usage result feedback
        PlayerController playerController = GetGame().GetPlayerController();
        if (playerController && playerController.GetControlledEntity() == user)
        {
            SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
            if (hintManager)
            {
                hintManager.ShowCustomHint(message, "MEDICAL_RESULT", 2.0);
            }
        }
    }
    
    void CancelUsage()
    {
        if (m_bIsUsing)
        {
            m_bIsUsing = false;
            GetGame().GetCallqueue().Remove(CompleteUsage);
        }
    }
    
    bool IsUsing()
    {
        return m_bIsUsing;
    }
    
    float GetUsageProgress()
    {
        if (!m_bIsUsing)
            return 0.0;
            
        float elapsed = GetGame().GetWorld().GetWorldTime() - m_fUsageStartTime;
        return Math.Clamp(elapsed / m_fUsageTime, 0.0, 1.0);
    }
}

// Additional component for pain relief effects
class SCR_ZombiePainReliefComponent : ScriptComponent
{
    protected float m_fPainReliefEndTime;
    protected bool m_bHasPainRelief;
    
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        m_bHasPainRelief = false;
    }
    
    void ApplyPainRelief(float duration)
    {
        m_bHasPainRelief = true;
        m_fPainReliefEndTime = GetGame().GetWorld().GetWorldTime() + duration;
        
        // Start checking for expiration
        GetGame().GetCallqueue().CallLater(CheckPainReliefExpiry, 1000, true);
        
        // Apply pain relief effects
        ApplyPainReliefEffects();
    }
    
    protected void CheckPainReliefExpiry()
    {
        if (!m_bHasPainRelief)
        {
            GetGame().GetCallqueue().Remove(CheckPainReliefExpiry);
            return;
        }
        
        if (GetGame().GetWorld().GetWorldTime() >= m_fPainReliefEndTime)
        {
            RemovePainRelief();
        }
    }
    
    protected void ApplyPainReliefEffects()
    {
        // Reduce visual pain effects
        SCR_PostProcessEffectsComponent ppEffects = SCR_PostProcessEffectsComponent.Cast(GetOwner().FindComponent(SCR_PostProcessEffectsComponent));
        if (ppEffects)
        {
            // Reduce red tint and other pain effects
            ppEffects.SetColorGradingTint(Color.FromRGBA(255, 255, 255, 255));
        }
        
        // Improve movement
        SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(GetOwner().FindComponent(SCR_CharacterControllerComponent));
        if (controller)
        {
            // Increase movement speed slightly
            controller.SetSpeedMultiplier(1.1);
        }
    }
    
    protected void RemovePainRelief()
    {
        m_bHasPainRelief = false;
        GetGame().GetCallqueue().Remove(CheckPainReliefExpiry);
        
        // Reset effects
        SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(GetOwner().FindComponent(SCR_CharacterControllerComponent));
        if (controller)
        {
            controller.SetSpeedMultiplier(1.0);
        }
    }
    
    bool HasPainRelief()
    {
        return m_bHasPainRelief;
    }
    
    float GetRemainingTime()
    {
        if (!m_bHasPainRelief)
            return 0.0;
            
        return Math.Max(0.0, m_fPainReliefEndTime - GetGame().GetWorld().GetWorldTime());
    }
}
