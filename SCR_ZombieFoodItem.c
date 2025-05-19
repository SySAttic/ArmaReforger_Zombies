class SCR_ZombieFoodItem : SCR_ConsumableItemComponent
{
    [Attribute("25.0", UIWidgets.Auto, "Amount of hunger restored when consumed")]
    protected float m_fHungerValue;
    
    override void OnConsume(IEntity user)
    {
        super.OnConsume(user);
        
        // Apply hunger restoration
        SCR_ZombieSurvivalComponent survivalComp = SCR_ZombieSurvivalComponent.Cast(user.FindComponent(SCR_ZombieSurvivalComponent));
        if (survivalComp)
        {
            survivalComp.AddHunger(m_fHungerValue);
        }
    }
}
