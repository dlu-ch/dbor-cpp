// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestString.hpp"
#include "Test.hpp"
#include "Dbor/String.hpp"


// Usage: StringBuilder{0x00, 0x12}.string
struct StringBuilder: public test::ByteBufferBuilder {
    StringBuilder(std::initializer_list<std::uint8_t> bytes)
        : test::ByteBufferBuilder(bytes)
        , string(buffer(), size())
    {}

    const dbor::String string;
};


static void testSizeOfUtf8ForCodepoint() {
    const auto f = dbor::String::sizeOfUtf8ForCodepoint;

    ASSERT_EQUAL(1, f(0x0000));
    ASSERT_EQUAL(1, f(0x007F));

    ASSERT_EQUAL(2, f(0x0080));
    ASSERT_EQUAL(2, f(0x07FF));

    ASSERT_EQUAL(3, f(0x0800));
    ASSERT_EQUAL(3, f(0xD7FF));
    ASSERT_EQUAL(0, f(0xD800));
    ASSERT_EQUAL(0, f(0xDFFF));
    ASSERT_EQUAL(3, f(0xE000));
    ASSERT_EQUAL(3, f(0xFFFF));

    ASSERT_EQUAL(4, f(0x010000));
    ASSERT_EQUAL(4, f(0x10FFFF));

    ASSERT_EQUAL(0, f(0x110000));
    ASSERT_EQUAL(0, f(UINT_FAST32_MAX));
    ASSERT_EQUAL(0, f(dbor::String::INVALID_CODEPOINT));
}


static void testFirstCodepointIn() {

    struct Builder: public test::ByteBufferBuilder {
        Builder(std::initializer_list<std::uint8_t> bytes) : test::ByteBufferBuilder(bytes) {}

        dbor::String::CodePoint firstCodepointIn(std::size_t &n) const noexcept {
            return dbor::String::firstCodepointIn(buffer(), size(), n);
        }
    };

    std::size_t n;

    n = 7;
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT, dbor::String::firstCodepointIn(nullptr, 12, n));
    ASSERT_EQUAL(0, n);

    n = 7;
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT, Builder{}.firstCodepointIn(n));
    ASSERT_EQUAL(0, n);

    // well-formed UTF-8:

    ASSERT_EQUAL(0x0000, Builder{0x00}.firstCodepointIn(n));
    ASSERT_EQUAL(1, n);
    ASSERT_EQUAL(0x007F, Builder{0x7F}.firstCodepointIn(n));
    ASSERT_EQUAL(1, n);

    ASSERT_EQUAL(0x0080, (Builder{0xC2, 0x80}).firstCodepointIn(n));
    ASSERT_EQUAL(2, n);
    ASSERT_EQUAL(0x07FF, (Builder{0xDF, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(2, n);

    ASSERT_EQUAL(0x0800, (Builder{0xE0, 0xA0, 0x80}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);
    ASSERT_EQUAL(0xD7FF, (Builder{0xED, 0x9F, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);
    ASSERT_EQUAL(0xE000, (Builder{0xEE, 0x80, 0x80}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);
    ASSERT_EQUAL(0xFFFF, (Builder{0xEF, 0xBF, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);

    ASSERT_EQUAL(0x10000, (Builder{0xF0, 0x90, 0x80, 0x80}).firstCodepointIn(n));
    ASSERT_EQUAL(4, n);
    ASSERT_EQUAL(0x10FFFF, (Builder{0xF4, 0x8F, 0xBF, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(4, n);

    // well-formed, too short

    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xC2}).firstCodepointIn(n));
    ASSERT_EQUAL(1, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xE0}).firstCodepointIn(n));
    ASSERT_EQUAL(1, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xEF, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(2, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xF0, 0x90}).firstCodepointIn(n));
    ASSERT_EQUAL(2, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xF4, 0x8F, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);

    // ill-formed: invalid first byte

    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0b10000000}).firstCodepointIn(n));
    ASSERT_EQUAL(1, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0b11111000}).firstCodepointIn(n));
    ASSERT_EQUAL(1, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0b11111111}).firstCodepointIn(n));
    ASSERT_EQUAL(1, n);

    // ill-formed: invalid next byte

    n = 7;
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xF4, 0x00, 0xBF, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(1, n);
    n = 7;
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xF4, 0xF4, 0xBF, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(1, n);
    n = 7;
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xF4, 0xFF, 0xBF, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(1, n);

    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xF4, 0x8F, 0xBF, 0x00}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);
    n = 7;
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xF4, 0x8F, 0xBF, 0xF4}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);
    n = 7;
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xF4, 0x8F, 0xBF, 0xFF}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);

    // ill-formed: invalid code point

    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xF4, 0x90, 0x80, 0x80}).firstCodepointIn(n));
    ASSERT_EQUAL(4, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xED, 0xA0, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);
    n = 7;
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xED, 0xBF, 0xBF}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);

    // ill-formed: not shortest form
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xC0, 0x80}).firstCodepointIn(n));
    ASSERT_EQUAL(2, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xE0, 0x80, 0x80}).firstCodepointIn(n));
    ASSERT_EQUAL(3, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT,
                 (Builder{0xF0, 0x80, 0x80, 0x80}).firstCodepointIn(n));
    ASSERT_EQUAL(4, n);
}


