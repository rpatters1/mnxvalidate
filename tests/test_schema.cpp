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

TEST(Schema, InputSchemaValid)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / utils::utf8ToPath("generic_nonascii_其れ.json");
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--schema", (getInputPath() / "generic_schema.json").u8string() };
    checkStderr({ "Processing", inputPath.filename().u8string(), "is valid" }, [&]() {
        EXPECT_EQ(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}

TEST(Schema, InputSchemaNotValid)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "valid.mnx";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--schema", (getInputPath() / "generic_schema.json").u8string() };
    checkStderr({ "Processing", inputPath.filename().u8string(), "is not valid" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}

TEST(Schema, EmbeddedSchemaValid)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "valid.mnx";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string() };
    checkStderr({ "Processing", inputPath.filename().u8string(), "is valid" }, [&]() {
        EXPECT_EQ(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}

TEST(Schema, EmbeddedSchemaNotValid)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / utils::utf8ToPath("generic_nonascii_其れ.json");
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string() };
    checkStderr({ "Processing", inputPath.filename().u8string(), "is not valid" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}
