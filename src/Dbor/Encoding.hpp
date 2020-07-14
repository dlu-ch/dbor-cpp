// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_ENCODING_HPP_
#define DBOR_ENCODING_HPP_

#include <cstdint>

namespace dbor {

    class Encoding {
    public:

        enum class ValueType: std::uint_least8_t {
            INVALID_OR_RESERVED = 0,

            NONE,
            INTEGER,  // NUMBER
            BYTE_STRING,  // STRING
            UTF8_STRING,  // STRING
            SEQUENCE,  // CONTAINER
            DICTIONARY,  // CONTAINER
            ALLOCATED,  // CONTAINER
            BINARY_RATIONAL,  // NUMBER
            DECIMAL_RATIONAL,  // NUMBER
            NUMBERLIKE,
        };

        enum class SingleByteValue: std::uint8_t {
            MINUS_ZERO = 0xFCu,
            MINUS_INF  = 0xFDu,
            INF        = 0xFEu,
            NONE       = 0xFFu
        };

        static std::size_t sizeOfTokenFromFirstByte(std::uint8_t b) noexcept;  // returns 1 .. 9

        // Size of first (well-formed or ill-formed) value in buffer p[0] ... p[capacity - 1] or
        // 0 or buffer is too small to calculate size (size would depends on bytes after the end
        // of the buffer).
        static std::size_t sizeOfValueIn(const std::uint8_t *p, std::size_t capacity) noexcept;


        static bool decodeNaturalTokenData(std::uint16_t &value,
                                           const std::uint8_t *p, std::size_t n,
                                           std::uint32_t offset) noexcept;
        static bool decodeNaturalTokenData(std::uint32_t &value,
                                           const std::uint8_t *p, std::size_t n,
                                           std::uint32_t offset) noexcept;
        static bool decodeNaturalTokenData(std::uint64_t &value,
                                           const std::uint8_t *p, std::size_t n,
                                           std::uint32_t offset) noexcept;

        static std::size_t encodeNaturalTokenData(const std::uint16_t &value,
                                                  std::uint8_t *p, std::size_t capacity) noexcept;
        static std::size_t encodeNaturalTokenData(const std::uint32_t &value,
                                                  std::uint8_t *p, std::size_t capacity) noexcept;
        static std::size_t encodeNaturalTokenData(const std::uint64_t &value,
                                                  std::uint8_t *p, std::size_t capacity) noexcept;

        static std::uint32_t decodeBinaryRationalTokenDataAs32b(const std::uint8_t *p,
                                                                std::size_t k) noexcept;
        static std::uint64_t decodeBinaryRationalTokenDataAs64b(const std::uint8_t *p,
                                                                std::size_t k) noexcept;
        static std::uint64_t convertBinaryRational32bTo64b(std::uint32_t value) noexcept;
    };

}


// Inline implementations ---

inline std::size_t dbor::Encoding::sizeOfTokenFromFirstByte(std::uint8_t b) noexcept {
    // 000xxxxx  IntegerValue
    // 001xxxxx  IntegerValue
    // 010xxxxx  ByteStringValue
    // 011xxxxx  Utf8StringValue
    // 100xxxxx  SequenceValue
    // 101xxxxx  DictionaryValue
    // 11000yyy  AllocatedValue
    // 11001yyy  BinaryRationalValue
    // 1101xyyy  DecimalRationalValue(..., e) with |e| > 8
    // 1110xxxx  DecimalRationalValue(..., e) with |e| <= 8
    // 1111xxxx  MinimalToken

    return (b >= 0xE0 || (b < 0xC0 && (b & 0x1F) < 0x18)) ? 1 : 2 + (b & 7);
}


#endif  // DBOR_ENCODING_HPP_