static void testOffsetOfLastCodepointIn() {

    struct Builder: public test::ByteBufferBuilder {
        Builder(std::initializer_list<std::uint8_t> bytes) : test::ByteBufferBuilder(bytes) {}

        dbor::String::CodePoint offsetOfLastCodepointIn() const noexcept {
            return dbor::String::offsetOfLastCodepointIn(buffer(), size());
        }
    };

    // empty

    ASSERT_EQUAL(0u, dbor::String::offsetOfLastCodepointIn(nullptr, 1));
    ASSERT_EQUAL(0u, Builder{}.offsetOfLastCodepointIn());

    // well-formed

    ASSERT_EQUAL(0u, Builder{0x00}.offsetOfLastCodepointIn());
    ASSERT_EQUAL(0u, (Builder{0xF4, 0x8F, 0xBF, 0xBF}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(1u, (Builder{0x7F, 0xF4, 0x8F, 0xBF, 0xBF}).offsetOfLastCodepointIn());

    // truncated well-formed

    ASSERT_EQUAL(1u, (Builder{0x7F, 0xF4, 0x8F, 0xBF}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(1u, (Builder{0x7F, 0xF4, 0x8F}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(1u, (Builder{0x7F, 0xF4}).offsetOfLastCodepointIn());

    // ill-formed

    ASSERT_EQUAL(2u, (Builder{0x7F, 0xF4, 0x8F, 0xBF, 0xBF, 0xBF}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(2u, (Builder{0x7F, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(1u, (Builder{0x7F, 0xBF, 0xBF, 0xBF, 0xBF}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(0u, (Builder{0xBF, 0xBF, 0xBF, 0xBF}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(0u, (Builder{0xBF, 0xBF, 0xBF}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(0u, Builder{0x80}.offsetOfLastCodepointIn());

    ASSERT_EQUAL(1u, (Builder{0xBF, 0xC0, 0xBF, 0xBF}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(1u, (Builder{0xBF, 0xFF, 0xBF, 0xBF}).offsetOfLastCodepointIn());

    ASSERT_EQUAL(1u, (Builder{0x01, 0b11000000}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(1u, (Builder{0x01, 0b11100000}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(1u, (Builder{0x01, 0b11110000}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(1u, (Builder{0x01, 0b11111000}).offsetOfLastCodepointIn());
    ASSERT_EQUAL(1u, (Builder{0x01, 0b11111100}).offsetOfLastCodepointIn())
}


static void testDefaultConstructedIsEmpty() {
    dbor::String s;
    ASSERT_EQUAL(nullptr, s.buffer());
    ASSERT_EQUAL(0u, s.size());
}


static void testIsEmptyWithoutBuffer() {
    {
        dbor::String s(nullptr, 27);
        ASSERT_EQUAL(nullptr, s.buffer());
        ASSERT_EQUAL(0u, s.size());
    }
    {
        const std::uint8_t buffer[1] = {};
        dbor::String s(buffer, 0);
        ASSERT_EQUAL(nullptr, s.buffer());
        ASSERT_EQUAL(0u, s.size());
    }
}


static void testCheck() {

    dbor::String::CodePoint mi, ma;
    std::size_t n;

    // empty

    n = 7;
    mi = 7;
    ma = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, StringBuilder{}.string.check());
    ASSERT_EQUAL(dbor::ResultCodes::OK, StringBuilder{}.string.check(n, mi, ma));
    ASSERT_EQUAL(0, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT, mi);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT, ma);

    // well-formed non-empty

    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (StringBuilder{'a', 0x01, 'Z', 0x7F}).string.check());
    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (StringBuilder{'a', 0x01, 'Z', 0x7F}).string.check(n, mi, ma));
    ASSERT_EQUAL(4, n);
    ASSERT_EQUAL(0x01, mi);
    ASSERT_EQUAL(0x7F, ma);

    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (StringBuilder{
                    0xED, 0x9F, 0xBF,
                    0x00,
                    0xF4, 0x8F, 0xBF, 0xBF,
                    0xDF, 0xBF
                 }).string.check(n, mi, ma));
    ASSERT_EQUAL(4, n);
    ASSERT_EQUAL(0x0000, mi);
    ASSERT_EQUAL(0x10FFFF, ma);

    // ill-formed

    ASSERT_EQUAL(dbor::ResultCodes::ILLFORMED,
                 (StringBuilder{
                    0x30,
                    0xF4, 0xFF, 0xBF, 0xBF
                 }).string.check(n, mi, ma));
    ASSERT_EQUAL(0, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT, mi);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT, ma);

    n = 7;
    mi = 7;
    ma = 7;
    ASSERT_EQUAL(dbor::ResultCodes::ILLFORMED,
                 (StringBuilder{
                    0xF0, 0x90, 0x80,
                    0x30
                 }).string.check());
    ASSERT_EQUAL(dbor::ResultCodes::ILLFORMED,
                 (StringBuilder{
                    0xF0, 0x90, 0x80,
                    0x30
                 }).string.check(n, mi, ma));
    ASSERT_EQUAL(0, n);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT, mi);
    ASSERT_EQUAL(dbor::String::INVALID_CODEPOINT, ma);
}


static void testGetAsAscii() {
    const char *p;
    std::size_t n;

    // empty

    p = "";
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, StringBuilder{}.string.getAsAscii(p, n));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);
    ASSERT_EQUAL(dbor::ResultCodes::OK, StringBuilder{}.string.getAsAscii(p, n, true));

    // (printable) ASCII

    {
        std::uint8_t buffer[] = {'a', 'b', 'c'};
        dbor::String s(buffer, sizeof(buffer));
        ASSERT_EQUAL(dbor::ResultCodes::OK, s.getAsAscii(p, n));
        ASSERT_EQUAL(reinterpret_cast<const char *>(s.buffer()), p);
        ASSERT_EQUAL(3, n);
    }

    ASSERT_EQUAL(dbor::ResultCodes::OK,
                (StringBuilder{0x20, 'a', 0x7F, 'Z', 0x00}).string.getAsAscii(p, n));
    ASSERT_TRUE(p != nullptr);
    ASSERT_EQUAL(5, n);

    p = nullptr;
    ASSERT_EQUAL(dbor::ResultCodes::OK,
                (StringBuilder{0x20, 'a', 0x7E, 'Z'}).string.getAsAscii(p, n, true));
    ASSERT_TRUE(p != nullptr);
    ASSERT_EQUAL(4, n);

    // not (printable) ASCII

    p = "";
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::RANGE,
                 (StringBuilder{0xC2, 0x80}).string.getAsAscii(p, n));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);

    p = "";
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::RANGE,
                 (StringBuilder{0xF4, 0x8F, 0xBF, 0xBF}).string.getAsAscii(p, n));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);

    p = "";
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::RANGE,
                 StringBuilder{0x1F}.string.getAsAscii(p, n, true));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);

    p = "";
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::RANGE,
                 StringBuilder{0x7F}.string.getAsAscii(p, n, true));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);

    // ill-formed

    p = "";
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::ILLFORMED,
                 (StringBuilder{0xF4, 0x8F, 0xBF}).string.getAsAscii(p, n));
    ASSERT_EQUAL(0, n);
    ASSERT_EQUAL(nullptr, p);
}


static void testGetAsUtf8() {
    const std::uint8_t b = 0;
    const std::uint8_t *p;
    std::size_t n;

    // empty

    p = &b;
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, StringBuilder{}.string.getAsUtf8(p, n, 0, 0x10FFFF));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);
    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 StringBuilder{}.string.getAsUtf8(p, n, dbor::String::INVALID_CODEPOINT, 0));

    // well-formed ASCII in specified range

    {
        std::uint8_t buffer[] = {
            0xED, 0x9F, 0xBF,
            0x00,
            0xF4, 0x8F, 0xBF, 0xBF,
            0xDF, 0xBF
        };
        dbor::String s(buffer, sizeof(buffer));
        ASSERT_EQUAL(dbor::ResultCodes::OK, s.getAsUtf8(p, n, 0, 0x10FFFF));
        ASSERT_EQUAL(s.buffer(), p);
        ASSERT_EQUAL(10, n);
    }

    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (StringBuilder{'a', 0xED, 0x9F, 0xBF, 'c'}).string.getAsUtf8(p, n, 'a', 0xD7FF));
    ASSERT_TRUE(p != nullptr);
    ASSERT_EQUAL(5, n);

    // well-formed ASCII outside specified range

    ASSERT_EQUAL(dbor::ResultCodes::RANGE,
                 (StringBuilder{'a', 0xED, 0x9F, 0xBF, 'c'}).string.getAsUtf8(p, n, 'b', 0xD7FF));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);
    ASSERT_EQUAL(dbor::ResultCodes::RANGE,
                 (StringBuilder{'a', 0xED, 0x9F, 0xBF, 'c'}).string.getAsUtf8(p, n, 'a', 0xD7FE));

    // ill-formed

    p = &b;
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::ILLFORMED,
                 (StringBuilder{0xF4, 0x8F, 0xBF}).string.getAsUtf8(p, n, 0, 0x10FFFF));
    ASSERT_EQUAL(0, n);
    ASSERT_EQUAL(nullptr, p);
}


void testString() {
    testSizeOfUtf8ForCodepoint();
    testFirstCodepointIn();
    testOffsetOfLastCodepointIn();

    testDefaultConstructedIsEmpty();
    testIsEmptyWithoutBuffer();

    testCheck();
    testGetAsAscii();
    testGetAsUtf8();
}
