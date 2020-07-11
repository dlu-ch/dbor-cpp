// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestType.hpp"
#include "Test.hpp"
#include "Dbor/Type.hpp"


static void testErrorCodeTestOperations() {
    ASSERT_TRUE(isOk(dbor::ErrorCodes::OK_PRECISE));
    ASSERT_TRUE(!isOk(dbor::ErrorCodes::OK_IMPRECISE));

    ASSERT_TRUE(isOkExcept(dbor::ErrorCodes::OUT_OF_RANGE << dbor::ErrorCodes::ILLFORMED,
                           dbor::ErrorCodes::OUT_OF_RANGE << dbor::ErrorCodes::ILLFORMED));
    ASSERT_TRUE(!isOkExcept(dbor::ErrorCodes::OUT_OF_RANGE << dbor::ErrorCodes::ILLFORMED,
                            dbor::ErrorCodes::ILLFORMED));
    ASSERT_TRUE(!isOkExcept(dbor::ErrorCodes::OUT_OF_RANGE << dbor::ErrorCodes::ILLFORMED,
                            dbor::ErrorCodes::OUT_OF_RANGE));

    ASSERT_TRUE(isApprox(dbor::ErrorCodes::OK_IMPRECISE));
    ASSERT_TRUE(isApprox(dbor::ErrorCodes::OUT_OF_RANGE));
    ASSERT_TRUE(!isApprox(dbor::ErrorCodes::OK_PRECISE));
}


static void testErrorCodeManipulations() {
    dbor::ErrorCodes e =
        dbor::ErrorCodes::NO_OBJECT
        << dbor::ErrorCodes::OUT_OF_RANGE
        << dbor::ErrorCodes::INCOMPATIBLE;

    ASSERT_TRUE(!isOk(e));

    dbor::ErrorCodes e2 = e;
    e2 -= dbor::ErrorCodes::OUT_OF_RANGE << dbor::ErrorCodes::ILLFORMED;
    ASSERT_EQUAL(dbor::ErrorCodes::NO_OBJECT << dbor::ErrorCodes::INCOMPATIBLE, e2);
    e2 -= dbor::ErrorCodes::OUT_OF_RANGE;
    ASSERT_EQUAL(dbor::ErrorCodes::NO_OBJECT << dbor::ErrorCodes::INCOMPATIBLE, e2);

    e2 = e;
    e2 &= dbor::ErrorCodes::OUT_OF_RANGE << dbor::ErrorCodes::ILLFORMED;
    ASSERT_EQUAL(dbor::ErrorCodes::OUT_OF_RANGE, e2);

    e2 <<= dbor::ErrorCodes::ILLFORMED;
    ASSERT_EQUAL(dbor::ErrorCodes::OUT_OF_RANGE << dbor::ErrorCodes::ILLFORMED, e2);
}


void testType() {
    testErrorCodeTestOperations();
    testErrorCodeManipulations();
}
