class SCR_ZombieWaterItem : SCR_ConsumableItemComponent
{
    [Attribute("30.0", UIWidgets.Auto, "Amount of thirst restored when consumed")]
    protected float m_fThirstValue;
    
    override void OnConsume(IEntity user)
    {
        super.OnConsume(user);
        
        // Apply thirst restoration
        SCR_ZombieSurvivalComponent survivalComp = SCR_ZombieSurvivalComponent.Cast(user.FindComponent(SCR_ZombieSurvivalComponent));
        if (survivalComp)
        {
            survivalComp.AddThirst(m_fThirstValue);
        }
    }
}
