
add_library(logging INTERFACE)
add_library(${PROJECT_NAME}::logging ALIAS logging)

target_include_directories(logging
        INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libs/logging/include>
        $<INSTALL_INTERFACE:include>
            )

target_link_libraries(logging
        INTERFACE
        ${PROJECT_NAME}::signal
        magic_enum::magic_enum
            )

if(FSM_BUILD_TESTS)
    add_subdirectory(test)
endif()
