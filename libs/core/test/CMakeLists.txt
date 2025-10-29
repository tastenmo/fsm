include(${CMAKE_SOURCE_DIR}/cmake/TestHelper.cmake)

assign_test(type_traits.cpp core c++17)

assign_test_with_libs(property.cpp core c++20 ctre::ctre)

