class SCR_ZombieWaterItem : SCR_ConsumableItemComponent
{
    [Attribute("30.0", UIWidgets.Auto, "Amount of thirst restored when consumed")]
    protected float m_fThirstValue;
    
    [Attribute("2.0", UIWidgets.Auto, "Amount of health restored when consumed")]
    protected float m_fHealthValue;
    
    [Attribute("1.5", UIWidgets.Auto, "Time in seconds to consume this item")]
    protected float m_fConsumptionTime;
    
    [Attribute("", UIWidgets.Auto, "Sound to play when consuming")]
    protected string m_sConsumptionSound;
    
    [Attribute("0", UIWidgets.CheckBox, "Whether this water is contaminated")]
    protected bool m_bIsContaminated;
    
    [Attribute("10.0", UIWidgets.Auto, "Infection amount if contaminated")]
    protected float m_fContaminationLevel;
    
    protected bool m_bIsConsuming;
    protected float m_fConsumptionStartTime;
    
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        m_bIsConsuming = false;
    }
    
    override bool CanConsume(IEntity user)
    {
        if (!super.CanConsume(user))
            return false;
            
        // Check if user has survival component
        SCR_ZombieSurvivalComponent survivalComp = SCR_ZombieSurvivalComponent.Cast(user.FindComponent(SCR_ZombieSurvivalComponent));
        if (!survivalComp)
            return false;
            
        // Check if thirst is not already full
        if (survivalComp.GetThirst() >= 100.0)
            return false;
            
        // Check if not already consuming
        if (m_bIsConsuming)
            return false;
            
        return true;
    }
    
    override void StartConsumption(IEntity user)
    {
        if (!CanConsume(user))
            return;
            
        m_bIsConsuming = true;
        m_fConsumptionStartTime = GetGame().GetWorld().GetWorldTime();
        
        // Play consumption sound
        if (m_sConsumptionSound != "")
        {
            AudioComponent audio = AudioComponent.Cast(user.FindComponent(AudioComponent));
            if (audio)
                audio.PlaySound(m_sConsumptionSound);
        }
        
        // Start consumption timer
        GetGame().GetCallqueue().CallLater(CompleteConsumption, m_fConsumptionTime * 1000, false, user);
        
        // Show consumption UI/animation if needed
        ShowConsumptionFeedback(user);
    }
    
    protected void CompleteConsumption(IEntity user)
    {
        if (!m_bIsConsuming)
            return;
            
        OnConsume(user);
        m_bIsConsuming = false;
    }
    
    override void OnConsume(IEntity user)
    {
        super.OnConsume(user);
        
        // Apply thirst restoration
        SCR_ZombieSurvivalComponent survivalComp = SCR_ZombieSurvivalComponent.Cast(user.FindComponent(SCR_ZombieSurvivalComponent));
        if (survivalComp)
        {
            survivalComp.AddThirst(m_fThirstValue);
            
            // Show feedback
            ShowConsumptionResult(user, "Thirst restored: +" + m_fThirstValue.ToString());
        }
        
        // Apply health restoration if any
        if (m_fHealthValue > 0)
        {
            DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(user.FindComponent(DamageManagerComponent));
            if (dmgMgr)
            {
                float currentHealth = dmgMgr.GetHealth();
                float newHealth = Math.Min(dmgMgr.GetHealthMax(), currentHealth + m_fHealthValue);
                dmgMgr.SetHealth(newHealth);
                
                ShowConsumptionResult(user, "Health restored: +" + m_fHealthValue.ToString());
            }
        }
        
        // Handle contamination if water is dirty
        if (m_bIsContaminated && m_fContaminationLevel > 0)
        {
            SCR_ZombieInfectionComponent infectionComp = SCR_ZombieInfectionComponent.Cast(user.FindComponent(SCR_ZombieInfectionComponent));
            if (infectionComp)
            {
                infectionComp.AddInfection(m_fContaminationLevel);
                ShowConsumptionResult(user, "You feel sick from the contaminated water!");
            }
            else
            {
                // If no infection system, apply direct damage
                DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(user.FindComponent(DamageManagerComponent));
                if (dmgMgr)
                {
                    DamageParams params = new DamageParams();
                    params.Damage = m_fContaminationLevel * 0.5; // Convert infection to damage
                    params.DamageType = EDamageType.BIOLOGICAL;
                    
                    dmgMgr.InflictDamage(params);
                    ShowConsumptionResult(user, "The contaminated water makes you ill!");
                }
            }
        }
        
        // Remove the item after consumption
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
    
    protected void ShowConsumptionFeedback(IEntity user)
    {
        // Show consumption in progress feedback
        PlayerController playerController = GetGame().GetPlayerController();
        if (playerController && playerController.GetControlledEntity() == user)
        {
            SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
            if (hintManager)
            {
                string message = m_bIsContaminated ? "Drinking contaminated water..." : "Drinking water...";
                hintManager.ShowCustomHint(message, "CONSUMING", 3.0);
            }
        }
    }
    
    protected void ShowConsumptionResult(IEntity user, string message)
    {
        // Show consumption result feedback
        PlayerController playerController = GetGame().GetPlayerController();
        if (playerController && playerController.GetControlledEntity() == user)
        {
            SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
            if (hintManager)
            {
                hintManager.ShowCustomHint(message, "CONSUMPTION_RESULT", 2.0);
            }
        }
    }
    
    void CancelConsumption()
    {
        if (m_bIsConsuming)
        {
            m_bIsConsuming = false;
            GetGame().GetCallqueue().Remove(CompleteConsumption);
        }
    }
    
    bool IsConsuming()
    {
        return m_bIsConsuming;
    }
    
    float GetConsumptionProgress()
    {
        if (!m_bIsConsuming)
            return 0.0;
            
        float elapsed = GetGame().GetWorld().GetWorldTime() - m_fConsumptionStartTime;
        return Math.Clamp(elapsed / m_fConsumptionTime, 0.0, 1.0);
    }
    
    bool IsContaminated()
    {
        return m_bIsContaminated;
    }
    
    void SetContaminated(bool contaminated, float contaminationLevel = 10.0)
    {
        m_bIsContaminated = contaminated;
        if (contaminated)
            m_fContaminationLevel = contaminationLevel;
    }
}
