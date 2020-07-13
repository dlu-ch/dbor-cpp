// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestResultCode.hpp"
#include "TestString.hpp"
#include "TestEncoding.hpp"
#include "TestValue.hpp"
#include "TestValueBlock.hpp"

int main() {
    // to avoid a dependency on a test library (like Google Test) to tests are very simple
    // (now fancy output on failure) but nonetheless extensive.

    testResultCode();
    testString();
    testEncoding();
    testValue();
    testValueBlock();

    return 0;
}
