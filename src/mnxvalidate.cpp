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
    std::string inputFile = inputFilePath.filename().u8string();
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
            std::cout << "Logging to " << logFilePath.value().u8string() << std::endl;
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
        auto validateMessage = doc->validate(context.mnxSchema);
        if (!validateMessage.has_value()) {
            context.logMessage(LogMsg() << "is valid against the MNX schema.");
            context.mnxDoc = std::move(doc);
            return true;
        }
        context.logMessage(LogMsg() << "Validation error: " << validateMessage.value(), LogSeverity::Error);        
    } catch (const json::exception& e) {
        context.logMessage(LogMsg() << "Parsing error: " << e.what(), LogSeverity::Error);
    }
    context.logMessage(LogMsg() << " is not valid against the MNX schema.", LogSeverity::Error);
    return false;
}

static void validateGlobal(const MnxValidateContext& context)
{
    bool valid = true;
    int measureId = 0;
    context.measCount = 0;
    auto x = context.mnxDoc->global().measures()[0];
    for (const auto meas : context.mnxDoc->global().measures()) {
        context.measCount++;
        measureId = meas.index_or(measureId + 1);
        auto it = context.mnxMeasureList.find(measureId);
        if (it == context.mnxMeasureList.end()) {
            context.mnxMeasureList.emplace(measureId, x);
        } else {
            context.logMessage(LogMsg() << "measure index " + std::to_string(measureId) + " is duplicated at location "
                + std::to_string(it->second) + " and " + std::to_string(meas.calcArrayIndex()) + ".", LogSeverity::Error);
            valid = false;
        }
    }
    if (valid) {
        context.logMessage(LogMsg() << "validated global data.");
    }
}

static void validateParts(const MnxValidateContext& context)
{
    bool valid = true;
    for (const auto part : context.mnxDoc->parts()) {
        size_t x = part.calcArrayIndex();
        std::string partName = "[" + std::to_string(x) + "]";
        if (auto partName = part.id()) {
            if (!context.addKey(partName.value(), context.mnxPartList, x, "part")) {
                valid = false;
            }
            partName = " \"" + partName.value() + "\"";
        }
        size_t numMeasures = part.measures() ? part.measures().value().size() : 0;
        if (numMeasures != context.measCount) {
            context.logMessage(LogMsg() << "Part" << partName << " contains a different number of measures (" + std::to_string(numMeasures)
                    + ") than are defined globally (" + std::to_string(context.measCount) + ").", LogSeverity::Error);
            valid = false;
        }
    }
    if (valid) {
        context.logMessage(LogMsg() << "validated all parts.");
    }
}

static void validateLayouts(const MnxValidateContext& context)
{
    bool valid = true;
    if (auto layouts = context.mnxDoc->layouts()) {  // layouts are *not* required in MNX
        for (const auto layout : layouts.value()) {
            if (!context.addKey(layout.id(), context.mnxLayoutList, layout.calcArrayIndex(), "layout")) {
                valid = false;
            }
            auto validateContent = [&](auto&& self, const mnx::ContentArray& content) -> void {
                for (const mnx::ContentObject element : content) {
                    if (element.type() == mnx::LayoutGroup::ContentTypeValue) {
                        auto group = element.get<mnx::LayoutGroup>();
                        self(self, group.content());
                    } else if (element.type() == mnx::LayoutStaff::ContentTypeValue) {
                        auto staff = element.get<mnx::LayoutStaff>();
                        /// @todo validate "labelref"?
                        if (auto sources = staff.sources()) {
                            for (const auto& source : element["sources"]) {
                                if (nodeExists(source, "part")) {
                                    if (auto index = context.getPartIndex(source["part"], "Layout " + layout["id"].dump())) {
                                        int staffNum = source.contains("staff") ? source["staff"].get<int>() : 1;
                                        const auto& part = jsonData["parts"][*index];
                                        int numStaves = part.contains("staves") ? part["staves"].get<int>() : 1;
                                        if (staffNum > numStaves || staffNum < 1) {
                                            context.logMessage(LogMsg() << "Layout " << layout["id"].dump() << "has invalid staff number ("
                                                    << std::to_string(staffNum) << ") for part " << source["part"] << ".", LogSeverity::Error);
                                            valid = false;
                                        }
                                    } else {
                                        valid = false;
                                    }
                                }                                    
                                /// @todo validate "labelref"?
                                /// @todo validate "voice"?
                            }
                        }
                    }
                }
            };
            validateContent(validateContent, layout.content());
        }
    }
    if (valid) {
        context.logMessage(LogMsg() << "validated all layouts.");
    }
}

