// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestResultCode.hpp"
#include <limits>
#include "Test.hpp"
#include "Dbor/ResultCode.hpp"


static void testElementAndSetTypesAreRelated() {
    typedef std::underlying_type<dbor::ResultCode>::type U;

    // same underlying unsigned type, but different
    static_assert(std::is_same<std::underlying_type<dbor::ResultCodeSet>::type, U>::value, "");
    static_assert(!std::numeric_limits<U>::is_signed, "");
    static_assert(!std::is_same<dbor::ResultCode, dbor::ResultCodeSet>::value, "");

    static_assert(static_cast<U>(dbor::ResultCode::OK)
               == static_cast<U>(dbor::ResultCodeSet::NONE), "");
}


static void testIncludingResultConvertsToSet() {
    ASSERT_EQUAL(dbor::ResultCodeSet::NONE, dbor::ResultCode::OK << dbor::ResultCode::OK);
    ASSERT_EQUAL(dbor::ResultCodeSet::NONE, dbor::ResultCodeSet::NONE << dbor::ResultCode::OK);
    ASSERT_EQUAL(dbor::ResultCodeSet::NONE, dbor::ResultCode::OK << dbor::ResultCodeSet::NONE);

    dbor::ResultCodeSet results{};
    ASSERT_EQUAL(dbor::ResultCodeSet::NONE, results);

    results <<= dbor::ResultCode::OK;
    results <<= dbor::ResultCodeSet::NONE;
    ASSERT_EQUAL(dbor::ResultCodeSet::NONE, results);
}


static void testTestOperations() {
    ASSERT_TRUE(isOk(dbor::ResultCode::OK));
    ASSERT_TRUE(isOk(dbor::ResultCodeSet::NONE));

    ASSERT_TRUE(!isOk(dbor::ResultCode::APPROX_IMPRECISE));
    ASSERT_TRUE(!isOk(dbor::ResultCodeSet::NONE << dbor::ResultCode::APPROX_IMPRECISE));

    ASSERT_TRUE(isOkExcept(dbor::ResultCode::OK, dbor::ResultCode::OK));
    ASSERT_TRUE(isOkExcept(dbor::ResultCode::OK, dbor::ResultCode::APPROX_EXTREME));
    ASSERT_TRUE(!isOkExcept(dbor::ResultCode::ILLFORMED, dbor::ResultCode::APPROX_EXTREME));

    ASSERT_TRUE(isOkExcept(dbor::ResultCode::OK, dbor::ResultCodeSet::NONE));
    ASSERT_TRUE(isOkExcept(dbor::ResultCode::OK,
                           dbor::ResultCode::APPROX_EXTREME
                           << dbor::ResultCode::APPROX_IMPRECISE));
    ASSERT_TRUE(isOkExcept(dbor::ResultCode::APPROX_EXTREME,
                           dbor::ResultCode::APPROX_EXTREME
                           << dbor::ResultCode::APPROX_IMPRECISE));
    ASSERT_TRUE(isOkExcept(dbor::ResultCode::APPROX_IMPRECISE,
                           dbor::ResultCode::APPROX_EXTREME
                           << dbor::ResultCode::APPROX_IMPRECISE));
    ASSERT_TRUE(!isOkExcept(dbor::ResultCode::ILLFORMED, dbor::ResultCode::APPROX_EXTREME));

    ASSERT_TRUE(isOkExcept(dbor::ResultCodeSet::NONE, dbor::ResultCode::OK));
    ASSERT_TRUE(isOkExcept(dbor::ResultCodeSet::NONE << dbor::ResultCode::APPROX_EXTREME,
                           dbor::ResultCode::APPROX_EXTREME));
    ASSERT_TRUE(!isOkExcept(dbor::ResultCodeSet::NONE << dbor::ResultCode::ILLFORMED,
                            dbor::ResultCode::APPROX_EXTREME));

    ASSERT_TRUE(isOkExcept(dbor::ResultCodeSet::NONE, dbor::ResultCodeSet::NONE));
    ASSERT_TRUE(isOkExcept(dbor::ResultCode::APPROX_EXTREME
                           << dbor::ResultCode::APPROX_IMPRECISE,
                           dbor::ResultCode::APPROX_EXTREME
                           << dbor::ResultCode::APPROX_IMPRECISE));
    ASSERT_TRUE(!isOkExcept(dbor::ResultCodeSet::NONE << dbor::ResultCode::ILLFORMED,
                            dbor::ResultCode::APPROX_EXTREME
                            << dbor::ResultCode::APPROX_IMPRECISE));

    ASSERT_TRUE(isApprox(dbor::ResultCode::APPROX_IMPRECISE));
    ASSERT_TRUE(isApprox(dbor::ResultCode::APPROX_EXTREME));
    ASSERT_TRUE(!isApprox(dbor::ResultCode::OK));
    ASSERT_TRUE(!isApprox(dbor::ResultCode::RANGE));

    ASSERT_TRUE(isApprox(dbor::ResultCode::APPROX_IMPRECISE << dbor::ResultCode::APPROX_EXTREME));
    ASSERT_TRUE(!isApprox(dbor::ResultCodeSet::NONE));
    ASSERT_TRUE(!isApprox(dbor::ResultCodeSet::NONE << dbor::ResultCode::RANGE));
}


