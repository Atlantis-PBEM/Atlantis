cmake_minimum_required(VERSION 3.22)
project(atlantis CXX)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_VERBOSE_MAKEFILE ON)
# set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_SOURCE_DIR}/CompilerSupport.cmake)
if (NOT COMPILER_HAS_RANGES)
    message(FATAL_ERROR "Compiler lacks full support for C++20 ranges.")
endif(NOT COMPILER_HAS_RANGES)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    #add_compile_options(-pedantic -Wsign-compare -Wextra -Werror -Wno-switch -Wno-unknown-pragmas -Wpointer-arith -Wno-char-subscripts -Wno-long-long)
    add_compile_options(-g -Wall -Werror -pedantic -Wparentheses -Wno-psabi
        -Wpointer-arith -Wignored-qualifiers 
    # -Wunused-parameter
    )
elseif(MSVC)
    #add_compile_options(/WX /MP /FC /D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/MP /FC /D_CRT_SECURE_NO_WARNINGS /wd4244 /wd4267 /wd4700)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG
        "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrt.lib")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")
else()
    message(STATUS "unknown compiler ${CMAKE_CXX_COMPILER_ID}")
endif()


add_library(engine
    aregion.cpp
    army.cpp
    battle.cpp
    economy.cpp
    edit.cpp
    faction.cpp
    game.cpp
    gamedata.cpp
    gamedefs.cpp
    genrules.cpp
    items.cpp
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

macro(add_game GAME)
add_executable(${GAME}
    main.cpp
    ${GAME}/extra.cpp
    ${GAME}/map.cpp
    ${GAME}/monsters.cpp
    ${GAME}/rules.cpp
    ${GAME}/world.cpp
)
target_link_libraries(${GAME} engine)
target_include_directories(${GAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set (_RULES_CSS ${GAME}.css)
set (_RULES_INTRO ${GAME}_intro.html)
set (_RULES_OUTPUT ${GAME}.html)
add_custom_command(
    COMMAND ${GAME} ARGS genrules ${CMAKE_SOURCE_DIR}/${GAME}/${_RULES_INTRO} ${_RULES_CSS} ${_RULES_OUTPUT}
    OUTPUT ${GAME}.html
    MAIN_DEPENDENCY ${GAME}
    DEPENDS ${GAME}/${_RULES_INTRO} ${GAME}/html/${_RULES_CSS}
    COMMENT "Creating ${GAME}.html"
)
add_custom_target(${GAME}-rules DEPENDS ${GAME}.html)
install(FILES ${CMAKE_BINARY_DIR}/${_RULES_OUTPUT} ${GAME}/html/${_RULES_CSS} DESTINATION html/${GAME})
unset(_RULES_CSS)
unset(_RULES_INTRO)
unset(_RULES_OUTPUT)
endmacro()

set (GAMES
basic
fracas
havilah
kingdoms
neworigins
standard
)

foreach(GAME IN ITEMS ${GAMES})
add_game(${GAME})
endforeach()

add_custom_target(games DEPENDS ${GAMES})

set(RULE_TARGETS ${GAMES})
list(TRANSFORM RULE_TARGETS APPEND "-rules")

add_custom_target(rules ALL DEPENDS ${RULE_TARGETS})
install(TARGETS ${GAMES} DESTINATION bin)

file(GLOB TEST_SOURCES unittest/*_test.cpp)

add_executable(unittest
    unittest/main.cpp
    unittest/extra.cpp
    unittest/map.cpp
    unittest/monsters.cpp
    unittest/rules.cpp
    unittest/world.cpp
    unittest/testhelper.cpp
    ${TEST_SOURCES}
)
target_link_libraries(unittest engine)
target_include_directories(unittest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_test(NAME unittests COMMAND unittest)
add_custom_target(tests COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS unittest)
