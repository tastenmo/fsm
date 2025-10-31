include(${CMAKE_SOURCE_DIR}/cmake/TestHelper.cmake)

assign_test(transition.cpp fsm c++20)
assign_test(contextWrapper.cpp fsm c++20)
assign_test(state_machine.cpp fsm c++20)
assign_test_with_includes(context.cpp fsm c++20 ./include)
assign_test_with_includes(composite_state.cpp fsm c++20 ./include)
assign_test_with_includes(composite_state_sep_context.cpp fsm c++20 ./include)
assign_test_with_includes(recursive_state.cpp fsm c++20 ./include)
assign_test(state.cpp fsm c++20)

# After all test assignment calls
get_property(_all_tests GLOBAL PROPERTY FSM_TEST_TARGETS)
add_custom_target(build_all_tests DEPENDS ${_all_tests})