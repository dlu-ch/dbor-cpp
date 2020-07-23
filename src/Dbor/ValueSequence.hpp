// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_VALUE_SEQUENCE_HPP_
#define DBOR_VALUE_SEQUENCE_HPP_

#include <cstdint>
#include "Dbor/Value.hpp"

namespace dbor {

    /**
     * \brief DBOR value block in an empty or non-empty buffer.
     *
     * Supports forward iteration over values:
     * \code
     * const std::uint8_t buffer[] = {0xFF, 12, 0xFE};
     * for (const dbor::Value &v: dbor::ValueSequence(buffer, sizeof(buffer)))
     *     ...
     * \endcode
     */
    class ValueSequence {
    public:
        class Iterator;

        ValueSequence(const void *buffer, std::size_t capacity) noexcept;
        ValueSequence(const ValueSequence &) noexcept = default;
        ValueSequence &operator=(const ValueSequence &) noexcept = delete;

        const void *buffer() const noexcept;
        std::size_t capacity() const noexcept;

        Iterator begin() const noexcept;
        Iterator end() const noexcept;
        bool empty() const noexcept;

    protected:
        const void *const buffer_;
        const std::size_t capacity_;
    };


    class ValueSequence::Iterator {
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

inline dbor::ValueSequence::ValueSequence(const void *buffer, std::size_t capacity) noexcept
    : buffer_(buffer)
    , capacity_(capacity)
{
}


inline const void *dbor::ValueSequence::buffer() const noexcept {
    return buffer_;
}


inline std::size_t dbor::ValueSequence::capacity() const noexcept {
    return capacity_;
}


inline dbor::ValueSequence::Iterator dbor::ValueSequence::begin() const noexcept {
    return ValueSequence::Iterator(buffer_, capacity_);
}


inline dbor::ValueSequence::Iterator dbor::ValueSequence::end() const noexcept {
    return ValueSequence::Iterator();
}


inline bool dbor::ValueSequence::empty() const noexcept {
    return !(buffer_ && capacity_);
}


inline std::size_t dbor::ValueSequence::Iterator::remainingSize() const noexcept {
    return remainingSize_;
}


inline bool dbor::ValueSequence::Iterator::operator!=(const Iterator &other) const noexcept {
    return front_.buffer() != other.front_.buffer();
}


inline bool dbor::ValueSequence::Iterator::operator==(const Iterator &other) const noexcept {
    return !operator!=(other);
}


inline bool dbor::ValueSequence::Iterator::isAtEnd() const noexcept {
    return !front_.buffer();
}


inline const dbor::Value &dbor::ValueSequence::Iterator::operator*() const noexcept {
    return front_;
}


inline const dbor::Value *dbor::ValueSequence::Iterator::operator->() const noexcept {
    return &front_;
}


#endif  // DBOR_VALUE_SEQUENCE_HPP_
