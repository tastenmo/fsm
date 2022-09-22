/**
 * @file NodeIdBase.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2022-09-16
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

namespace ua
{
      template <class TValue>
        class NodeId
        {
        public:
            explicit NodeId(TValue &&value, uint16_t namespaceIndex = 0) : value_(std::forward<TValue>(value)), namespaceIndex_(namespaceIndex)
            {
                ;
            }

            enum class IdType: uint8_t { UInt32, String, Guid, ByteString};

            TValue id() const
            {
                return value_;
            }

            void id(TValue &&value)
            {
                value_ = std::forward<TValue>(value);
            }

            void namespaceIndex(uint16_t namespaceIndex)
            {
                namespaceIndex_ = namespaceIndex;
            }

            uint16_t namespaceIndex(void) const
            {
                return namespaceIndex_;
            };

        private:
            IdType IdType_;
            TValue value_;
            uint16_t namespaceIndex_;
            
        };

        template <class TValue>
        NodeId(const TValue &&, uint16_t) -> NodeId<TValue>; // deduction guide

    }
