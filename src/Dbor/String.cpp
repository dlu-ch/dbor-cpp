// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/String.hpp"

// UTF-8 encoding:
// https://www.unicode.org/versions/Unicode13.0.0/ch03.pdf#G31703

using namespace dbor;


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


std::size_t String::offsetOfLastCodepointIn(const std::uint8_t *p,
                                            std::size_t capacity) noexcept
{
    if (!p || !capacity)
        return 0;

    std::size_t offset = capacity - 1u;

    // at most 3 bytes back until p[offset] is other than 10xxxxxx
    std::uint_fast8_t n = offset < 3 ? offset : 3;
    while (n-- > 0 && (p[offset] & 0xC0) == 0x80)
        offset--;

    return offset;
}


String::String() noexcept
    : buffer_(nullptr)
    , size_(0)
{
}


String::String(const uint8_t *buffer, std::size_t size) noexcept
    : buffer_(size ? buffer : nullptr)
    , size_(buffer ? size : 0)
{
}


ResultCodes String::getAsAscii(const char *&buffer, std::size_t &size,
                               bool printableOnly) const noexcept
{
    ResultCodes r = ResultCodes::OK;

    if (size_) {
        std::size_t n;
        CodePoint minCodePoint, maxCodePoint;
        r = checkNonEmpty(n, minCodePoint, maxCodePoint);
        if (r == ResultCodes::OK) {
            if (printableOnly ? minCodePoint >= 0x20 && maxCodePoint < 0x7F : maxCodePoint < 0x80) {
                buffer = reinterpret_cast<const char *>(buffer_);  // may be nullptr
                size = size_;
                return r;
            }
            r = ResultCodes::INCOMPATIBLE;
        }
    }

    buffer = nullptr;
    size = 0;
    return r;
}


ResultCodes String::getAsUtf8(const std::uint8_t *&buffer, std::size_t &size,
                              CodePoint minCodePoint, CodePoint maxCodePoint) const noexcept
{
    ResultCodes r = ResultCodes::OK;

    if (size_) {
        std::size_t n;
        CodePoint minc, maxc;
        r = checkNonEmpty(n, minc, maxc);
        if (r == ResultCodes::OK) {
            if (minc >= minCodePoint && maxc <= maxCodePoint) {
                buffer = buffer_;  // may be nullptr
                size = size_;
                return r;
            }
            r = ResultCodes::INCOMPATIBLE;
        }
    }

    buffer = nullptr;
    size = 0;
    return r;
}


ResultCodes String::checkNonEmpty(std::size_t &count,
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
            return ResultCodes::ILLFORMED;
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
            return ResultCodes::OK;
        }
    }
}


ResultCodes String::check(std::size_t &count,
                          CodePoint &minCodePoint, CodePoint &maxCodePoint) const noexcept
{
    if (!size_) {
        count = 0;
        minCodePoint = String::INVALID_CODEPOINT;
        maxCodePoint = String::INVALID_CODEPOINT;
        return ResultCodes::OK;
    }

    return checkNonEmpty(count, minCodePoint, maxCodePoint);
}


ResultCodes String::check() const noexcept {
    std::size_t count;
    CodePoint minCodePoint, maxCodePoint;
    return check(count, minCodePoint, maxCodePoint);
}
