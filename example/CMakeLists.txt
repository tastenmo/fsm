include(CheckCXXCompilerFlag)

if((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    set(OPTIONS -Werror -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(OPTIONS /W4 /WX)
    if(HAS_PERMISSIVE_FLAG)
        set(OPTIONS ${OPTIONS} /permissive-)
    endif()
endif()

function(make_example target)
    add_executable(${target} ${target}.cpp)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    #target_compile_features(${target} PRIVATE cxx_std_17)
    target_compile_options(${target} PRIVATE ${OPTIONS})
    target_link_libraries(${target} PRIVATE ${CMAKE_PROJECT_NAME})
endfunction()

make_example(example)
make_example(simple_fsm)
