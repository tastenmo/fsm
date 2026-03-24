add_library(reflect_json INTERFACE)
add_library(${PROJECT_NAME}::reflect_json ALIAS reflect_json)

target_include_directories(reflect_json
        INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libs/reflect_json/include>
        $<INSTALL_INTERFACE:include>
            )

target_link_libraries(reflect_json INTERFACE reflect json)

if(FSM_BUILD_TESTS)
    add_subdirectory(test)
endif()
