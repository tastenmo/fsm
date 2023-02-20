cmake_minimum_required(VERSION 3.18)

project(fsm VERSION "0.1.0" LANGUAGES CXX)
set(PROJECT_DESCRIPTION "Modern C++ fsm library")

include(FetchContent)
include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(GNUInstallDirs)
include(CTest)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(IS_TOPLEVEL_PROJECT TRUE)
else()
    set(IS_TOPLEVEL_PROJECT FALSE)
endif()

find_program(FSM_DPKG_BUILDPACKAGE_FOUND dpkg-buildpackage)
find_program(FSM_RPMBUILD_FOUND rpmbuild)

cmake_dependent_option(FSM_BUILD_TESTS "Build fsm Tests" ON
  "BUILD_TESTING;CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)
cmake_dependent_option(FSM_BUILD_PACKAGE "Build fsm Packages" ON
  "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)
cmake_dependent_option(FSM_BUILD_PACKAGE_DEB
  "Create DEB Package (${PROJECT_NAME})" ON
  "FSM_BUILD_PACKAGE;FSM_DPKG_BUILDPACKAGE_FOUND" OFF)
cmake_dependent_option(FSM_BUILD_PACKAGE_RPM
  "Create RPM Package (${PROJECT_NAME})" ON
  "FSM_BUILD_PACKAGE;FSM_RPMBUILD_FOUND" OFF)
cmake_dependent_option(FSM_BUILD_DOC "Build fsm Docs" ON
  "BUILD_TESTING;CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)


set(CMAKE_CXX_STANDARD 17)

#add_compile_options(-Wall -Wextra -pedantic -Werror)


FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
  GIT_TAG        v0.8.2 # or a later release
)

set(MAGIC_ENUM_OPT_INSTALL ON) 

FetchContent_MakeAvailable(magic_enum)

set(USE_SYSTEM_TZ_DB ON) # use system timezone support
set(BUILD_TZ_LIB ON) # build timezone support

FetchContent_Declare(
  date_src
  GIT_REPOSITORY https://github.com/HowardHinnant/date.git
  GIT_TAG        v3.0.1 # or a later release
)



FetchContent_MakeAvailable(date_src)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
        INTERFACE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
            )

target_compile_options(${PROJECT_NAME} INTERFACE
            -Wall -Wextra -Wpedantic
          )

target_link_libraries(${PROJECT_NAME} INTERFACE date::date magic_enum::magic_enum)

#target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17})

#install(TARGETS magic_enum EXPORT ${PROJECT_NAME}Targets)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets)

if (NOT EXISTS "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in")
  file(WRITE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in [[
    @PACKAGE_INIT@
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
    FILES_MATCHING PATTERN *.hpp)

if(FSM_OPT_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(FSM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()


if (NOT FSM_BUILD_PACKAGE)
  return()
endif()

list(APPEND source-generators TBZ2 TGZ TXZ ZIP)

if (FSM_BUILD_PACKAGE_DEB)
  list(APPEND binary-generators "DEB")
endif()

if (FSM_BUILD_PACKAGE_RPM)
  list(APPEND binary-generators "RPM")
endif()

set(CPACK_SOURCE_GENERATOR ${source-generators})
set(CPACK_GENERATOR ${binary-generators})

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MArtin Heubuch")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_DEBIAN_PACKAGE_NAME "lib${PROJECT_NAME}-dev")

set(CPACK_RPM_PACKAGE_NAME "lib${PROJECT_NAME}-devel")

set(PKG_CONFIG_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/packaging/pkgconfig.pc.in" "${PKG_CONFIG_FILE_NAME}" @ONLY)
install(FILES "${PKG_CONFIG_FILE_NAME}"
	DESTINATION "${CMAKE_INSTALL_DATADIR}/pkgconfig"
)

list(APPEND CPACK_SOURCE_IGNORE_FILES /.git/ /build/ .gitignore .DS_Store)

include(CPack)

#
# Documentation
#

if(FSM_BUILD_DOC)
    find_package(Doxygen 1.8)

    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()