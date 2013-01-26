PROJECT(hearthstone-world)
SET( prefix ${ROOT_PATH}/src/hearthstone-world )
SET(sources
    AddonMgr.cpp
    AchievementInterface.cpp
    AI/AICombat.cpp
    AI/AIEvent.cpp
    AI/AIInterface.cpp
    AI/AIMovement.cpp
    AI/AISpellCasting.cpp
    AI/AIWaypoints.cpp
    AlteracValley.cpp
    ArathiBasin.cpp
    AreaTrigger.cpp
    Arenas.cpp
    ArenaTeam.cpp
    AuctionHouse.cpp
    AuctionMgr.cpp
    AuraInterface.cpp
    BattlegroundCommands.cpp
    BattlegroundHandler.cpp
    BattlegroundMgr.cpp
    Channel.cpp
    ChannelHandler.cpp
    CharacterHandler.cpp
    Chat.cpp
    ChatHandler.cpp
    CollideInterface.cpp
    CombatHandler.cpp
    ConsoleCommands.cpp
    ConsoleListener.cpp
    Container.cpp
    Corpse.cpp
    Creature.cpp
    DatabaseCommands.cpp
    DayWatcherThread.cpp
    debugcmds.cpp
    DuelHandler.cpp
    DynamicObject.cpp
    EventableObject.cpp
    EyeOfTheStorm.cpp
    EventMgr.cpp
    faction.cpp
    GameObject.cpp
    GMTicket.cpp
    GMTicketCommands.cpp
    Group.cpp
    GroupHandler.cpp
    GuildHandler.cpp
    GuildManager.cpp
    GuildManagerFunctions.cpp
    HonorHandler.cpp
    IsleOfConquest.cpp
    Item.cpp
    ItemHandler.cpp
    ItemInterface.cpp
    Level0.cpp
    Level1.cpp
    Level2.cpp
    Level3.cpp
    LfgHandler.cpp
    LfgMgr.cpp
    LootMgr.cpp
    LogonCommClient.cpp
    LogonCommHandler.cpp
    MailSystem.cpp
    Main.cpp
    Map.cpp
    MapCell.cpp
    MapManagerScript.cpp
    MapMgr.cpp
    Master.cpp
    MiscHandler.cpp
    MovementHandler.cpp
    NavMeshInterface.cpp
    NPCHandler.cpp
    Object.cpp
    ObjectMgr.cpp
    ObjectStorage.cpp
    Opcodes.cpp
    Pet.cpp
    PetHandler.cpp
    Player.cpp
    PlayerPacketWrapper.cpp
    QueryHandler.cpp
    Quest.cpp
    QuestHandler.cpp
    QuestMgr.cpp
    QuestCommands.cpp
    RaidHandler.cpp
    RecallCommands.cpp
    ReputationHandler.cpp
    ScriptMgr.cpp
    SkillHandler.cpp
    SkillNameMgr.cpp
    SocialHandler.cpp
    SpellAuras.cpp
    SpellTarget.cpp
    Spell.cpp
    SpellEffects.cpp
    SpellHandler.cpp
    Spellfixes/SpellFixes.cpp
    Spellfixes/SingleCoefFixes.cpp
    Spellfixes/SingleSpellFixes.cpp
    Stats.cpp
    StdAfx.cpp
    StrandOfTheAncients.cpp
    Summons.cpp
    TaxiHandler.cpp
    TaxiMgr.cpp
    TerrainMgr.cpp
    Tracker.cpp
    TradeHandler.cpp
    TransporterHandler.cpp
    Unit.cpp
    Vehicle.cpp
    VoiceChatClientSocket.cpp
    VoiceChatHandler.cpp
    WarnSystem.cpp
    WarnSystem_Punishments.cpp
    WarsongGulch.cpp
    WayPoints.cpp
    WeatherMgr.cpp
    Wintergrasp.cpp
    WintergraspInternal.cpp
    World.cpp
    WorldCreator.cpp
    WorldSession.cpp
    WorldSocket.cpp
    WorldRunnable.cpp
    WorldStateManager.cpp
    WordFilter.cpp
)

SET( headers
    AddonMgr.h
    AchievementInterface.h
    AI/AI_Headers.h
    AI/AI_Defines.h
    AI/AIInterface.h
    AI/AIMovement.h
    AlteracValley.h
    ArathiBasin.h
    AreaTrigger.h
    Arenas.h
    ArenaTeam.h
    AuctionHouse.h
    AuctionMgr.h
    AuraInterface.h
    BattlegroundMgr.h
    CallScripting.h
    CellHandler.h
    Channel.h
    ChannelMgr.h
    Chat.h
    CollideInterface.h
    ConsoleCommands.h
    Container.h
    Corpse.h
    Creature.h
    DayWatcherThread.h
    DynamicObject.h
    EyeOfTheStorm.h
    EventableObject.h
    EventMgr.h
    faction.h
    GameObject.h
    Group.h
    GuildDefines.h
    GuildManager.h
    HonorHandler.h
    IsleOfConquest.h
    Item.h
    ItemInterface.h
    ItemPrototype.h
    LfgMgr.h
    LootMgr.h
    LogonCommHandler.h
    LogonCommClient.h
    MailSystem.h
    Map.h
    MapCell.h
    MapManagerScript.h
    MapMgr.h
    Master.h
    MiscHandler.h
    NameTables.h
    NavMeshInterface.h
    NPCHandler.h
    Object.h
    ObjectMgr.h
    ObjectStorage.h
    Opcodes.h
    Pet.h
    Player.h
    Quest.h
    QuestMgr.h
    ScriptMgr.h
    Skill.h
    SkillNameMgr.h
    SpellAuras.h
    SpellDefines.h
    Spell.h
    Stats.h
    StdAfx.h
    StrandOfTheAncients.h
    Summons.h
    TaxiMgr.h
    TerrainMgr.h
    Tracker.h
    TransporterHandler.h
    Unit.h
    UpdateFields.h
    UpdateMask.h
    Vehicle.h
    VoiceChatClientSocket.h
    VoiceChatHandler.h
    WarnSystem.h
    WarsongGulch.h
    WeatherMgr.h
    Wintergrasp.h
    WintergraspInternal.h
    World.h
    WorldCreator.h
    WorldSession.h
    WorldSocket.h
    WorldRunnable.h
    WorldStates.h
    WorldStateManager.h
    WordFilter.h
)
	

foreach(src IN ITEMS ${sources} )
  SET( SRCS ${SRCS} ${prefix}/${src} )
endforeach(src)
foreach(src IN ITEMS ${headers} )
  SET( HEADERS ${HEADERS} ${prefix}/${src} )
endforeach(src)

include(PCHSupport)
include_directories( ${GLOBAL_INCLUDE_DIRS} )
link_directories( ${DEPENDENCY_LIBS} ${EXTRA_LIBS_PATH} )

add_executable( ${PROJECT_NAME} ${SRCS} ${HEADERS} )

IF( CMAKE_GENERATOR MATCHES Visual*)
	ADD_NATIVE_PRECOMPILED_HEADER(${PROJECT_NAME} StdAfx.h ${prefix}/StdAfx.cpp)
ENDIF()

set_source_files_properties(icon.ico resources.rc PROPERTIES LANGUAGE RC)
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS TRUE)

add_dependencies( ${PROJECT_NAME} hearthstone-shared )
target_link_libraries( ${PROJECT_NAME} hearthstone-shared ${PCRE_LIBRARIES} )
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)





