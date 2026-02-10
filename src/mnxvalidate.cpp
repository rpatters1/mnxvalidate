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
#include <iostream>
#include <memory>

#include "mnxvalidate.h"
#include "mnxdom.h"

namespace mnxvalidate {

std::vector<const arg_char*> MnxValidateContext::parseOptions(int argc, arg_char* argv[])
{
    std::vector<const arg_char*> args;
    for (int x = 1; x < argc; x++) {
        auto getNextArg = [&]() -> arg_view {
                if (x + 1 < argc) {
                    arg_view arg(argv[x + 1]);
                    if (x < (argc - 1) && arg.rfind(_ARG("--"), 0) != 0) {
                        x++;
                        return arg;
                    }
                }
                return {};
            };
        const arg_view next(argv[x]);
        if (next == _ARG("--version")) {
            showVersion = true;
        } else if (next == _ARG("--about")) {
            showAbout = true;
        } else if (next == _ARG("--help")) {
            showHelp = true;
        } else if (next == _ARG("--log")) {
            logFilePath = getNextArg();
        } else if (next == _ARG("--no-log")) {
            noLog = true;
        } else if (next == _ARG("--recursive")) {
            recursiveSearch = true;
        } else if (next == _ARG("--quiet")) {
            quiet = true;
        } else if (next == _ARG("--verbose")) {
            verbose = true;
        } else if (next == _ARG("--schema")) {
            std::filesystem::path schemaPath = getNextArg();
            if (!schemaPath.empty()) {
                mnxSchemaPath = schemaPath;
            }
        } else if (next == _ARG("--schema-only")) {
            schemaOnly = true;
#ifdef MNXVALIDATE_TEST // this is defined on the command line by the test program
        } else if (next == _ARG("--testing")) {
            testOutput = true;
#endif
        } else {
            args.push_back(argv[x]);
        }
    }
    return args;
}

std::string getTimeStamp(const std::string& fmt)
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &time_t_now); // Windows
#else
    std::tm* tmPtr = localtime(&time_t_now); // POSIX: localtime is thread-safe per thread
    if (tmPtr) {
        localTime = *tmPtr;
    } else {
        return {}; // Handle failure gracefully
    }
#endif
    std::ostringstream timestamp;
    timestamp << std::put_time(&localTime, fmt.c_str());
    return timestamp.str();
}

void MnxValidateContext::logMessage(LogMsg&& msg, bool alwaysShow, LogSeverity severity) const
{
    auto getSeverityStr = [severity]() -> std::string {
            switch (severity) {
            default:
            case LogSeverity::Info: return "";
            case LogSeverity::Warning: return "[WARNING] ";
            case LogSeverity::Error: return "[***ERROR***] ";
            }
        };
    if (!alwaysShow) {
        if (severity == LogSeverity::Verbose && (!verbose || quiet)) {
            return;
        }
        if (severity == LogSeverity::Info && quiet) {
            return;
        }
    }
    if (severity == LogSeverity::Error) {
        errorOccurred = true;
    }
    msg.flush();
    std::string inputFile = utils::pathToString(inputFilePath.filename());
    if (!inputFile.empty()) {
        inputFile += ' ';
    }
    if (logFile && logFile->is_open()) {
        LogMsg prefix = LogMsg() << "[" << getTimeStamp("%Y-%m-%d %H:%M:%S") << "] " << inputFile;
        prefix.flush();
        *logFile << prefix.str() << getSeverityStr() << msg.str() << std::endl;
        if (severity != LogSeverity::Error) {
            return;
        }
    }

#if defined(_WIN32) && !defined(MNXVALIDATE_TEST)
    HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
    if (hConsole && hConsole != INVALID_HANDLE_VALUE) {
        DWORD consoleMode{};
        if (::GetConsoleMode(hConsole, &consoleMode)) {
            std::wstringstream wMsg;
            wMsg << utils::stringToWstring(inputFile + getSeverityStr() + msg.str()) << std::endl;
            DWORD written{};
            if (::WriteConsoleW(hConsole, wMsg.str().data(), static_cast<DWORD>(wMsg.str().size()), &written, nullptr)) {
                return;
            }
            std::wcerr << L"Failed to write message to console: " << ::GetLastError() << std::endl;
        }
    }
    std::wcerr << utils::stringToWstring(msg.str()) << std::endl;
#else
    std::cerr << inputFile << getSeverityStr() << msg.str() << std::endl;
#endif
}

/** returns true if input path is a directory */
bool createDirectoryIfNeeded(const std::filesystem::path& path)
{
    // std::filesystem::path::exists() somethimes erroneously throws on Windows network drives.
    // this works around that issue.
    bool exists = false;
    try {
        exists = std::filesystem::exists(path);
    } catch(...) {}
    if (!exists && !path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
    }
    if (std::filesystem::is_directory(path) || (!exists && !path.has_extension())) {
        std::filesystem::create_directories(path);
        return true;
    }
    return false;
}

