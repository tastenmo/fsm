/**
 * @file assert.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief 
 * @version 0.1
 * @date 2022-10-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once

#include <cassert>

#define FSM_ASSERT(EXPR, MSG) std::cout << "Assertion-Beschreibung: " MSG << std::endl; assert(EXPR)