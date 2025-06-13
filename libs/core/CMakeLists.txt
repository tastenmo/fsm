
add_library(core INTERFACE)
add_library(${PROJECT_NAME}::core ALIAS core)

target_include_directories(core
        INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libs/core/include>
        $<INSTALL_INTERFACE:include>
            )            

if(FSM_BUILD_TESTS)
    add_subdirectory(test)
endif()
