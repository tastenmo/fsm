include(${CMAKE_SOURCE_DIR}/cmake/TestHelper.cmake)

assign_test_with_libs(property.cpp reflect c++20 ctre::ctre)
