include(${CMAKE_SOURCE_DIR}/cmake/TestHelper.cmake)

assign_test(tokenizer.cpp json c++20)
assign_test(contexts.cpp json c++20)
assign_test(string.cpp json c++20)
assign_test(number.cpp json c++20)
assign_test(object.cpp json c++20)
assign_test(array.cpp json c++20)