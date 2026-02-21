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
    ArgList args = { MNXVALIDATE_NAME, utils::pathToString(inputPath), "--schema", utils::pathToString(getInputPath() / "generic_schema.json"), "--schema-only" };
    checkStderr({ "Processing", utils::pathToString(inputPath.filename()), "Schema validation succeeded" }, [&]() {
        EXPECT_EQ(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << utils::pathToString(inputPath);
    });
}

TEST(Schema, InputSchemaNotValid)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "valid.mnx";
    ArgList args = { MNXVALIDATE_NAME, utils::pathToString(inputPath), "--schema", utils::pathToString(getInputPath() / "generic_schema.json") };
    checkStderr({ "Processing", utils::pathToString(inputPath.filename()), "Schema validation failed" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << utils::pathToString(inputPath);
    });
}

TEST(Schema, EmbeddedSchemaValid)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "valid.mnx";
    ArgList args = { MNXVALIDATE_NAME, utils::pathToString(inputPath) };
    checkStderr({ "Processing", utils::pathToString(inputPath.filename()), "Schema validation succeeded" }, [&]() {
        EXPECT_EQ(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << utils::pathToString(inputPath);
    });
}

TEST(Schema, EmbeddedSchemaNotValid)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / utils::utf8ToPath("generic_nonascii_其れ.json");
    ArgList args = { MNXVALIDATE_NAME, utils::pathToString(inputPath) };
    checkStderr({ "Processing", utils::pathToString(inputPath.filename()), "Schema validation failed" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << utils::pathToString(inputPath);
    });
}

TEST(Schema, UnknownOption)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "valid.mnx";
    ArgList args = { MNXVALIDATE_NAME, utils::pathToString(inputPath), "--bogus" };
    checkStderr("Unknown option: --bogus", [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "unknown option should fail";
    });
}
