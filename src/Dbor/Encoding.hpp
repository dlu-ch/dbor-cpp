// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_ENCODING_HPP_
#define DBOR_ENCODING_HPP_

#include <cstdint>
#include "Dbor/Type.hpp"

namespace dbor {

    class Encoding {
    public:

        enum class ValueType: std::uint_least8_t {
            INVALID_OR_RESERVED = 0,

            NONE = 1,

            INTEGER,
            BINARY_RATIONAL,
            DECIMAL_RATIONAL,
            MINUS_ZERO,
            INF,  // INFINITY is a macro in <cmath>
            MINUS_INF,

            BYTE_STRING,
            UTF8_STRING,

            SEQUENCE,
            DICTIONARY,
            ALLOCATED
        };

        enum class MinimalTokenNonHeader: std::uint_least8_t {
            MINUS_ZERO = 0x1Cu,
            MINUS_INF  = 0x1Du,
            INF        = 0x1Eu,
            NONE       = 0x1Fu
        };

        static std::uint_least8_t sizeInfoFromFirstByte(std::uint8_t b) noexcept;

        static ErrorCode decodeNaturalTokenData(std::uint16_t &value,
                                                const std::uint8_t *p, std::size_t n,
                                                std::uint32_t) noexcept;
        static ErrorCode decodeNaturalTokenData(std::uint32_t &value,
                                                const std::uint8_t *p, std::size_t n,
                                                std::uint32_t) noexcept;
        static ErrorCode decodeNaturalTokenData(std::uint64_t &value,
                                                const std::uint8_t *p, std::size_t n,
                                                std::uint32_t offset) noexcept;

        static std::size_t encodeNaturalTokenData(const std::uint16_t &value,
                                                  std::uint8_t *p, std::size_t capacity) noexcept;
        static std::size_t encodeNaturalTokenData(const std::uint32_t &value,
                                                  std::uint8_t *p, std::size_t capacity) noexcept;
        static std::size_t encodeNaturalTokenData(const std::uint64_t &value,
                                                  std::uint8_t *p, std::size_t capacity) noexcept;
    };

};

#endif  // DBOR_ENCODING_HPP_