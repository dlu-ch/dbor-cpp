// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestType.hpp"
#include "Test.hpp"
#include "Dbor/Type.hpp"


static void testErrorCodeTestOperations() {
    ASSERT_TRUE(isOk(dbor::ResultCodes::OK));
    ASSERT_TRUE(!isOk(dbor::ResultCodes::APPROX_PRECISION));

    ASSERT_TRUE(isOkExcept(dbor::ResultCodes::APPROX_RANGE << dbor::ResultCodes::ILLFORMED,
                           dbor::ResultCodes::APPROX_RANGE << dbor::ResultCodes::ILLFORMED));
    ASSERT_TRUE(!isOkExcept(dbor::ResultCodes::APPROX_RANGE << dbor::ResultCodes::ILLFORMED,
                            dbor::ResultCodes::ILLFORMED));
    ASSERT_TRUE(!isOkExcept(dbor::ResultCodes::APPROX_RANGE << dbor::ResultCodes::ILLFORMED,
                            dbor::ResultCodes::APPROX_RANGE));

    ASSERT_TRUE(isApprox(dbor::ResultCodes::APPROX_PRECISION));
    ASSERT_TRUE(isApprox(dbor::ResultCodes::APPROX_RANGE));
    ASSERT_TRUE(!isApprox(dbor::ResultCodes::OK));
}


static void testErrorCodeManipulations() {
    dbor::ResultCodes e =
        dbor::ResultCodes::NO_OBJECT
        << dbor::ResultCodes::APPROX_RANGE
        << dbor::ResultCodes::INCOMPATIBLE;

    ASSERT_TRUE(!isOk(e));

    dbor::ResultCodes e2 = e;
    e2 -= dbor::ResultCodes::APPROX_RANGE << dbor::ResultCodes::ILLFORMED;
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT << dbor::ResultCodes::INCOMPATIBLE, e2);
    e2 -= dbor::ResultCodes::APPROX_RANGE;
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT << dbor::ResultCodes::INCOMPATIBLE, e2);

    e2 = e;
    e2 &= dbor::ResultCodes::APPROX_RANGE << dbor::ResultCodes::ILLFORMED;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_RANGE, e2);

    e2 <<= dbor::ResultCodes::ILLFORMED;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_RANGE << dbor::ResultCodes::ILLFORMED, e2);
}


void testType() {
    testErrorCodeTestOperations();
    testErrorCodeManipulations();
}
