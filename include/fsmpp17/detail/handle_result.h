/**
 * @file handle_result.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief 
 * @version 0.1
 * @date 2022-10-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

namespace fsm::detail
{

struct handled {};
struct not_handled {};
template<class T> struct transition { using type = T; };

} // namespace fsm::detail

