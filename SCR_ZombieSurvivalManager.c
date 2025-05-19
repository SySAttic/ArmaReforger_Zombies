class SCR_ZombieSurvivalManager : ScriptComponent
{
    [Attribute("10.0", UIWidgets.Auto, "Food item respawn time in minutes")]
    protected float m_fFoodRespawnTime;
    
    [Attribute("8.0", UIWidgets.Auto, "Water item respawn time in minutes")]
    protected float m_fWaterRespawnTime;
    
    [Attribute("15.0", UIWidgets.Auto, "Medical item respawn time in minutes")]
    protected float m_fMedicalRespawnTime;
    
    [Attribute("$FOOD_SPAWNER_PREFAB", UIWidgets.ResourceNamePicker, "Food spawner prefab")]
    protected ResourceName m_FoodSpawnerPrefab;
    
    [Attribute("$WATER_SPAWNER_PREFAB", UIWidgets.ResourceNamePicker, "Water spawner prefab")]
    protected ResourceName m_WaterSpawnerPrefab;
    
    [Attribute("$MEDICAL_SPAWNER_PREFAB", UIWidgets.ResourceNamePicker, "Medical spawner prefab")]
    protected ResourceName m_MedicalSpawnerPrefab;
    
    protected ref array<IEntity> m_aFoodSpawners = new array<IEntity>();
    protected ref array<IEntity> m_aWaterSpawners = new array<IEntity>();
    protected ref array<IEntity> m_aMedicalSpawners = new array<IEntity>();
    
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        
        // Find all spawners in the world
        FindAllSpawners();
        
        // Initialize regular respawn cycles
        GetGame().GetCallqueue().CallLater(RespawnFoodItems, m_fFoodRespawnTime * 60 * 1000, true);
        GetGame().GetCallqueue().CallLater(RespawnWaterItems, m_fWaterRespawnTime * 60 * 1000, true);
        GetGame().GetCallqueue().CallLater(RespawnMedicalItems, m_fMedicalRespawnTime * 60 * 1000, true);
    }
    
    override void OnDelete(IEntity owner)
    {
        GetGame().GetCallqueue().Remove(RespawnFoodItems);
        GetGame().GetCallqueue().Remove(RespawnWaterItems);
        GetGame().GetCallqueue().Remove(RespawnMedicalItems);
        super.OnDelete(owner);
    }
    
    protected void FindAllSpawners()
    {
        // Find all spawner entities in the world
        array<IEntity> allEntities = new array<IEntity>();
        GetGame().GetWorld().GetEntityRegistry().FindEntitiesByName("*_Spawner", allEntities);
        
        foreach (IEntity entity : allEntities)
        {
            if (entity.GetName().Contains("Food"))
            {
                m_aFoodSpawners.Insert(entity);
            }
            else if (entity.GetName().Contains("Water"))
            {
                m_aWaterSpawners.Insert(entity);
            }
            else if (entity.GetName().Contains("Medical"))
            {
                m_aMedicalSpawners.Insert(entity);
            }
        }
    }
    
    protected void RespawnFoodItems()
    {
        foreach (IEntity spawner : m_aFoodSpawners)
        {
            if (!spawner)
                continue;
                
            // Check if spawner is empty
            SCR_ZombieItemSpawnerComponent spawnerComp = SCR_ZombieItemSpawnerComponent.Cast(spawner.FindComponent(SCR_ZombieItemSpawnerComponent));
            if (spawnerComp && spawnerComp.IsEmpty())
            {
                spawnerComp.SpawnItem();
            }
        }
    }
    
    protected void RespawnWaterItems()
    {
        foreach (IEntity spawner : m_aWaterSpawners)
        {
            if (!spawner)
                continue;
                
            SCR_ZombieItemSpawnerComponent spawnerComp = SCR_ZombieItemSpawnerComponent.Cast(spawner.FindComponent(SCR_ZombieItemSpawnerComponent));
            if (spawnerComp && spawnerComp.IsEmpty())
            {
                spawnerComp.SpawnItem();
            }
        }
    }
    
    protected void RespawnMedicalItems()
    {
        foreach (IEntity spawner : m_aMedicalSpawners)
        {
            if (!spawner)
                continue;
                
            SCR_ZombieItemSpawnerComponent spawnerComp = SCR_ZombieItemSpawnerComponent.Cast(spawner.FindComponent(SCR_ZombieItemSpawnerComponent));
            if (spawnerComp && spawnerComp.IsEmpty())
            {
                spawnerComp.SpawnItem();
            }
        }
    }
}
