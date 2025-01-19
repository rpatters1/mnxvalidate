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

TEST(Logging, SingleFileNoLog)
{
    setupTestDataPaths();
    std::string inputFile = "accidentals_example.mnx";
    std::filesystem::path inputPath;
    copyInputToOutput(inputFile, inputPath);
     ArgList args = { MNXVALIDATE_NAME, inputPath.u8string() };
    checkStderr({ "Processing", inputPath.filename().u8string(), "is valid" }, [&]() {
        EXPECT_EQ(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
    auto logPath = inputPath.parent_path() / (std::string(MNXVALIDATE_NAME) + "-logs");
    EXPECT_FALSE(std::filesystem::exists(logPath)) << "no log file should have been created";
}

TEST(Logging, InPlace)
{
    setupTestDataPaths();
    std::string inputFile = "generic_nonascii_其れ.json";
    std::filesystem::path inputPath;
    copyInputToOutput(inputFile, inputPath);
    // default log
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--log" };
    checkStderr("is not valid", [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
    auto logPath = inputPath.parent_path() / (std::string(MNXVALIDATE_NAME) + "-logs");
    assertStringsInFile({ "Processing", inputPath.filename().u8string(), "is not valid" }, logPath, ".log");
}

TEST(Logging, Subdirectory)
{
    setupTestDataPaths();
    std::string inputFile = "accidentals_example.mnx";
    std::filesystem::path inputPath;
    copyInputToOutput(inputFile, inputPath);
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--log", "logs" };
    EXPECT_EQ(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    auto logPath = inputPath.parent_path() / "logs";
    assertStringsInFile({ "Processing", inputPath.filename().u8string(), "is valid" }, logPath, ".log");
}

TEST(Logging, SpecificFile)
{
    setupTestDataPaths();
    std::string validFile = "accidentals_example.mnx";
    std::filesystem::path validPath;
    copyInputToOutput(validFile, validPath);
    std::string invalidFile = "generic_nonascii_其れ.json";
    std::filesystem::path invalidPath;
    copyInputToOutput(invalidFile, invalidPath);
    // specific file with appending
    ArgList args1 = { MNXVALIDATE_NAME, validPath.u8string(), "--log", "logs/mylog.log" };
    EXPECT_EQ(mnxValidateTestMain(args1.argc(), args1.argv()), 0) << "validate " << validPath.u8string();
    ArgList args2 = { MNXVALIDATE_NAME, invalidPath.u8string(), "--log", "logs/mylog.log" };
    checkStderr("is not valid", [&]() {
        EXPECT_NE(mnxValidateTestMain(args2.argc(), args2.argv()), 0) << "validate " << invalidPath.u8string();
    });
    auto logPath = validPath.parent_path() / "logs";
    assertStringsInFile({ "Processing", validPath.filename().u8string(), invalidPath.filename().u8string(), "is valid", "is not valid" }, logPath, ".log");
}

TEST(Logging, NonExistentFile)
{
    setupTestDataPaths();
    auto inputPath = getOutputPath() / "doesntExist.mnx";
    ArgList args1 = { MNXVALIDATE_NAME, inputPath.u8string() };
    checkStderr({ "does not exist or is not a file or directory", inputPath.filename().u8string() }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args1.argc(), args1.argv()), 0) << "validate " << inputPath.u8string();
    });
    auto logPath = inputPath.parent_path() / (std::string(MNXVALIDATE_NAME) + "-logs");
    EXPECT_FALSE(std::filesystem::exists(logPath)) << "no log file should have been created";
}

TEST(Logging, PatternFile)
{
    setupTestDataPaths();
    std::string inputFile = "accidentals_example.mnx";
    std::filesystem::path inputPath;
    copyInputToOutput(inputFile, inputPath);
    inputPath = getOutputPath() / "accidentals*.?nx";
    ArgList args1 = { MNXVALIDATE_NAME, inputPath.u8string() };
    checkStderr("", [&]() {
        EXPECT_EQ(mnxValidateTestMain(args1.argc(), args1.argv()), 0) << "validate " << inputPath.u8string();
    });
    auto logPath = inputPath.parent_path() / (std::string(MNXVALIDATE_NAME) + "-logs");
    EXPECT_TRUE(std::filesystem::exists(logPath)) << "log file should have been created";
    assertStringsInFile({ "Processing", "accidentals_example.mnx", "is valid", "!generic" }, logPath, ".log");
}

TEST(Logging, Directory)
{
    setupTestDataPaths();
    std::string inputFile = "accidentals_example.mnx";
    std::filesystem::path inputPath;
    copyInputToOutput(inputFile, inputPath);
    inputPath = getOutputPath();
    ArgList args1 = { MNXVALIDATE_NAME, inputPath.u8string() };
    checkStderr("", [&]() {
        EXPECT_EQ(mnxValidateTestMain(args1.argc(), args1.argv()), 0) << "validate " << inputPath.u8string();
    });
    auto logPath = inputPath / (std::string(MNXVALIDATE_NAME) + "-logs");
    EXPECT_TRUE(std::filesystem::exists(logPath)) << "log file should have been created";
    assertStringsInFile({ "Processing", "accidentals_example.mnx", "is valid" }, logPath, ".log");
}

TEST(Logging, MultipleInputs)
{
    setupTestDataPaths();
    std::string validFile = "accidentals_example.mnx";
    std::filesystem::path validPath;
    copyInputToOutput(validFile, validPath);
    std::string invalidFile = "generic_nonascii_其れ.json";
    std::filesystem::path invalidPath;
    copyInputToOutput(invalidFile, invalidPath);
    // specific file with appending
    ArgList args = { MNXVALIDATE_NAME, validPath.u8string(), invalidPath.u8string() };
    checkStderr("is not valid", [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << invalidPath.u8string();
    });
    auto logPath = validPath.parent_path() / (std::string(MNXVALIDATE_NAME) + "-logs");
    assertStringsInFile({ "Processing", validPath.filename().u8string(), invalidPath.filename().u8string(), "is valid", "is not valid" }, logPath, ".log");
}
