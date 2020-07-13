// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestResultCodes.hpp"
#include "Test.hpp"
#include "Dbor/ResultCodes.hpp"


static void testTestOperations() {
    ASSERT_TRUE(isOk(dbor::ResultCodes::OK));
    ASSERT_TRUE(!isOk(dbor::ResultCodes::APPROX_IMPRECISE));

    ASSERT_TRUE(isOkExcept(dbor::ResultCodes::APPROX_EXTREME << dbor::ResultCodes::ILLFORMED,
                           dbor::ResultCodes::APPROX_EXTREME << dbor::ResultCodes::ILLFORMED));
    ASSERT_TRUE(!isOkExcept(dbor::ResultCodes::APPROX_EXTREME << dbor::ResultCodes::ILLFORMED,
                            dbor::ResultCodes::ILLFORMED));
    ASSERT_TRUE(!isOkExcept(dbor::ResultCodes::APPROX_EXTREME << dbor::ResultCodes::ILLFORMED,
                            dbor::ResultCodes::APPROX_EXTREME));

    ASSERT_TRUE(isApprox(dbor::ResultCodes::APPROX_IMPRECISE));
    ASSERT_TRUE(isApprox(dbor::ResultCodes::APPROX_EXTREME));
    ASSERT_TRUE(!isApprox(dbor::ResultCodes::OK));
}


static void testManipulations() {
    dbor::ResultCodes e =
        dbor::ResultCodes::NO_OBJECT
        << dbor::ResultCodes::APPROX_EXTREME
        << dbor::ResultCodes::INCOMPATIBLE;

    ASSERT_TRUE(!isOk(e));

    dbor::ResultCodes e2 = e;
    e2 -= dbor::ResultCodes::APPROX_EXTREME << dbor::ResultCodes::ILLFORMED;
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT << dbor::ResultCodes::INCOMPATIBLE, e2);
    e2 -= dbor::ResultCodes::APPROX_EXTREME;
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT << dbor::ResultCodes::INCOMPATIBLE, e2);

    e2 = e;
    e2 &= dbor::ResultCodes::APPROX_EXTREME << dbor::ResultCodes::ILLFORMED;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, e2);

    e2 <<= dbor::ResultCodes::ILLFORMED;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME << dbor::ResultCodes::ILLFORMED, e2);
}


static void testIteration() {
    typedef std::underlying_type<dbor::ResultCodes>::type U;

    ASSERT_EQUAL(dbor::ResultCodes::OK, leastSevereIn(dbor::ResultCodes::OK));
    ASSERT_EQUAL(dbor::ResultCodes::RANGE, leastSevereIn(dbor::ResultCodes::RANGE));

    static constexpr std::size_t NUMBER_OF_NONOK_RESULTCODES = 7;
    static_assert(static_cast<U>(dbor::ResultCodes::ALL) > 0, "");
    static_assert(static_cast<U>(dbor::ResultCodes::ALL)
                  == (1U << NUMBER_OF_NONOK_RESULTCODES) - 1, "");

    // over dbor::ResultCodes::ALL

    dbor::ResultCodes results = dbor::ResultCodes::ALL;
    std::uint_least8_t n = 0;
    while (results != dbor::ResultCodes::OK) {
        dbor::ResultCodes r = leastSevereIn(results);
        ASSERT_TRUE(r != dbor::ResultCodes::OK);
        ASSERT_EQUAL(r, leastSevereIn(r));
        results -= r;
        n++;
    }

    ASSERT_EQUAL(NUMBER_OF_NONOK_RESULTCODES, n);

    // over all possible bits

    results = static_cast<dbor::ResultCodes>(~static_cast<U>(0));
    n = 0;
    while (results != dbor::ResultCodes::OK) {
        dbor::ResultCodes r = leastSevereIn(results);
        results -= r;
        n++;
    }

    ASSERT_TRUE(n >= NUMBER_OF_NONOK_RESULTCODES);
}


void testResultCodes() {
    testTestOperations();
    testManipulations();
    testIteration();
}
