cmake_minimum_required(VERSION 3.18)

project(fsm VERSION "0.1.0" LANGUAGES C CXX)
set(PROJECT_DESCRIPTION "Modern C++ fsm library")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CMakeDependentOption)
include_directories(${CMAKE_BINARY_DIR})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(IS_TOPLEVEL_PROJECT TRUE)
else()
    set(IS_TOPLEVEL_PROJECT FALSE)
endif()

cmake_dependent_option(FSM_BUILD_TESTS "Build fsm Tests" ON
  "BUILD_TESTING;IS_TOPLEVEL_PROJECT" OFF)
cmake_dependent_option(FSM_BUILD_DOC "Build fsm Docs" ON
  "IS_TOPLEVEL_PROJECT" OFF)
cmake_dependent_option(FSM_OPT_BUILD_EXAMPLES "Build Examples" ON
  "IS_TOPLEVEL_PROJECT" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)

if(FSM_BUILD_TESTS)
    add_subdirectory(ext/Catch2)
    enable_testing()
endif()

set(MAGIC_ENUM_OPT_INSTALL ON) 
add_subdirectory(ext/magic_enum)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(USE_SYSTEM_TZ_DB ON) # use system timezone support
set(BUILD_TZ_LIB ON) # build timezone support
add_subdirectory(ext/date)


add_subdirectory(ext/compile-time-regular-expressions)

add_subdirectory(libs/core)

add_subdirectory(libs/fsm)

#add_library(${PROJECT_NAME} STATIC)
#add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

#target_include_directories(${PROJECT_NAME}
#        PUBLIC
#            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
#            $<INSTALL_INTERFACE:include>
#            )

#target_sources(${PROJECT_NAME} PRIVATE
#    src/json/value.cpp
#    src/json/json.cpp
#)

#target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
#target_link_libraries(${PROJECT_NAME} PUBLIC date::date magic_enum::magic_enum ctre::ctre fsm::core)

# Dynamic versioning
find_package(Git)
add_custom_target(version
  ${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/include/version.h.in
                   -D DST=${CMAKE_BINARY_DIR}/include/version.h
                   -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
                   -P ${CMAKE_SOURCE_DIR}/cmake/GenerateVersionHeader.cmake
  )
add_dependencies(${PROJECT_NAME} version)

install(TARGETS 
          ${PROJECT_NAME}
          core
        EXPORT ${PROJECT_NAME}-targets)

if (NOT EXISTS "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in")
  file(WRITE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in [[
    @PACKAGE_INIT@
    include(CMakeFindDependencyMacro)

    find_dependency(date REQUIRED)
    find_dependency(magic_enum REQUIRED)
    find_dependency(ctre REQUIRED)

    include("${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-targets.cmake")
  ]])
endif()

configure_package_config_file(
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(EXPORT ${PROJECT_NAME}-targets 
  DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}"
  NAMESPACE ${PROJECT_NAME}::)
install(
  FILES
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include
    FILES_MATCHING PATTERN *.h PATTERN *.hpp)

#if(FSM_OPT_BUILD_EXAMPLES)
#    add_subdirectory(example)
#endif()

#if(FSM_BUILD_TESTS)
#    add_subdirectory(ext/Catch2)
#    enable_testing()
#    add_subdirectory(test)
#endif()

#
# Documentation
#

if(FSM_BUILD_DOC)
    find_package(Doxygen 1.8)

    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()