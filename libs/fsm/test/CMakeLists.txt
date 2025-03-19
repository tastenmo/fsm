include(${CMAKE_SOURCE_DIR}/cmake/TestHelper.cmake)

assign_test(transition.cpp fsm c++20)
assign_test(state_machine.cpp fsm c++20)
assign_test_with_includes(context.cpp fsm c++20 ./include)
assign_test_with_includes(composite_state.cpp fsm c++20 ./include)
assign_test_with_includes(recursive_state.cpp fsm c++20 ./include)