add_library(reflect INTERFACE)
add_library(${PROJECT_NAME}::reflect ALIAS reflect)

# Keep this target header-only and isolated from concrete parsers.
target_include_directories(reflect
        INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libs/reflect/include>
        $<INSTALL_INTERFACE:include>
            )

target_link_libraries(reflect INTERFACE core ctre::ctre)

if(FSM_BUILD_TESTS)
    add_subdirectory(test)
endif()
