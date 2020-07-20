// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_STRING_HPP_
#define DBOR_STRING_HPP_

#include <cstdint>
#include "Dbor/ResultCode.hpp"

namespace dbor {

    /**
     * \brief Potentially UTF-8 encoded Unicode string in an assigned immutable non-empty
     * byte buffer of given size.
     *
     * Supports UTF-8 validation according to Unicode Standard 13.0 and conversion.
     *
     * Use \c firstCodepointIn() for validating forward iteration or check and construct an
     * UTF-8 string container with the help of \c getUtf8().
     */
    class String {
    public:

        /**
         * \brief Unicode code point.
         *
         * Valid if in the range 0x0000 ... 0xD7FF or 0xE000 ... 0x10FFFF.
         */
        typedef std::uint_fast32_t CodePoint;

        static constexpr CodePoint INVALID_CODEPOINT = UINT_FAST32_MAX;

        static std::size_t sizeOfUtf8ForCodepoint(CodePoint codePoint) noexcept;  // 0 if not valid

        static CodePoint firstCodepointIn(const std::uint8_t *p, std::size_t capacity,
                                          std::size_t &size) noexcept;
        static std::size_t offsetOfLastCodepointIn(const std::uint8_t *p,
                                                   std::size_t capacity) noexcept;

        String() noexcept;
        String(const uint8_t *buffer, std::size_t size) noexcept;

        /** \brief Assigns empty or non-empty buffer of \c other without owning it. */
        String(const String &other) noexcept = default;

        /** \brief Assigns empty or non-empty buffer of \c other without owning it. */
        String &operator=(const String &other) noexcept = default;

        const uint8_t *buffer() const noexcept;  // nullptr if and only if size() = 0
        std::size_t size() const noexcept;  // = 0 if and only if buffer() = nullptr

        ResultCode check(std::size_t &count, CodePoint &minCodePoint,
                         CodePoint &maxCodePoint) const noexcept;
        ResultCode check() const noexcept;

        ResultCode getAscii(const char *&buffer, std::size_t &size,
                            bool printableOnly = false) const noexcept;

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

/** \brief Return assigned non-empty buffer or \c nullptr. */
inline const uint8_t *dbor::String::buffer() const noexcept {
    return buffer_;
}

/** \brief Return size of assigned non-empty buffer or 0. */
inline std::size_t dbor::String::size() const noexcept {
    return size_;
}


#endif  // DBOR_STRING_HPP_
