[EntityEditorProps(category: "GameScripted/GameMode", description: "DayZ-like zombie game mode")]
class SCR_ZombieGameMode : SCR_GameModeBase
{
    [Attribute("50", UIWidgets.Auto, "Number of zombies to spawn initially")]
    protected int m_iInitialZombieCount;
    
    [Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Enable player infection system")]
    protected bool m_bEnableInfection;
    
    [Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Enable hunger and thirst")]
    protected bool m_bEnableSurvival;
    
    protected ref ZombieManager m_ZombieManager;
    protected ref SCR_ZombieSurvivalManager m_SurvivalManager;
    protected ref SCR_ZombieInfectionManager m_InfectionManager;
    
    override void OnGameModeStart()
    {
        super.OnGameModeStart();
        
        // Create and initialize zombie manager
        m_ZombieManager = new ZombieManager();
        GetGame().GetWorld().AddComponent(m_ZombieManager);
        
        // Create survival manager if enabled
        if (m_bEnableSurvival)
        {
            m_SurvivalManager = new SCR_ZombieSurvivalManager();
            GetGame().GetWorld().AddComponent(m_SurvivalManager);
        }
        
        // Create infection manager if enabled
        if (m_bEnableInfection)
        {
            m_InfectionManager = new SCR_ZombieInfectionManager();
            GetGame().GetWorld().AddComponent(m_InfectionManager);
        }
        
        // Set up world
        SetupWorld();
    }
    
    override void OnGameModeEnd()
    {
        super.OnGameModeEnd();
        
        // Clean up managers
        if (m_ZombieManager)
            GetGame().GetWorld().RemoveComponent(m_ZombieManager);
            
        if (m_SurvivalManager)
            GetGame().GetWorld().RemoveComponent(m_SurvivalManager);
            
        if (m_InfectionManager)
            GetGame().GetWorld().RemoveComponent(m_InfectionManager);
    }
    
    override protected void OnPlayerConnected(int playerId)
    {
        super.OnPlayerConnected(playerId);
        
        // Initialize player for zombie game mode
        PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
        if (playerController)
        {
            IEntity controlledEntity = playerController.GetControlledEntity();
            if (controlledEntity)
            {
                InitializePlayerEntity(controlledEntity);
            }
        }
    }
    
    protected void InitializePlayerEntity(IEntity playerEntity)
    {
        // Add survival component if enabled
        if (m_bEnableSurvival)
        {
            SCR_ZombieSurvivalComponent survivalComp = SCR_ZombieSurvivalComponent.Cast(playerEntity.FindComponent(SCR_ZombieSurvivalComponent));
            if (!survivalComp)
            {
                survivalComp = new SCR_ZombieSurvivalComponent();
                playerEntity.AddComponent(survivalComp);
            }
        }
        
        // Add infection component if enabled
        if (m_bEnableInfection)
        {
            SCR_ZombieInfectionComponent infectionComp = SCR_ZombieInfectionComponent.Cast(playerEntity.FindComponent(SCR_ZombieInfectionComponent));
            if (!infectionComp)
            {
                infectionComp = new SCR_ZombieInfectionComponent();
                playerEntity.AddComponent(infectionComp);
            }
        }
    }
    
    protected void SetupWorld()
    {
        // Adjust world settings for a more apocalyptic feel
        WorldEntity world = GetGame().GetWorld();
        if (!world)
            return;
            
        // Adjust weather if needed
        WeatherManagerEntity weatherManager = world.GetWeatherManager();
        if (weatherManager)
        {
            // Set overcast/fog for more apocalyptic atmosphere
            weatherManager.SetOvercast(0.7);
            weatherManager.SetFog(0.2);
        }
        
        // Adjust time of day if needed
        TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
        if (timeManager)
        {
            // Set time to evening for more atmosphere
            timeManager.SetTime(18, 0);
        }
    }
}
