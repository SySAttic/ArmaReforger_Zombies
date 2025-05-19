class SCR_ZombieMedicalItem : SCR_ConsumableItemComponent
{
    [Attribute("25.0", UIWidgets.Auto, "Amount of health restored")]
    protected float m_fHealthValue;
    
    [Attribute("0", UIWidgets.CheckBox, "Whether this item cures infection")]
    protected bool m_bCuresInfection;
    
    [Attribute("0.0", UIWidgets.Auto, "Amount of infection reduced (0-100)")]
    protected float m_fInfectionReduction;
    
    override void OnConsume(IEntity user)
    {
        super.OnConsume(user);
        
        // Apply health restoration
        DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(user.FindComponent(DamageManagerComponent));
        if (dmgMgr)
        {
            float currentHealth = dmgMgr.GetHealth();
            dmgMgr.SetHealth(Math.Min(dmgMgr.GetHealthMax(), currentHealth + m_fHealthValue));
        }
        
        // Handle infection effects
        SCR_ZombieInfectionComponent infectionComp = SCR_ZombieInfectionComponent.Cast(user.FindComponent(SCR_ZombieInfectionComponent));
        if (infectionComp)
        {
            if (m_bCuresInfection)
            {
                infectionComp.Cure();
            }
            else if (m_fInfectionReduction > 0)
            {
                float currentInfection = infectionComp.GetInfectionLevel();
                infectionComp.AddInfection(-m_fInfectionReduction); // Negative to reduce
            }
        }
    }
}