static void testSetOperations() {
    const dbor::ResultCodeSet r =
        dbor::ResultCode::NO_OBJECT
        << dbor::ResultCode::APPROX_EXTREME
        << dbor::ResultCode::INCOMPATIBLE;

    dbor::ResultCodeSet rm;

    rm = r;
    rm -= dbor::ResultCode::APPROX_EXTREME << dbor::ResultCode::ILLFORMED;
    ASSERT_EQUAL(dbor::ResultCode::NO_OBJECT << dbor::ResultCode::INCOMPATIBLE, rm);
    rm -= dbor::ResultCode::APPROX_EXTREME;
    ASSERT_EQUAL(dbor::ResultCode::NO_OBJECT << dbor::ResultCode::INCOMPATIBLE, rm);

    rm = r;
    rm &= dbor::ResultCode::APPROX_EXTREME << dbor::ResultCode::ILLFORMED;
    ASSERT_EQUAL(dbor::ResultCodeSet::NONE << dbor::ResultCode::APPROX_EXTREME, rm);

    rm <<= dbor::ResultCode::ILLFORMED;
    ASSERT_EQUAL(dbor::ResultCode::APPROX_EXTREME << dbor::ResultCode::ILLFORMED, rm);
}


static void testSetIteration() {
    typedef std::underlying_type<dbor::ResultCode>::type U;

    static constexpr std::size_t NUMBER_OF_NONOK_RESULTCODES = 7;
    static_assert(static_cast<U>(dbor::ResultCodeSet::ALL) > 0, "");
    static_assert(static_cast<U>(dbor::ResultCodeSet::ALL)
                  == (1U << NUMBER_OF_NONOK_RESULTCODES) - 1, "");

    ASSERT_EQUAL(dbor::ResultCode::OK, leastSevereIn(dbor::ResultCodeSet::NONE));

    // over dbor::ResultCodeSet::ALL

    dbor::ResultCodeSet results = dbor::ResultCodeSet::ALL;
    std::uint_least8_t n = 0;
    while (results != dbor::ResultCodeSet::NONE) {
        dbor::ResultCode r = leastSevereIn(results);
        ASSERT_TRUE(r != dbor::ResultCode::OK);
        results -= r;
        n++;
    }

    ASSERT_EQUAL(NUMBER_OF_NONOK_RESULTCODES, n);

    // over all possible bits

    results = static_cast<dbor::ResultCodeSet>(~static_cast<U>(0));
    n = 0;
    while (results != dbor::ResultCodeSet::NONE) {
        dbor::ResultCode r = leastSevereIn(results);
        results -= r;
        n++;
    }

    ASSERT_TRUE(n >= NUMBER_OF_NONOK_RESULTCODES);
}


void testResultCode() {
    testElementAndSetTypesAreRelated();
    testIncludingResultConvertsToSet();
    testTestOperations();
    testSetOperations();
    testSetIteration();
}