void MnxValidateContext::startLogging(const std::filesystem::path& defaultLogPath, int argc, arg_char* argv[])
{
    errorOccurred = false;
    if (!noLog && logFilePath.has_value() && !logFile) {
        if (forTestOutput()) {
            std::cout << "Logging to " << utils::pathToString(logFilePath.value()) << std::endl;
            return;
        }
        auto& path = logFilePath.value();
        if (path.empty()) {
            path = programName + "-logs";
        }
        if (path.is_relative()) {
            path = defaultLogPath / path;
        }
        if (createDirectoryIfNeeded(path)) {
            std::string logFileName = programName + "-" + getTimeStamp("%Y%m%d-%H%M%S") + ".log";
            path /= logFileName;
        }
        bool appending = std::filesystem::is_regular_file(path);
        logFile = std::make_shared<std::ofstream>();
        logFile->exceptions(std::ios::failbit | std::ios::badbit);
        logFile->open(path, std::ios::app);
        if (appending) {
            *logFile << std::endl;
        }
        logMessage(LogMsg() << "======= START =======", true);
        logMessage(LogMsg() << programName << " executed with the following arguments:", true);
        LogMsg args;
        args << programName << " ";
        for (int i = 1; i < argc; i++) {
            args << std::string(arg_string(argv[i])) << " ";
        }
        logMessage(std::move(args), true);
    }   
}

void MnxValidateContext::endLogging()
{
    if (!noLog && logFilePath.has_value() && !forTestOutput()) {
        inputFilePath = "";
        logMessage(LogMsg(), true);
        logMessage(LogMsg() << programName << " processing complete", true);
        logMessage(LogMsg() << "======== END ========", true);
        logFile.reset();
    }
}

static bool validateJsonAgainstSchema(const std::filesystem::path& jsonFilePath, const MnxValidateContext& context)
{
    try {
        auto doc = std::make_unique<mnx::Document>(mnx::Document::create(jsonFilePath));
        auto validateResult = mnx::validation::schemaValidate(*doc, context.mnxSchema);
        if (validateResult) {
            context.logMessage(LogMsg() << "Schema validation succeeded.");
            context.mnxDoc = std::move(doc);
            return true;
        }
        context.logMessage(LogMsg() << "Validation errors:", LogSeverity::Error);
        for (const auto& error : validateResult.errors) {
            context.logMessage(LogMsg() << "    "  << error.to_string(), LogSeverity::Error);
        }
    } catch (const json::exception& e) {
        context.logMessage(LogMsg() << "Parsing error: " << e.what(), LogSeverity::Error);
    }
    context.logMessage(LogMsg() << "Schema validation failed.", LogSeverity::Error);
    return false;
}

void MnxValidateContext::processFile(const std::filesystem::path inpFilePath) const
{
    try {
        if (!std::filesystem::is_regular_file(inpFilePath) && !forTestOutput()) {
            throw std::runtime_error("Input file " + utils::pathToString(inpFilePath) + " does not exist or is not a file.");
        }
        constexpr char kProcessingMessage[] = "Processing File: ";
        constexpr size_t kProcessingMessageSize = sizeof(kProcessingMessage) - 1; // account for null terminator.
        std::string delimiter(kProcessingMessageSize + inpFilePath.u32string().size(), '='); // use u32string().size to get actual number of characters displayed
        // log header for each file
        logMessage(LogMsg(), true);
        logMessage(LogMsg() << delimiter, true);
        logMessage(LogMsg() << kProcessingMessage << utils::pathToString(inpFilePath), true);
        logMessage(LogMsg() << delimiter, true);
        resetForFile(inpFilePath); // reset after logging the header

        bool success = validateJsonAgainstSchema(inputFilePath, *this); // side-effect: validateJsonAgainstSchema creates the mnxDocument
        if (success && !schemaOnly) {
            auto result = mnx::validation::semanticValidate(*mnxDoc);
            if (result) {
                size_t layoutSize = mnxDoc->layouts() ? mnxDoc->layouts().value().size() : 0;
                logMessage(LogMsg() << "Semantic validation complete (" << mnxDoc->global().measures().size() << " measures, "
                    << mnxDoc->parts().size() << " parts, " << layoutSize << " layouts).");
            } else {
                logMessage(LogMsg() << "Semantic validation errors:", LogSeverity::Error);
                for (const auto& error : result.errors) {
                    logMessage(LogMsg() << "    "  << error.to_string(), LogSeverity::Error);
                }
            }
        }
    } catch (const std::exception& e) {
        logMessage(LogMsg() << e.what(), true, LogSeverity::Error);
    }
}

} // namespace mnxvalidate
