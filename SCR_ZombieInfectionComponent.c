class SCR_ZombieInfectionComponent : ScriptComponent
{
    [Attribute("0", UIWidgets.Auto, "Current infection level (0-100)")]
    protected float m_fInfectionLevel;
    
    [Attribute("5.0", UIWidgets.Auto, "Amount of infection from zombie attack")]
    protected float m_fInfectionPerAttack;
    
    [Attribute("0.1", UIWidgets.Auto, "Rate at which infection progresses per second")]
    protected float m_fInfectionProgressRate;
    
    [Attribute("60.0", UIWidgets.Auto, "Update interval for infection effects in seconds")]
    protected float m_fEffectsInterval;
    
    [Attribute("80.0", UIWidgets.Auto, "Infection level at which severe effects begin")]
    protected float m_fSevereEffectsThreshold;
    
    [Attribute("100.0", UIWidgets.Auto, "Infection level at which death occurs")]
    protected float m_fDeathThreshold;
    
    protected bool m_bIsInfected;
    
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        
        if (owner.FindComponent(SCR_CharacterControllerComponent))
        {
            GetGame().GetCallqueue().CallLater(UpdateInfection, 1000, true);
            GetGame().GetCallqueue().CallLater(ApplyInfectionEffects, m_fEffectsInterval * 1000, true);
        }
    }
    
    override void OnDelete(IEntity owner)
    {
        GetGame().GetCallqueue().Remove(UpdateInfection);
        GetGame().GetCallqueue().Remove(ApplyInfectionEffects);
        super.OnDelete(owner);
    }
    
    void UpdateInfection()
    {
        if (m_bIsInfected && m_fInfectionLevel < m_fDeathThreshold)
        {
            m_fInfectionLevel += m_fInfectionProgressRate;
            
            // Check for death threshold
            if (m_fInfectionLevel >= m_fDeathThreshold)
            {
                KillFromInfection();
            }
        }
    }
    
    void ApplyInfectionEffects()
    {
        if (!m_bIsInfected || m_fInfectionLevel <= 0)
            return;
            
        // Get player controller component
        SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(GetOwner().FindComponent(SCR_CharacterControllerComponent));
        if (!controller)
            return;
            
        // Apply effects based on infection level
        if (m_fInfectionLevel > m_fSevereEffectsThreshold)
        {
            // Severe effects
            controller.SetMaxStamina(controller.GetMaxStamina() * 0.5);
            ApplySevereVisualEffects();
        }
        else if (m_fInfectionLevel > 50)
        {
            // Moderate effects
            controller.SetMaxStamina(controller.GetMaxStamina() * 0.7);
            ApplyModerateVisualEffects();
        }
        else if (m_fInfectionLevel > 20)
        {
            // Mild effects
            controller.SetMaxStamina(controller.GetMaxStamina() * 0.9);
            ApplyMildVisualEffects();
        }
    }
    
    protected void ApplyMildVisualEffects()
    {
        // Apply mild visual effects like slight color changes
        SCR_PostProcessEffectsComponent ppEffects = SCR_PostProcessEffectsComponent.Cast(GetOwner().FindComponent(SCR_PostProcessEffectsComponent));
        if (ppEffects)
        {
            ppEffects.SetVignetteIntensity(0.2);
            ppEffects.SetVignetteColor(Color.FromRGBA(128, 0, 0, 128));
        }
    }
    
    protected void ApplyModerateVisualEffects()
    {
        // Apply moderate visual effects
        SCR_PostProcessEffectsComponent ppEffects = SCR_PostProcessEffectsComponent.Cast(GetOwner().FindComponent(SCR_PostProcessEffectsComponent));
        if (ppEffects)
        {
            ppEffects.SetVignetteIntensity(0.4);
            ppEffects.SetVignetteColor(Color.FromRGBA(128, 0, 0, 180));
            ppEffects.SetColorGradingTint(Color.FromRGBA(180, 130, 130, 255));
        }
    }
    
    protected void ApplySevereVisualEffects()
    {
        // Apply severe visual effects
        SCR_PostProcessEffectsComponent ppEffects = SCR_PostProcessEffectsComponent.Cast(GetOwner().FindComponent(SCR_PostProcessEffectsComponent));
        if (ppEffects)
        {
            ppEffects.SetVignetteIntensity(0.7);
            ppEffects.SetVignetteColor(Color.FromRGBA(150, 0, 0, 220));
            ppEffects.SetColorGradingTint(Color.FromRGBA(220, 100, 100, 255));
            ppEffects.SetBlurIntensity(0.2);
        }
    }
    
    protected void KillFromInfection()
    {
        // Kill character when infection reaches lethal level
        DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));
        if (dmgMgr)
        {
            DamageParams params = new DamageParams();
            params.Damage = 100.0;
            params.DamageType = EDamageType.BIOLOGICAL;
            
            dmgMgr.InflictDamage(params);
        }
    }
    
    float GetInfectionLevel()
    {
        return m_fInfectionLevel;
    }
    
    void AddInfection(float amount)
    {
        if (!m_bIsInfected)
            m_bIsInfected = true;
            
        m_fInfectionLevel += amount;
        
        // Clamp to max
        if (m_fInfectionLevel > m_fDeathThreshold)
            m_fInfectionLevel = m_fDeathThreshold;
    }
    
    void Cure()
    {
        m_bIsInfected = false;
        m_fInfectionLevel = 0;
        
        // Reset visual effects
        SCR_PostProcessEffectsComponent ppEffects = SCR_PostProcessEffectsComponent.Cast(GetOwner().FindComponent(SCR_PostProcessEffectsComponent));
        if (ppEffects)
        {
            ppEffects.SetVignetteIntensity(0);
            ppEffects.SetColorGradingTint(Color.FromRGBA(255, 255, 255, 255));
            ppEffects.SetBlurIntensity(0);
        }
        
        // Reset stamina
        SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(GetOwner().FindComponent(SCR_CharacterControllerComponent));
        if (controller)
        {
            controller.ResetMaxStamina();
        }
    }
}
