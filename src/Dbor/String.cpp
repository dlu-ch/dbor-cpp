// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/String.hpp"

// UTF-8 encoding:
// https://www.unicode.org/versions/Unicode13.0.0/ch03.pdf#G31703

using namespace dbor;


/**
 * \brief Returns the number of byte of the UTF-8 encoded code point \c codePoint
 * or 0 if it is not a valid code point.
 * A code point if valid if and only if it is in the range 0x0000 .. 0xD7FF or 0xE000 .. 0x10FFFF.
 */
std::size_t String::sizeOfUtf8ForCodepoint(CodePoint codePoint) noexcept {
    if (codePoint < 0x80)
        return 1;
    if (codePoint < 0x800)
        return 2;
    if (codePoint < 0x10000) {
        if (codePoint >= 0xD800 && codePoint <= 0xDFFF)
            return 0;
        return 3;
    }
    if (codePoint < 0x110000)
        return 4;
    return 0;
}


/**
 * \brief Returns the first well-formed UTF-8 encoded code point in the buffer
 * or \c INVALID_CODEPOINT if none (not well-formed or buffer empty).
 *
 * The buffer is considered empty if and only if \c p = \c nullptr or \c capacity = 0.
 *
 * \param[in] p
 *   Pointer to the first byte of the buffer or \c nullptr.
 * \param[in] capacity
 *   Size of the buffer in byte.
 * \param[out] size
 *   0 if buffer empty and in the range 1 .. min(4, \c capacity) otherwise.
 * \return
 *   Valid code point or \c INVALID_CODEPOINT
 */
String::CodePoint String::firstCodepointIn(const std::uint8_t *p, std::size_t capacity,
                                           std::size_t &size) noexcept
{
    if (p && capacity)
        return firstCodepointInNonEmpty(p, capacity, size);

    size = 0;
    return INVALID_CODEPOINT;
}


String::CodePoint String::firstCodepointInNonEmpty(const std::uint8_t *p, std::size_t capacity,
                                                   std::size_t &size) noexcept
{
    const std::uint8_t first = *p;
    if (first < 0b10000000) {
        size = 1;
        return first;
    }

    if (first < 0b11000000 || first >= 0b11111000) {
        // invalid as first byte, cannot determine size
        size = 1;
        return INVALID_CODEPOINT;
    }

    // first      h      n
    // ---------  -----  --
    //
    // 110xxxxx   0010   2
    // 1110xxxx   0001   3
    // 11110xxx   0000   4

    const std::size_t h = (~first >> 4u) & (first >> 5u);  // 0 .. 2
    const std::size_t n = 4u - h;

    if (n > capacity) {
        size = capacity;
        return INVALID_CODEPOINT;
    }

    CodePoint c = first & (0x7F >> n);
    const std::uint8_t *q = p;
    std::size_t i = 1;
    for (;;) {
        const std::uint8_t b = *++q;  // 10xxxxxx if well-formed
        if ((b & 0xC0) != 0x80) {
            size = i;
            return INVALID_CODEPOINT;
        }
        c = (c << 6u) | (b & 0x3F);
        if (++i >= n)
            break;
    }

    size = n;
    if (n != sizeOfUtf8ForCodepoint(c))
        return INVALID_CODEPOINT;

    return c;
}


/**
 * \brief Returns the offset of the beginning of the last potential UTF-8 encoded code point in the
 * buffer.
 * \return 0 if empty and value in max(0, \c capacity - 3) .. \c capacity - 1 otherwise.
 */
std::size_t String::offsetOfLastCodepointIn(const std::uint8_t *p, std::size_t capacity) noexcept {
    if (!p || !capacity)
        return 0;

    std::size_t offset = capacity - 1u;

    // at most 3 bytes back until p[offset] is other than 10xxxxxx
    std::uint_fast8_t n = offset < 3 ? offset : 3;
    while (n-- > 0 && (p[offset] & 0xC0) == 0x80)
        offset--;

    return offset;
}


/**
 * \brief Assigns an empty buffer.
 */
String::String() noexcept
    : buffer_(nullptr)
    , size_(0)
{
}


/**
 * \brief Assigns an empty or non-empty buffer without owning it.
 * The buffer must remain unchanged as long as this instance exists.
 */
String::String(const uint8_t *buffer, std::size_t size) noexcept
    : buffer_(size ? buffer : nullptr)
    , size_(buffer ? size : 0)
{
}


/**
 * \brief Gets the assigned buffer as ASCII string if it is empty or contains a well-formed UTF-8
 * encoded Unicode string of (printable) ASCII characters only.
 *
 * \param[in] printableOnly
 *   Accept only printable ASCII characters?
 * \param[out] buffer
 *   Pointer to non-empty ASCII string (not necessarily NUL terminated) or \c nullptr.
 *   If not \c nullptr, \c p[0] .. \c p[size - 1] are ASCII characters with characters code
 *   in the range 0x20 .. 0x7E if \c printableOnly is \c true and in the range 0x00 .. 0x7F
 *   otherwise.
 * \param[out] size Size of ASCII string or 0.
 * \return
 *   ResultCode::OK if well-formed and all code points in the requested range,
 *   ResultCode::RANGE if well-formed and @em not all code points in the requested range,
 *   ResultCode::ILLFORMED if ill-formed.
 */
