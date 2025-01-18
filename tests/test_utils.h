/*merged/
 * Copyright (C) 2024, Robert Patterson
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
#pragma once

#include <vector>
#include <string>
#include <functional>

#include "mnxvalidate.h"

class ArgList {
public:
    // Constructor to allow initialization with { "arg1", "arg2", ... }
    ArgList(std::initializer_list<std::string> init)
    {
        args_.reserve(init.size()); // Reserve space for performance.
        for (const auto& str : init) {
            args_.emplace_back(str); // Convert std::string to mnxvalidate::arg_string.
        }
    }

    // Default constructor for manual addition
    ArgList() = default;

    // Add a single argument
    void add(const mnxvalidate::arg_string& arg) {
        args_.emplace_back(arg);
    }

    // Add multiple arguments
    void add(const std::vector<mnxvalidate::arg_string>& args) {
        args_.insert(args_.end(), args.begin(), args.end());
    }

    // Get argc (number of arguments)
    int argc() const {
        return static_cast<int>(args_.size());
    }

    // Get argv (C-style mnxvalidate::arg_char** array)
    mnxvalidate::arg_char** argv() {
        argv_.clear();
        for (const auto& arg : args_) {
            argv_.push_back(const_cast<mnxvalidate::arg_char*>(arg.c_str()));
        }
        argv_.push_back(nullptr); // Null-terminate
        return argv_.data();
    }

private:
    std::vector<mnxvalidate::arg_string> args_; // Store arguments as strings
    std::vector<mnxvalidate::arg_char*> argv_;       // Cache for mnxvalidate::arg_char** representation
};

void checkStderr(const std::vector<std::string>& expectedMessages, std::function<void()> callback);
inline void checkStderr(const std::string& expectedMessage, std::function<void()> callback)
{ checkStderr(std::vector<std::string>({ expectedMessage }), callback); }

void checkStdout(const std::vector<std::string>& expectedMessages, std::function<void()> callback);
inline void checkStdout(const std::string& expectedMessage, std::function<void()> callback)
{ checkStdout(std::vector<std::string>({ expectedMessage }), callback); }

constexpr const char MNXVALIDATE_NAME[] = "mnxvalidate";

#ifdef _WIN32
constexpr char DIRECTORY_SEP = '\\';
#else
constexpr char DIRECTORY_SEP = '/';
#endif

inline std::filesystem::path getInputPath()
{ return std::filesystem::current_path() / "inputs"; }

inline std::filesystem::path getOutputPath()
{ return std::filesystem::current_path() / "outputs"; }

void setupTestDataPaths();
void copyInputToOutput(const std::string& fileName, std::filesystem::path& outputPath);
void compareFiles(const std::filesystem::path& path1, const std::filesystem::path& path2);

void assertStringsInFile(const std::vector<std::string>& targets, const std::filesystem::path& filePath, const std::filesystem::path& extension = {});
inline void assertStringInFile(const std::string& target, const std::filesystem::path& filePath, const std::filesystem::path& extension = {})
{ assertStringsInFile(std::vector<std::string>({ target }), filePath, extension); }
