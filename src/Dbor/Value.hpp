// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_VALUE_HPP_
#define DBOR_VALUE_HPP_

#include "Dbor/Type.hpp"

namespace dbor {

    class Value {
    public:
        Value() noexcept;  // incomplete
        Value(const uint8_t *buffer, std::size_t capacity) noexcept;
        Value(const Value &) noexcept = default;
        Value &operator=(const Value &) noexcept = default;

        const uint8_t *buffer() const noexcept;  // nullptr if and only if size() = 0
        std::size_t size() const noexcept;  // = 0 if and only if buffer() = nullptr

        bool isNone() const noexcept;  // well-formed
        bool isNumberlike() const noexcept;  // well-formed
        bool isNumber() const noexcept;  // well-formed or ill-formed
        bool isString() const noexcept;  // well-formed or ill-formed
        bool isContainer() const noexcept;  // well-formed or ill-formed

        ResultCodes getAsInteger(std::uint8_t &value) const noexcept;
        ResultCodes getAsInteger(std::uint16_t &value) const noexcept;
        ResultCodes getAsInteger(std::uint32_t &value) const noexcept;
        ResultCodes getAsInteger(std::uint64_t &value) const noexcept;

        ResultCodes getAsInteger(std::int8_t &value) const noexcept;
        ResultCodes getAsInteger(std::int16_t &value) const noexcept;
        ResultCodes getAsInteger(std::int32_t &value) const noexcept;
        ResultCodes getAsInteger(std::int64_t &value) const noexcept;

    protected:
        const uint8_t *buffer_;
        std::size_t size_;
    };

}


// Inline implementations ---

inline const uint8_t *dbor::Value::buffer() const noexcept {
    return buffer_;
}


inline std::size_t dbor::Value::size() const noexcept {
    return size_;
}


#endif  // DBOR_VALUE_HPP_
