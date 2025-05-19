class SCR_ZombieSurvivalComponent : ScriptComponent
{
    [Attribute("100", UIWidgets.Auto, "Current hunger level (0-100)")]
    protected float m_fHunger;
    
    [Attribute("100", UIWidgets.Auto, "Current thirst level (0-100)")]
    protected float m_fThirst;
    
    [Attribute("0.1", UIWidgets.Auto, "Rate at which hunger decreases per second")]
    protected float m_fHungerDecreaseRate;
    
    [Attribute("0.15", UIWidgets.Auto, "Rate at which thirst decreases per second")]
    protected float m_fThirstDecreaseRate;
    
    [Attribute("20.0", UIWidgets.Auto, "Hunger level at which damage starts")]
    protected float m_fHungerDamageThreshold;
    
    [Attribute("15.0", UIWidgets.Auto, "Thirst level at which damage starts")]
    protected float m_fThirstDamageThreshold;
    
    [Attribute("1.0", UIWidgets.Auto, "Health damage per update when starving/dehydrated")]
    protected float m_fStarvationDamage;
    
    [Attribute("60.0", UIWidgets.Auto, "Interval in seconds between starvation damage")]
    protected float m_fStarvationDamageInterval;
    
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        
        if (owner.FindComponent(SCR_CharacterControllerComponent))
        {
            GetGame().GetCallqueue().CallLater(UpdateSurvival, 1000, true);
            GetGame().GetCallqueue().CallLater(ApplyStarvationEffects, m_fStarvationDamageInterval * 1000, true);
        }
    }
    
    override void OnDelete(IEntity owner)
    {
        GetGame().GetCallqueue().Remove(UpdateSurvival);
        GetGame().GetCallqueue().Remove(ApplyStarvationEffects);
        super.OnDelete(owner);
    }
    
    void UpdateSurvival()
    {
        // Decrease hunger and thirst over time
        m_fHunger -= m_fHungerDecreaseRate;
        m_fThirst -= m_fThirstDecreaseRate;
        
        // Clamp values
        if (m_fHunger < 0)
            m_fHunger = 0;
            
        if (m_fThirst < 0)
            m_fThirst = 0;
            
        // Update UI if needed
        UpdateSurvivalUI();
    }
    
    void ApplyStarvationEffects()
    {
        if (m_fHunger <= m_fHungerDamageThreshold || m_fThirst <= m_fThirstDamageThreshold)
        {
            // Apply damage for starvation/dehydration
            DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));
            if (dmgMgr)
            {
                DamageParams params = new DamageParams();
                params.Damage = m_fStarvationDamage;
                params.DamageType = EDamageType.BIOLOGICAL;
                
                dmgMgr.InflictDamage(params);
            }
            
            // Apply visual effects
            ApplyStarvationVisualEffects();
        }
    }
    
    protected void ApplyStarvationVisualEffects()
    {
        SCR_PostProcessEffectsComponent ppEffects = SCR_PostProcessEffectsComponent.Cast(GetOwner().FindComponent(SCR_PostProcessEffectsComponent));
        if (ppEffects)
        {
            float hungerEffect = 1.0 - (m_fHunger / 100.0);
            float thirstEffect = 1.0 - (m_fThirst / 100.0);
            float combinedEffect = Math.Max(hungerEffect, thirstEffect);
            
            ppEffects.SetVignetteIntensity(0.3 * combinedEffect);
            ppEffects.SetDesaturation(0.4 * combinedEffect);
        }
    }
    
    protected void UpdateSurvivalUI()
    {
        // Update UI elements
        // This would connect to your HUD system
    }
    
    float GetHunger()
    {
        return m_fHunger;
    }
    
    float GetThirst()
    {
        return m_fThirst;
    }
    
    void AddHunger(float amount)
    {
        m_fHunger += amount;
        
        if (m_fHunger > 100)
            m_fHunger = 100;
    }
    
    void AddThirst(float amount)
    {
        m_fThirst += amount;
        
        if (m_fThirst > 100)
            m_fThirst = 100;
    }
}
