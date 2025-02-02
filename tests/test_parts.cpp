/*
 * Copyright (C) 2025, Robert Patterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <string>
#include <filesystem>
#include <iterator>

#include "gtest/gtest.h"
#include "mnxvalidate.h"
#include "test_utils.h"

using namespace mnxvalidate;

TEST(Parts, DuplicateId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_parts.json";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--no-log" };
    checkStderr({ std::string("duplicate_parts.json"), "more than one part with id \"P1\"" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}

TEST(Parts, MeasuresMismatch)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "measures_mismatch.json";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--no-log" };
    checkStderr({ std::string("measures_mismatch.json"), "contains a different number of measures (4) than are defined globally (3)" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}