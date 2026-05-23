# ── AIPlayer static library ───────────────────────────────────────────────────
add_library(AIPlayer STATIC
    AIPlayer.cpp
)

target_include_directories(AIPlayer
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(AIPlayer PUBLIC project_warnings)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(combat_system
    main.cpp
)

target_include_directories(combat_system
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(combat_system
    PRIVATE
        AIPlayer
        project_warnings
)

# ── Output directory ──────────────────────────────────────────────────────────
set_target_properties(combat_system PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)

# ── Install rules ─────────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS combat_system
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS AIPlayer
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
    AIPlayer.h
    Card.h
    CombatState.h
    Combat.h
    Deck.h
    Die.h
    Enemy.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/blackjack_combat
)
