cmake_minimum_required(VERSION 3.31)
project(atlantis CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(engine
    aregion.cpp
    army.cpp
    astring.cpp
    battle.cpp
    economy.cpp
    edit.cpp
    faction.cpp
    game.cpp
    gamedata.cpp
    gamedefs.cpp
    genrules.cpp
    items.cpp
    main.cpp
    market.cpp
    modify.cpp
    monthorders.cpp
    npc.cpp
    object.cpp
    orders.cpp
    parseorders.cpp
    production.cpp
    quests.cpp
    runorders.cpp
    skills.cpp
    skillshows.cpp
    specials.cpp
    spells.cpp
    unit.cpp
    events.cpp
    events-battle.cpp
    events-assassination.cpp
    mapgen.cpp
    simplex.cpp
    namegen.cpp
    indenter.cpp
    text_report_generator.cpp
)
add_executable(basic
    basic/extra.cpp
    basic/map.cpp
    basic/monsters.cpp
    basic/rules.cpp
    basic/world.cpp
)
target_link_libraries(basic engine)
target_include_directories(basic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
