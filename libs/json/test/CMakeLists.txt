include(${CMAKE_SOURCE_DIR}/cmake/TestHelper.cmake)

assign_test(tokenizer.cpp json c++20)
assign_test(contexts.cpp json c++20)
assign_test(string.cpp json c++20)
assign_test(number.cpp json c++20)
assign_test(object.cpp json c++20)
assign_test(array.cpp json c++20)
assign_test(value.cpp json c++20)
assign_test(test_real_data.cpp json c++20)
# assign_test(benchmark_nativejson.cpp json c++20)  # skipped from normal regression runs
