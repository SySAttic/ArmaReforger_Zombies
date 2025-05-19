class SCR_ZombieItemSpawnerComponent : ScriptComponent
{
    [Attribute("", UIWidgets.Auto, "Array of item prefabs that can spawn here")]
    protected ref array<ResourceName> m_aPossibleItems;
    
    [Attribute("0.5", UIWidgets.Auto, "Chance that an item will spawn (0-1)")]
    protected float m_fSpawnChance;
    
    protected IEntity m_CurrentSpawnedItem;
    
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        
        // Initial spawn with chance
        if (Math.RandomFloat01() <= m_fSpawnChance)
        {
            SpawnItem();
        }
    }
    
    bool IsEmpty()
    {
        return !m_CurrentSpawnedItem;
    }
    
    void SpawnItem()
    {
        if (!IsEmpty() || m_aPossibleItems.IsEmpty())
            return;
            
        // Pick random item from possible items
        ResourceName selectedItem = m_aPossibleItems.GetRandomElement();
        Resource itemRes = Resource.Load(selectedItem);
        if (!itemRes)
            return;
            
        // Set up spawn parameters
        EntitySpawnParams params = new EntitySpawnParams();
        params.TransformMode = ETransformMode.WORLD;
        params.Transform = GetOwner().GetTransform();
        
        // Spawn the item
        IEntity itemEntity = GetGame().SpawnEntityPrefab(itemRes, GetOwner(), params);
        if (!itemEntity)
            return;
            
        // Store reference
        m_CurrentSpawnedItem = itemEntity;
        
        // Set up removal on pickup
        SCR_InventoryItemComponent itemComp = SCR_InventoryItemComponent.Cast(itemEntity.FindComponent(SCR_InventoryItemComponent));
        if (itemComp)
        {
            itemComp.GetOnItemPickedUp().Insert(OnItemPickedUp);
        }
    }
    
    protected void OnItemPickedUp(IEntity item, IEntity user)
    {
        if (item == m_CurrentSpawnedItem)
        {
            m_CurrentSpawnedItem = null;
        }
    }
}