static void validateScores(json jsonData, const MnxValidateContext& context)
{
    bool valid = true;
    if (nodeExists(jsonData, "scores", false)) {  // scores are *not* required in MNX
        if (!jsonData["scores"].is_array()) {
            throw std::invalid_argument("Scores node in validated JSON is not an array!");
        }
        for (const auto& score : jsonData["scores"]) {
            if (score.contains("layout")) {
                if (!context.getLayoutIndex(score["layout"], "Score " + score["name"].dump())) {
                    valid = false;
                }
            }
            if (score.contains("multimeasureRests")) {
                for (const auto& mmRest : score["multimeasureRests"]) {
                    if (auto index = context.getMeasureIndex(mmRest["start"],"Multimeasure rest in score " + score["name"].dump())) {
                        if (*index + mmRest["duration"].get<size_t>() >= context.measCount) {
                            context.logMessage(LogMsg() << "Multimeasure rest at measure " + std::to_string(mmRest["start"].get<int>()) + " in score "
                                + score["name"].dump() + " spans non-existent measures.", LogSeverity::Error);
                            valid = false;
                        }
                    } else {
                        valid = false;
                    }
                }
            }
            if (score.contains("pages")) {
                for (size_t x = 0; x < score["pages"].size(); x++) {
                    const auto& page = score["pages"][x];
                    if (page.contains("layout")) {
                        if (!context.getLayoutIndex(page["layout"], "Page[" + std::to_string(x) + "] in score " + score["name"].dump())) {
                            valid = false;
                        }
                    }
                    if (nodeExists(page, "systems")) { // "systems" is required
                        for (size_t y = 0; y < page["systems"].size(); y++) {
                            const auto& system = page["systems"][y];
                            if (system.contains("layout")) {
                                if (!context.getLayoutIndex(system["layout"], "System[" + std::to_string(y)
                                            + "] in page[" + std::to_string(x) + "] in score " + score["name"].dump())) {
                                    valid = false;
                                }
                            }
                            if (!context.getMeasureIndex(system["measure"], "System[" + std::to_string(y)
                                            + "] in page[" + std::to_string(x) + "] in score " + score["name"].dump())) {
                                valid = false;
                            }
                            if (system.contains("layoutChanges")) {
                                for (size_t z = 0; z < system["layoutChanges"].size(); z++) {
                                    const auto& layoutChange = system["layoutChanges"][z];
                                    if (nodeExists(layoutChange, "layout")) { // "layout" is required
                                        if (!context.getLayoutIndex(layoutChange["layout"], "Layout change[" + std::to_string(z) + "] in system[" + std::to_string(y)
                                                    + "] in page[" + std::to_string(x) + "] in score " + score["name"].dump())) {
                                            valid = false;
                                        }
                                        /// @todo validate location.bar
                                        /// @todo perhaps eventually flag location.position.fraction if it is too large for the measure
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (valid) {
        context.logMessage(LogMsg() << "validated all scores.");
    }
}

void MnxValidateContext::processFile(const std::filesystem::path inpFilePath) const
{
    try {
        if (!std::filesystem::is_regular_file(inpFilePath) && !forTestOutput()) {
            throw std::runtime_error("Input file " + inpFilePath.u8string() + " does not exist or is not a file.");
        }
        constexpr char kProcessingMessage[] = "Processing File: ";
        constexpr size_t kProcessingMessageSize = sizeof(kProcessingMessage) - 1; // account for null terminator.
        std::string delimiter(kProcessingMessageSize + inpFilePath.u32string().size(), '='); // use u32string().size to get actual number of characters displayed
        // log header for each file
        logMessage(LogMsg(), true);
        logMessage(LogMsg() << delimiter, true);
        logMessage(LogMsg() << kProcessingMessage << inpFilePath.u8string(), true);
        logMessage(LogMsg() << delimiter, true);
        resetForFile(inpFilePath); // reset after logging the header

        bool success = validateJsonAgainstSchema(inputFilePath, *this);
        if (success && !schemaOnly) {
            // these calls are order-dependent
            validateGlobal(*this);
            validateParts(jsonData, *this);
            validateLayouts(jsonData, *this);
            validateScores(jsonData, *this);
        }
    } catch (const std::exception& e) {
        logMessage(LogMsg() << e.what(), true, LogSeverity::Error);
    }
}

} // namespace mnxvalidate
