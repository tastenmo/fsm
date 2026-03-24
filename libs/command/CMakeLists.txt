add_library(command STATIC)
add_library(${PROJECT_NAME}::command ALIAS command)

target_include_directories(command
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/libs/command/include>
        $<INSTALL_INTERFACE:include>
            )

target_sources(command PRIVATE
    src/text_parser.cpp
)

target_link_libraries(command PUBLIC fsm reflect reflect_json json signal ctre::ctre)

if(FSM_BUILD_TESTS)
    add_subdirectory(test)
endif()
