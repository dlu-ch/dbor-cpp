// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_VALUE_BLOCK_HPP_
#define DBOR_VALUE_BLOCK_HPP_

#include <cstdint>
#include "Dbor/Value.hpp"

namespace dbor {

    /**
     * \brief DBOR value block in an empty or non-empty buffer.
     *
     * Supports forward iteration over values:
     * \code
     * const std::uint8_t buffer[] = {0xFF, 12, 0xFE};
     * for (const dbor::Value &v: dbor::ValueBlock(buffer, sizeof(buffer)))
     *     ...
     * \endcode
     */
    class ValueBlock {
    public:
        class Iterator;

        ValueBlock(const void *buffer, std::size_t capacity) noexcept;
        ValueBlock(const ValueBlock &) noexcept = default;
        ValueBlock &operator=(const ValueBlock &) noexcept = delete;

        const void *buffer() const noexcept;
        std::size_t capacity() const noexcept;

        Iterator begin() const noexcept;
        Iterator end() const noexcept;
        bool empty() const noexcept;

    protected:
        const void *const buffer_;
        const std::size_t capacity_;
    };


    class ValueBlock::Iterator {
    public:
        Iterator() noexcept;
        Iterator(const void *buffer, std::size_t capacity) noexcept;
        Iterator(const Iterator &) noexcept = default;
        Iterator &operator=(const Iterator &) noexcept = default;

        bool operator==(const Iterator &other) const noexcept;
        bool operator!=(const Iterator &other) const noexcept;

        Iterator &operator++() noexcept;
        // no post-decrement operator on purpuse (would be slow)
        const Value &operator*() const noexcept;  // returned value "moves" with the iterator
        const Value *operator->() const noexcept;  // returned value "moves" with the iterator

        bool isAtEnd() const noexcept;
        std::size_t remainingSize() const noexcept;

    protected:
        Value front_;
        std::size_t remainingSize_;  // remaining size after front_
    };
}


// Inline implementations ---

inline dbor::ValueBlock::ValueBlock(const void *buffer, std::size_t capacity) noexcept
    : buffer_(buffer)
    , capacity_(capacity)
{
}


inline const void *dbor::ValueBlock::buffer() const noexcept {
    return buffer_;
}


inline std::size_t dbor::ValueBlock::capacity() const noexcept {
    return capacity_;
}


inline dbor::ValueBlock::Iterator dbor::ValueBlock::begin() const noexcept {
    return ValueBlock::Iterator(buffer_, capacity_);
}


inline dbor::ValueBlock::Iterator dbor::ValueBlock::end() const noexcept {
    return ValueBlock::Iterator();
}


inline bool dbor::ValueBlock::empty() const noexcept {
    return !(buffer_ && capacity_);
}


inline std::size_t dbor::ValueBlock::Iterator::remainingSize() const noexcept {
    return remainingSize_;
}


inline bool dbor::ValueBlock::Iterator::operator!=(const Iterator &other) const noexcept {
    return front_.buffer() != other.front_.buffer();
}


inline bool dbor::ValueBlock::Iterator::operator==(const Iterator &other) const noexcept {
    return !operator!=(other);
}


inline bool dbor::ValueBlock::Iterator::isAtEnd() const noexcept {
    return !front_.buffer();
}


inline const dbor::Value &dbor::ValueBlock::Iterator::operator*() const noexcept {
    return front_;
}


inline const dbor::Value *dbor::ValueBlock::Iterator::operator->() const noexcept {
    return &front_;
}


#endif  // DBOR_VALUE_BLOCK_HPP_
