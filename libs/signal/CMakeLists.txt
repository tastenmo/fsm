
add_library(signal INTERFACE)
add_library(${PROJECT_NAME}::signal ALIAS signal)

target_include_directories(signal
        INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
            )

target_link_libraries(signal
        INTERFACE
        ${PROJECT_NAME}::core
            )

if(FSM_BUILD_TESTS)
    add_subdirectory(test)
endif()
