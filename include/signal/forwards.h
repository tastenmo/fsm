/**
 * @file forwards.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief 
 * @version 0.1
 * @date 2022-10-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <memory>

namespace signal {

template<typename>
class delegate;

//template<typename = std::allocator<void>>
//class basic_dispatcher;

//template<typename, typename = std::allocator<void>>
//class emitter;

class connection;

struct scoped_connection;

template<typename>
class slot;

template<typename Type, typename = std::allocator<void>>
class signal;

/*! @brief Alias declaration for the most common use case. */
//using dispatcher = basic_dispatcher<>;

} // namespace signal