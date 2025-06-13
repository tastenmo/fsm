
add_library(json STATIC)
add_library(${PROJECT_NAME}::json ALIAS json)

target_include_directories(json
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libs/json/include>
        $<INSTALL_INTERFACE:include>
            )            

target_sources(json PRIVATE
    src/value.cpp
    src/json.cpp
)

target_link_libraries(json PUBLIC ctre::ctre fsm::fsm)

if(FSM_BUILD_TESTS)
    add_subdirectory(test)
endif()
