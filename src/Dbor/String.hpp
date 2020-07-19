// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_STRING_HPP_
#define DBOR_STRING_HPP_

#include <cstdint>
#include "Dbor/ResultCode.hpp"

namespace dbor {

    class String {
    public:

        // Unicode code point (valid if in the range 0x0000 ... 0xD7FF or 0xE000 ... 0x10FFFF)
        typedef std::uint_fast32_t CodePoint;
        static constexpr CodePoint INVALID_CODEPOINT = UINT_FAST32_MAX;

        static std::size_t sizeOfUtf8ForCodepoint(CodePoint codePoint) noexcept;  // 0 if not valid

        // Returns first well-formed UTF-8 encoded code point in p or INVALID_CODEPOINT if none
        // (empty or not well-formed).
        // size is 0 if empty and in the range 1 .. min(4, capacity) otherwise.
        // Empty if and only if p = nullptr or capacity = 0.
        static CodePoint firstCodepointIn(const std::uint8_t *p, std::size_t capacity,
                                          std::size_t &size) noexcept;
        static std::size_t offsetOfLastCodepointIn(const std::uint8_t *p,
                                                   std::size_t capacity) noexcept;

        String() noexcept;  // empty
        String(const uint8_t *buffer, std::size_t size) noexcept;  // safe to use with any content
        String(const String &) noexcept = default;
        String &operator=(const String &) noexcept = default;

        const uint8_t *buffer() const noexcept;  // nullptr if and only if size() = 0
        std::size_t size() const noexcept;  // = 0 if and only if buffer() = nullptr

        // minCodePoint, maxCodePoint are the minimum and maximum code point, respectively,
        // if ResultCode::OK and size() > 0, and INVALID_CODEPOINT otherwise.
        // ResultCode::OK or ResultCode::ILLFORMED.
        ResultCode check(std::size_t &count, CodePoint &minCodePoint,
                         CodePoint &maxCodePoint) const noexcept;
        ResultCode check() const noexcept;

        // p[0] ... p[size - 1] is an ASCII string of characters with ASCII code in the
        // range 0x20 ... 0x7E if printableOnly is true and in the range 0x00 ... 0x7F otherwise.
        // p is not necessarily NUL terminated.
        // ResultCode::OK, ResultCode::ILLFORMED, ResultCode::RANGE.
        ResultCode getAscii(const char *&buffer, std::size_t &size,
                            bool printableOnly = false) const noexcept;

        // p[0] ... p[size - 1] is a well-formed UTF-8 string of (valid) code points
        // in the range minCodePoint .. maxCodePoint.
        // p is not necessarily NUL terminated.
        // ResultCode::OK, ResultCode::ILLFORMED, ResultCode::RANGE.
        ResultCode getUtf8(const std::uint8_t *&buffer, std::size_t &size,
                           CodePoint minCodePoint, CodePoint maxCodePoint) const noexcept;

    protected:
        static CodePoint firstCodepointInNonEmpty(const std::uint8_t *p, std::size_t capacity,
                                                  std::size_t &size) noexcept;

        ResultCode checkNonEmpty(std::size_t &count,
                                 CodePoint &minCodePoint, CodePoint &maxCodePoint) const noexcept;

        const std::uint8_t *buffer_;
        std::size_t size_;  // number of bytes
    };

}


// Inline implementations ---

inline const uint8_t *dbor::String::buffer() const noexcept {
    return buffer_;
}


inline std::size_t dbor::String::size() const noexcept {
    return size_;
}


#endif  // DBOR_STRING_HPP_
