include(${CMAKE_SOURCE_DIR}/cmake/TestHelper.cmake)

assign_test(testDelegates.cpp signal c++17)

assign_test(testSignalSlot.cpp signal c++17)

assign_test(testEmitter.cpp signal c++17)

assign_test(testDispatcher.cpp signal c++17)
