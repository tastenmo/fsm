/**
 * @file test.cpp
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief 
 * @version 0.1
 * @date 2022-08-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//#include <catch2/catch.hpp>
#include <catch2/catch_test_macros.hpp>
#include <opcua/BuiltInTypes/NodeId.h>

using namespace ua;

TEST_CASE("NodeID numeric")
{
  NodeId node1(13);

  REQUIRE(node1.id() == 13);
  REQUIRE(node1.namespaceIndex() == 0);







  
}

