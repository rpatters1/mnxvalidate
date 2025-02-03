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

TEST(Scores, InvalidScoreLayoutId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_bad_layout.json";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--no-log" };
    checkStderr({ std::string("score_bad_layout.json"), "references non-existent layout \"does-not-exist\"" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}

TEST(Scores, InvalidScorePageLayoutId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_page_bad_layout.json";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--no-log" };
    checkStderr({ std::string("score_page_bad_layout.json"), "Page[0]", "references non-existent layout \"does-not-exist\"" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}

TEST(Scores, InvalidScoreSystemLayoutId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_system_bad_layout.json";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--no-log" };
    checkStderr({ std::string("score_system_bad_layout.json"), "System[0]", "references non-existent layout \"does-not-exist\"" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}

TEST(Scores, InvalidScoreLayoutChangeLayoutId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_layoutchange_bad_layout.json";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--no-log" };
    checkStderr({ std::string("score_layoutchange_bad_layout.json"), "Layout change[0]", "references non-existent layout \"does-not-exist\"" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}

TEST(Scores, InvalidMMRestStartMeasure)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_mmrest_bad_start.json";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--no-log" };
    checkStderr({ std::string("score_mmrest_bad_start.json"), "Multimeasure rest in score \"Score\" references non-existent measure 3" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}

TEST(Scores, InvalidMMRestSpan)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_mmrest_bad_span.json";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--no-log" };
    checkStderr({ std::string("score_mmrest_bad_span.json"), "Multimeasure rest at measure 1 in score \"Score\" spans non-existent measures" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}

TEST(Scores, InvalidSystemStartMeasure)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_system_bad_measure.json";
    ArgList args = { MNXVALIDATE_NAME, inputPath.u8string(), "--no-log" };
    checkStderr({ std::string("score_system_bad_measure.json"), "System[0] in page[0] in score \"Score\" references non-existent measure 1" }, [&]() {
        EXPECT_NE(mnxValidateTestMain(args.argc(), args.argv()), 0) << "validate " << inputPath.u8string();
    });
}
