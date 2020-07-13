// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_VALUE_HPP_
#define DBOR_VALUE_HPP_

#include "Dbor/ResultCodes.hpp"
#include "Dbor/String.hpp"

namespace dbor {

    class Value {
    public:
        Value() noexcept;  // incomplete with size() = 0
        Value(const uint8_t *buffer, std::size_t capacity) noexcept;  // first in buffer
        Value(const Value &) noexcept = default;
        Value &operator=(const Value &) noexcept = default;

        const uint8_t *buffer() const noexcept;  // nullptr if and only if size() = 0
        std::size_t size() const noexcept;  // = 0 if and only if buffer() = nullptr
        bool isComplete() const noexcept; // true only if size() > 0

        bool isNone() const noexcept;
        bool isNumberlike() const noexcept;
        bool isNumber() const noexcept;  // well-formed or ill-formed/incomplete
        bool isString() const noexcept;  // well-formed or ill-formed/incomplete
        bool isContainer() const noexcept;  // well-formed or ill-formed/incomplete

        ResultCodes get(std::uint8_t &value) const noexcept;
        ResultCodes get(std::uint16_t &value) const noexcept;
        ResultCodes get(std::uint32_t &value) const noexcept;
        ResultCodes get(std::uint64_t &value) const noexcept;
        ResultCodes get(std::int8_t &value) const noexcept;
        ResultCodes get(std::int16_t &value) const noexcept;
        ResultCodes get(std::int32_t &value) const noexcept;
        ResultCodes get(std::int64_t &value) const noexcept;

        ResultCodes get(const std::uint8_t *&bytes, std::size_t &size) const noexcept;

        // ResultCodes::OK does not mean that this value is a well-formed Utf8StringValue.
        // Use string.check() or string.getXXX() in addition.
        ResultCodes get(String &string, std::size_t maxSize) const noexcept;

    protected:
        const uint8_t *buffer_;
        std::size_t size_;
        bool isComplete_;
    };

}


// Inline implementations ---

inline const uint8_t *dbor::Value::buffer() const noexcept {
    return buffer_;
}


inline std::size_t dbor::Value::size() const noexcept {
    return size_;
}


inline bool dbor::Value::isComplete() const noexcept {
    return isComplete_;
}


#endif  // DBOR_VALUE_HPP_
