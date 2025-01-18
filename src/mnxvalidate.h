/*
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

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <optional>
#include <fstream>
#include <functional>


#include "utils/stringutils.h"

inline constexpr char MNX_EXTENSION[]       = "mnx";
inline constexpr char JSON_EXTENSION[]      = "json";

#ifdef _WIN32
#define _ARG(S) L##S
#define _ARG_CONV(S) (utils::wstringToString(std::wstring(S)))
#define _MAIN wmain
#else
#define _ARG(S) S
#define _ARG_CONV(S) S
#define _MAIN main
#endif

namespace mnxvalidate {

// This function exists as std::to_array in C++20
template <typename T, std::size_t N>
inline constexpr std::array<T, N> to_array(const T(&arr)[N])
{
    std::array<T, N> result{};
    for (std::size_t i = 0; i < N; ++i) {
        result[i] = arr[i];
    }
    return result;
}

#ifdef _WIN32
using arg_view = std::wstring_view;
using arg_char = WCHAR;
struct arg_string : public std::wstring
{
    using std::wstring::wstring;
    arg_string(const std::wstring& wstr) : std::wstring(wstr) {}
    arg_string(const std::string& str) : std::wstring(utils::stringToWstring(str)) {}
    arg_string(const char * str) : std::wstring(utils::stringToWstring(str)) {}

    operator std::string() const
    {
        return utils::wstringToString(*this);
    }
};

inline std::string operator+(const std::string& lhs, const arg_string& rhs) {
    return lhs + static_cast<std::string>(rhs);
}
#else
using arg_view = std::string_view;
using arg_char = char;
using arg_string = std::string;
#endif

using Buffer = std::vector<char>;
using LogMsg = std::stringstream;

// Function to find the appropriate processor
template <typename Processors>
inline decltype(Processors::value_type::processor) findProcessor(const Processors& processors, const std::string& extension)
{
    std::string key = utils::toLowerCase(extension);
    if (key.rfind(".", 0) == 0) {
        key = extension.substr(1);
    }
    for (const auto& p : processors) {
        if (key == p.extension) {
            return p.processor;
        }
    }
    throw std::invalid_argument("Unsupported format: " + key);
}

/// @brief defines log message severity
enum class LogSeverity
{
    Info,       ///< No error. The message is for information.
    Warning,    ///< An event has occurred that may affect the result, but processing of output continues.
    Error,      ///< Processing of the current file has aborted. This level usually occurs in catch blocks.
    Verbose     ///< Only emit if --verbose option specified. The message is for information.
};

class ICommand;
struct MnxValidateContext
{
public:
    MnxValidateContext(const arg_string& progName)
        : programName(std::string(progName)) {}

    mutable bool errorOccurred{};
    bool outputIsFilename;

    std::string programName;
    bool showVersion{};
    bool showHelp{};
    bool showAbout{};
    bool recursiveSearch{};
    bool noLog{};
    bool verbose{};
    bool quiet{};
    std::optional<std::filesystem::path> logFilePath;
    std::shared_ptr<std::ofstream> logFile;
    std::filesystem::path inputFilePath;

 #ifdef MNXVALIDATE_TEST // this is defined on the command line by the test program
    bool testOutput{};
#endif

    // Parse general options and return remaining options
    std::vector<const arg_char*> parseOptions(int argc, arg_char* argv[]);
    
    // validate paths
    bool validatePathsAndOptions(const std::filesystem::path& outputFilePath) const;

    void processFile(const std::shared_ptr<ICommand>& currentCommand, const std::filesystem::path inpFilePath, const std::vector<const arg_char*>& args);

    // Logging methods
    void startLogging(const std::filesystem::path& defaultLogPath, int argc, arg_char* argv[]); ///< Starts logging if logging was requested

    /**
     * @brief logs a message using the mnxvalidateContext or outputs to std::cerr
     * @param msg a utf-8 encoded message.
     * @param severity the message severity
    */
    void logMessage(LogMsg&& msg, LogSeverity severity = LogSeverity::Info) const
    { logMessage(std::move(msg), false, severity); }

    void endLogging(); ///< Ends logging if logging was requested

    bool forTestOutput() const
    {
#ifdef MNXVALIDATE_TEST
        return testOutput;
#else
        return false;
#endif
    }

private:
    void logMessage(LogMsg&& msg, bool alwaysShow, LogSeverity severity = LogSeverity::Info) const;
};

class ICommand
{
public:
    ICommand() = default;
    virtual ~ICommand() = default;

    virtual int showHelpPage(const std::string_view& programName, const std::string& indentSpaces = {}) const = 0;

    virtual bool canProcess(const std::filesystem::path& inputPath) const = 0;
    virtual Buffer processInput(const std::filesystem::path& inputPath, const MnxValidateContext& mnxvalidateContext) const = 0;
    virtual void processOutput(const Buffer& enigmaXml, const std::filesystem::path& outputPath, const std::filesystem::path& inputPath, const MnxValidateContext& mnxvalidateContext) const = 0;
    virtual std::optional<std::string_view> defaultInputFormat() const { return std::nullopt; }
    virtual std::optional<std::string> defaultOutputFormat(const std::filesystem::path&) const { return std::nullopt; }
    
    virtual const std::string_view commandName() const = 0;
};

std::string getTimeStamp(const std::string& fmt);

bool createDirectoryIfNeeded(const std::filesystem::path& path);
void showAboutPage();

} // namespace mnxvalidate

#ifdef MNXVALIDATE_TEST // this is defined on the command line by the test program
#undef _MAIN
#define _MAIN mnxvalidateTestMain
int mnxvalidateTestMain(int argc, mnxvalidate::arg_char* argv[]);
#ifdef MNXVALIDATE_VERSION
#undef MNXVALIDATE_VERSION
#define MNXVALIDATE_VERSION "TEST"
#endif
#define MNXVALIDATE_TEST_CODE(C) C
#else
#define MNXVALIDATE_TEST_CODE(C)
#endif
