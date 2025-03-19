
add_library(fsm INTERFACE)
add_library(${PROJECT_NAME}::fsm ALIAS fsm)

target_include_directories(fsm
        INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libs/fsm/include>
        $<INSTALL_INTERFACE:include>
            )            

target_link_libraries(fsm INTERFACE magic_enum::magic_enum fsm::core)

if(FSM_BUILD_TESTS)
    add_subdirectory(test)
endif()
