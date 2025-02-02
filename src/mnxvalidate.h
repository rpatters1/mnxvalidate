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
#pragma once

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <optional>
#include <fstream>
#include <functional>
#include <unordered_set>

#include "utils/stringutils.h"
#include "nlohmann/json.hpp"

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
using json = nlohmann::json;

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

    std::optional<std::filesystem::path> mnxSchemaPath;
    std::optional<std::string> mnxSchema;
    bool schemaOnly{};

    mutable std::filesystem::path inputFilePath;
    mutable std::map<int, size_t> mnxMeasureList; // key: measId; value: index of measure in global measure array
    mutable size_t measCount{}; // can be different than mnxMeasureList.size() if there is a duplicate key error
    mutable std::unordered_set<std::string> mnxPartList;
    mutable std::unordered_set<std::string> mnxLayoutList;

#ifdef MNXVALIDATE_TEST // this is defined on the command line by the test program
    bool testOutput{};
#endif

    // Parse general options and return remaining options
    std::vector<const arg_char*> parseOptions(int argc, arg_char* argv[]);

    void processFile(const std::filesystem::path inpFilePath) const;

    // Logging methods
    void startLogging(const std::filesystem::path& defaultLogPath, int argc, arg_char* argv[]); ///< Starts logging if logging was requested

    /**
     * @brief logs a message using the mnxValidateContext or outputs to std::cerr
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

    bool addKey(const std::string& key, std::unordered_set<std::string>& keySet, const std::string& setDescription) const
    {
        auto it = keySet.find(key);
        if (it == keySet.end()) {
            keySet.insert(key);
            return true;
        } else {
            logMessage(LogMsg() << "more than one " << setDescription << " with id \"" << key << "\" exists.", LogSeverity::Error);
        }
        return false;
    }

    bool checkPart(const std::string& partId, const std::string& parentDesc) const
    {
        auto it = mnxPartList.find(partId);
        if (it == mnxPartList.end()) {
            logMessage(LogMsg() << parentDesc << " references non-existent part \"" << partId << "\".", LogSeverity::Error);
            return false;
        }
        return true;
    }

    bool checkLayout(const std::string& layoutId, const std::string& parentDesc) const
    {
        auto it = mnxLayoutList.find(layoutId);
        if (it == mnxLayoutList.end()) {
            logMessage(LogMsg() << parentDesc << " references non-existent layout \"" << layoutId << "\".", LogSeverity::Error);
            return false;
        }
        return true;
    }

    std::optional<size_t> getMeasureIndex(int measureId, const std::string& parentDesc) const
    {
        auto it = mnxMeasureList.find(measureId);
        if (it == mnxMeasureList.end()) {
            logMessage(LogMsg() << parentDesc << " references non-existent measure " << std::to_string(measureId) << ".", LogSeverity::Error);
            return std::nullopt;
        }
        return it->second;
    }

private:
    void logMessage(LogMsg&& msg, bool alwaysShow, LogSeverity severity = LogSeverity::Info) const;

    void resetForFile(const std::filesystem::path& inpFile) const
    {
        inputFilePath = inpFile;
        mnxMeasureList.clear();
        measCount = 0;
        mnxPartList.clear();
        mnxLayoutList.clear();
    }
};

std::string getTimeStamp(const std::string& fmt);

bool createDirectoryIfNeeded(const std::filesystem::path& path);
void showAboutPage();

inline bool nodeExists(json jsonData, const std::string_view& nodeName, bool required = true)
{
    bool retval = jsonData.contains(nodeName);
    if (required && !retval) {
        throw std::invalid_argument("Validated JSON node does not contain required value \"" + std::string(nodeName) + "\"!");
    }
    return retval;
}

} // namespace mnxvalidate

#ifdef MNXVALIDATE_TEST // this is defined on the command line by the test program
#undef _MAIN
#define _MAIN mnxValidateTestMain
int mnxValidateTestMain(int argc, mnxvalidate::arg_char* argv[]);
#ifdef MNXVALIDATE_VERSION
#undef MNXVALIDATE_VERSION
#define MNXVALIDATE_VERSION "TEST"
#endif
#define MNXVALIDATE_TEST_CODE(C) C
#else
#define MNXVALIDATE_TEST_CODE(C)
#endif
