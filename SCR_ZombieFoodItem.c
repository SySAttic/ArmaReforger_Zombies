class SCR_ZombieFoodItem : SCR_ConsumableItemComponent
{
    [Attribute("25.0", UIWidgets.Auto, "Amount of hunger restored when consumed")]
    protected float m_fHungerValue;
    
    [Attribute("5.0", UIWidgets.Auto, "Amount of health restored when consumed")]
    protected float m_fHealthValue;
    
    [Attribute("2.0", UIWidgets.Auto, "Time in seconds to consume this item")]
    protected float m_fConsumptionTime;
    
    [Attribute("", UIWidgets.Auto, "Sound to play when consuming")]
    protected string m_sConsumptionSound;
    
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
            
        // Check if hunger is not already full
        if (survivalComp.GetHunger() >= 100.0)
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
        
        // Apply hunger restoration
        SCR_ZombieSurvivalComponent survivalComp = SCR_ZombieSurvivalComponent.Cast(user.FindComponent(SCR_ZombieSurvivalComponent));
        if (survivalComp)
        {
            survivalComp.AddHunger(m_fHungerValue);
            
            // Show feedback
            ShowConsumptionResult(user, "Hunger restored: +" + m_fHungerValue.ToString());
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
        // This would integrate with your UI system
        PlayerController playerController = GetGame().GetPlayerController();
        if (playerController && playerController.GetControlledEntity() == user)
        {
            // Show UI feedback that consumption is in progress
            SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
            if (hintManager)
            {
                hintManager.ShowCustomHint("Consuming food...", "CONSUMING", 3.0);
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
}