ResultCode String::getAscii(const char *&buffer, std::size_t &size,
                            bool printableOnly) const noexcept
{
    // C++:2011: char is at least 7 bit (signed or unsigned)

    ResultCode r = ResultCode::OK;

    if (size_) {
        std::size_t n;
        CodePoint minCodePoint, maxCodePoint;
        r = checkNonEmpty(n, minCodePoint, maxCodePoint);
        if (r == ResultCode::OK) {
            if (printableOnly ? minCodePoint >= 0x20 && maxCodePoint < 0x7F : maxCodePoint < 0x80) {
                static_assert(sizeof(char) == sizeof(std::uint8_t), "");
                buffer = reinterpret_cast<const char *>(buffer_);  // may be nullptr
                size = size_;
                return r;
            }
            r = ResultCode::RANGE;
        }
    }

    buffer = nullptr;
    size = 0;
    return r;
}


/**
 * \brief Gets the assigned buffer as ASCII string if it is empty or contains a well-formed UTF-8
 * encoded Unicode string of code points in the specified range only.
 *
 * \param[in] minCodePoint Minimum accepted code point.
 * \param[in] maxCodePoint Maximum accepted code point.
 * \param[out] buffer
 *   Pointer to non-empty ASCII string (not necessarily NUL terminated) or \c nullptr.
 *   If not \c nullptr, \c p[0] .. \c p[size - 1] is a well-formed UTF-8 string of valid
 *   code points in the range \c minCodePoint .. \c maxCodePoint.
 * \param[out] size Size of ASCII string or 0.
 * \return
 *   ResultCode::OK if well-formed and all code points in the requested range,
 *   ResultCode::RANGE if well-formed and @em not all code points in the requested range,
 *   ResultCode::ILLFORMED if ill-formed.
 */
ResultCode String::getUtf8(const std::uint8_t *&buffer, std::size_t &size,
                           CodePoint minCodePoint, CodePoint maxCodePoint) const noexcept
{
    ResultCode r = ResultCode::OK;

    if (size_) {
        std::size_t n;
        CodePoint minc, maxc;
        r = checkNonEmpty(n, minc, maxc);
        if (r == ResultCode::OK) {
            if (minc >= minCodePoint && maxc <= maxCodePoint) {
                buffer = buffer_;  // may be nullptr
                size = size_;
                return r;
            }
            r = ResultCode::RANGE;
        }
    }

    buffer = nullptr;
    size = 0;
    return r;
}


ResultCode String::checkNonEmpty(std::size_t &count,
                                 CodePoint &minCodePoint,
                                 CodePoint &maxCodePoint) const noexcept
{
    CodePoint minc = String::INVALID_CODEPOINT;
    CodePoint maxc = 0;

    const std::uint8_t *p = buffer_;
    std::size_t unprocessed = size_;
    std::size_t n = 0;

    for (;;) {
        std::size_t len;
        CodePoint c = firstCodepointInNonEmpty(p, unprocessed, len);
        if (c == String::INVALID_CODEPOINT) {
            count = 0;
            minCodePoint = String::INVALID_CODEPOINT;
            maxCodePoint = String::INVALID_CODEPOINT;
            return ResultCode::ILLFORMED;
        }
        // len <= unprocessed
        // c <= 0x10FFFF

        if (minc > c)
            minc = c;
        if (maxc < c)
            maxc = c;

        p += len;
        unprocessed -= len;
        n++;

        if (unprocessed == 0) {
            count = n;
            minCodePoint = minc;
            maxCodePoint = maxc;
            return ResultCode::OK;
        }
    }
}


/**
 * \brief Checks if the assigned buffer is empty or contains a well-formed UTF-8 encoded Unicode
 * string and returns information on the code points in the Unicode string.
 *
 * \param[out] count Number of codepoints in the buffer.
 * \param[out] minCodePoint
 *   Minimum code point if buffer if non-empty and valid and \c INVALID_CODEPOINT otherwise.
 * \param[out] maxCodePoint
 *   Maximum code point if buffer if non-empty and valid and \c INVALID_CODEPOINT otherwise.
 * \return \c ResultCode::OK if empty or well-formed and \c ResultCode::ILLFORMED otherwise
 */
ResultCode String::check(std::size_t &count,
                         CodePoint &minCodePoint, CodePoint &maxCodePoint) const noexcept
{
    if (!size_) {
        count = 0;
        minCodePoint = String::INVALID_CODEPOINT;
        maxCodePoint = String::INVALID_CODEPOINT;
        return ResultCode::OK;
    }

    return checkNonEmpty(count, minCodePoint, maxCodePoint);
}


/**
 * \brief Checks if the assigned buffer is empty or contains a well-formed UTF-8 encoded Unicode
 * string.
 *
 * \return \c ResultCode::OK if empty or well-formed and \c ResultCode::ILLFORMED otherwise
 */
ResultCode String::check() const noexcept {
    std::size_t count;
    CodePoint minCodePoint, maxCodePoint;
    return check(count, minCodePoint, maxCodePoint);
}
