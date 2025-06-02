include(CheckCXXCompilerFlag)
#Include(FetchContent)

#FetchContent_Declare(
#  Catch2
#  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#  GIT_TAG        v3.0.1 # or a later release
#)

#FetchContent_MakeAvailable(Catch2)

#list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

#find_package(Catch2 3 REQUIRED)

include(CTest)
include(Catch)


set(SOURCES test.cpp)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(OPTIONS /W4 /WX)
    check_cxx_compiler_flag(/permissive HAS_PERMISSIVE_FLAG)
    if(HAS_PERMISSIVE_FLAG)
        set(OPTIONS ${OPTIONS} /permissive-)
    endif()

    check_cxx_compiler_flag(/std:c++20 HAS_CPP20_FLAG)
    check_cxx_compiler_flag(/std:c++latest HAS_CPPLATEST_FLAG)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_VERBOSE_MAKEFILE ON)
    set(OPTIONS -Wall -Wextra -pedantic-errors)

    check_cxx_compiler_flag(-std=c++20 HAS_CPP20_FLAG)
endif()



function(make_test src target std)
    add_executable(${target} ${src})
    target_compile_options(${target} PRIVATE ${OPTIONS})
    #target_include_directories(${target} PRIVATE 3rdparty/Catch2/include)
    target_link_libraries(${target} PRIVATE ${CMAKE_PROJECT_NAME} Catch2::Catch2WithMain)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    if(std)
        if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
            target_compile_options(${target} PRIVATE /std:${std})
        else()
            target_compile_options(${target} PRIVATE -std=${std})
        endif()
    endif()
    #add_test(NAME ${target} COMMAND ${target})
    catch_discover_tests(${target} TEST_SUFFIX ${std})
endfunction()

function(make_test_with_libs src target std libs)
    add_executable(${target} ${src})
    target_compile_options(${target} PRIVATE ${OPTIONS})
    #target_include_directories(${target} PRIVATE 3rdparty/Catch2/include)
    target_link_libraries(${target} PRIVATE ${CMAKE_PROJECT_NAME} ${libs} Catch2::Catch2WithMain)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    if(std)
        if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
            target_compile_options(${target} PRIVATE /std:${std})
        else()
            target_compile_options(${target} PRIVATE -std=${std})
        endif()
    endif()
    #add_test(NAME ${target} COMMAND ${target})
    catch_discover_tests(${target} TEST_SUFFIX ${std})
endfunction()

function(make_test_with_includes src target std include_dirs)
    add_executable(${target} ${src})
    target_compile_options(${target} PRIVATE ${OPTIONS})
    target_include_directories(${target} PRIVATE ${include_dirs})
    target_link_libraries(${target} PRIVATE ${CMAKE_PROJECT_NAME} Catch2::Catch2WithMain)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    if(std)
        if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
            target_compile_options(${target} PRIVATE /std:${std})
        else()
            target_compile_options(${target} PRIVATE -std=${std})
        endif()
    endif()
    #add_test(NAME ${target} COMMAND ${target})
    catch_discover_tests(${target} TEST_SUFFIX ${std})
endfunction()

make_test(testFirst.cpp testFirst-cpp17 c++17)

if(HAS_CPP20_FLAG)
    make_test(testFirst.cpp testFirst-cpp20 c++20)
endif()

if(HAS_CPPLATEST_FLAG)
    make_test(testFirst.cpp testFirst-cpplatest c++latest)
endif()

make_test(testDelegates.cpp testDelegates-cpp17 c++17)

if(HAS_CPP20_FLAG)
    make_test(testDelegates.cpp testDelegates-cpp20 c++20)
endif()

make_test(testSignalSlot.cpp testSignalSlot-cpp17 c++17)

if(HAS_CPP20_FLAG)
    make_test(testSignalSlot.cpp testSignalSlot-cpp20 c++20)
endif()

make_test(testTypeTraits.cpp testTypeTraits-cpp17 c++17)

if(HAS_CPP20_FLAG)
    make_test(testTypeTraits.cpp testTypeTraits-cpp20 c++20)
endif()

make_test(testStateManager.cpp testStateManager-cpp17 c++17)

make_test(testFSMpp17.cpp testFSMpp-cpp17 c++17)

make_test(testCompressedPair.cpp testCompressedPair-cpp17 c++17)

make_test(testIterator.cpp testIterator-cpp17 c++17)

make_test(testDenseMap.cpp testDenseMap-cpp17 c++17)

make_test(testEmitter.cpp testEmitter-cpp17 c++17)

make_test(testDispatcher.cpp testDispatcher-cpp17 c++17)

#make_test(testLogging.cpp testLogging-cpp17 c++17)

if(HAS_CPP20_FLAG)
    make_test(testLogging.cpp testLogging-cpp20 c++20)
endif()

make_test_with_libs(testDateTime.cpp testDateTime-cpp17 c++17 date::date-tz) 

if(HAS_CPP20_FLAG)
    make_test(testNewFsmStateVariant.cpp testNewFsmStateVariant-cpp20 c++20)
endif()



if(HAS_CPP20_FLAG)
    make_test(testNewFSM.cpp testNewFSM-cpp20 c++20)

    make_test(testNewFsmTransition.cpp testNewFsmTransition-cpp20 c++20)

    make_test_with_includes(testNewFsmContext.cpp testNewFsmContext-cpp20 c++20 ./NewFSM)

    make_test_with_includes(testNewFsmComposite.cpp testNewFsmComposite-cpp20 c++20 ./NewFSM)

    make_test_with_includes(testNewFsmRecursive.cpp testNewFsmRecursive-cpp20 c++20 ./NewFSM)

    make_test(testJsonTokenizer.cpp testJsonTokenizer-cpp20 c++20)

    make_test(testJsonContexts.cpp testJsonContexts-cpp20 c++20)

    make_test(testJsonString.cpp testJsonString-cpp20 c++20)

    make_test(testJsonNumber.cpp testJsonNumber-cpp20 c++20)

    make_test(testJsonObject.cpp testJsonObject-cpp20 c++20)
endif()

#make_test(testNewFsmPublish.cpp testNewFsmPublish-cpp17 c++17)

#make_test(testReflection.cpp testReflection-cpp17 c++17)

#if(HAS_CPP20_FLAG)
#    make_test(testReflection.cpp testReflection-cpp20 c++20)
#endif()

#make_test(testMeta.cpp testMeta-cpp17 c++17)

#if(HAS_CPP20_FLAG)
#    make_test(testMeta.cpp testMeta-cpp20 c++20)
#endif()