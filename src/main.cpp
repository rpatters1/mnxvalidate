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
#include <optional>
#include <memory>
#include <chrono>
#include <regex>
#include <unordered_set>

#include "mnxvalidate.h"
#include "utils/stringutils.h"

namespace {

struct PathHash
{
    auto operator()(const std::filesystem::path& p) const noexcept {
        return std::filesystem::hash_value(p);
    }
};

std::filesystem::path normalizePathForDedupe(const std::filesystem::path& path)
{
    std::error_code ec;
    std::filesystem::path absolutePath = std::filesystem::absolute(path, ec);
    if (!ec) {
        return absolutePath.lexically_normal();
    }
    return path.lexically_normal();
}

} // namespace

static int showHelpPage(const std::string_view& programName)
{
    std::cout << "Usage: " << programName << " <input-pattern> [<input-pattern>...] [--options]" << std::endl;
    std::cout << std::endl;

    // General options
    std::cout << "General options:" << std::endl;
    std::cout << "  --about                         Show acknowledgements and exit" << std::endl;
    std::cout << "  --help                          Show this help message and exit" << std::endl;
    std::cout << "  --recursive                     Recursively search subdirectories of the input directory" << std::endl;
    std::cout << "  --schema [file-path]            Validate against this json schema file rather than the embedded one." << std::endl;
    std::cout << "  --schema-only                   Only validate against the schema. Perform no other validation checks." << std::endl;
    std::cout << "  --version                       Show program version and exit" << std::endl;
    std::cout << std::endl;

    std::cout << std::endl;
    std::cout << "By default, messages are sent to std::cerr." << std::endl;
    std::cout << std::endl;
    std::cout << "Logging options:" << std::endl;
    std::cout << "  --log [optional-logfile-path]   Log messages to file instead of sending them to std::cerr" << std::endl;
    std::cout << "  --no-log                        Always send messages to std::cerr (overrides any other logging options)" << std::endl;
    std::cout << "  --quiet                         Only display errors and warning messages (overrides --verbose)" << std::endl;
    std::cout << "  --verbose                       Verbose output" << std::endl;
    std::cout << std::endl;
    std::cout << "Relative input patterns are resolved from the current working directory." << std::endl;
    std::cout << "Relative log paths for --log are resolved from the first input pattern's parent directory." << std::endl;

    return 1;
}

using namespace mnxvalidate;

void processInputPathArg(const std::filesystem::path& rawInputPattern, MnxValidateContext& mnxValidateContext, int argc, arg_char* argv[],
                         std::unordered_set<std::filesystem::path, PathHash>& seenPaths)
{
    std::filesystem::path inputFilePattern = rawInputPattern;

    // collect inputs
    const auto inputPatternU8 = inputFilePattern.u8string();
    const bool isSpecificFileOrDirectory = inputPatternU8.find(u8'*') == std::u8string::npos && inputPatternU8.find(u8'?') == std::u8string::npos;
    bool isSpecificFile = isSpecificFileOrDirectory && inputFilePattern.has_filename();
    if (std::filesystem::is_directory(inputFilePattern)) {
        isSpecificFile = false;
        inputFilePattern /= "*.*";
    }
    std::filesystem::path inputDir = inputFilePattern.parent_path();
    if (inputDir.is_relative()) {
        inputDir = std::filesystem::current_path() / inputDir;
    }
    bool inputIsOneFile = std::filesystem::is_regular_file(inputFilePattern);
    mnxValidateContext.startLogging(inputDir, argc, argv);

    if (mnxValidateContext.mnxSchemaPath.has_value() && !mnxValidateContext.mnxSchema.has_value()) {
        mnxValidateContext.mnxSchema = utils::fileToString(mnxValidateContext.mnxSchemaPath.value());
    }

    if (isSpecificFileOrDirectory && !std::filesystem::exists(rawInputPattern) && !mnxValidateContext.forTestOutput()) {
        throw std::runtime_error("Input path " + utils::pathToString(inputFilePattern) + " does not exist or is not a file or directory.");
    }

    // convert wildcard pattern to regex
    auto wildcardPattern = inputFilePattern.filename().native(); // native format avoids encoding issues
#ifdef _WIN32
    auto regexPattern = std::regex_replace(wildcardPattern, std::wregex(LR"(\*)"), L".*");
    regexPattern = std::regex_replace(regexPattern, std::wregex(LR"(\?)"), L".");
    std::wregex regex(regexPattern);
#else
    auto regexPattern = std::regex_replace(wildcardPattern, std::regex(R"(\*)"), R"(.*)");
    regexPattern = std::regex_replace(regexPattern, std::regex(R"(\?)"), R"(.)");
    std::regex regex(regexPattern);
#endif

    // collect files to process first
    // this avoids potential infinite recursion if input and output are the same format
    std::vector<std::filesystem::path> pathsToProcess;
    auto appendUniquePath = [&](const std::filesystem::path& inputFilePath) {
        const auto normalizedPath = normalizePathForDedupe(inputFilePath);
        if (seenPaths.emplace(normalizedPath).second) {
            pathsToProcess.emplace_back(inputFilePath);
        }
    };
    auto iterate = [&](auto& iterator) {
        for (auto it = iterator; it != std::filesystem::end(iterator); ++it) {
            const auto& entry = *it;
            if constexpr (std::is_same_v<std::remove_reference_t<decltype(iterator)>, std::filesystem::recursive_directory_iterator>) {
                if (entry.is_directory()) {
                }
            }
            if (!entry.is_directory()) {
                mnxValidateContext.logMessage(LogMsg() << "considered file " << utils::pathToString(entry.path()), LogSeverity::Verbose);
            }
            if (entry.is_regular_file() && std::regex_match(entry.path().filename().native(), regex)) {
                auto inputFilePath = entry.path();
                if (utils::hasExtension(inputFilePath, MNX_EXTENSION) || utils::hasExtension(inputFilePath, JSON_EXTENSION)) {
                    appendUniquePath(inputFilePath);
                }
            }
        }
    };
    if (inputIsOneFile || (mnxValidateContext.forTestOutput() && isSpecificFile)) {
        appendUniquePath(inputFilePattern);
    } else if (mnxValidateContext.recursiveSearch) {
        std::filesystem::recursive_directory_iterator it(inputDir);
        iterate(it);
    } else {
        std::filesystem::directory_iterator it(inputDir);
        iterate(it);
    }
    for (const auto& path : pathsToProcess) {
        mnxValidateContext.inputFilePath = "";
        mnxValidateContext.processFile(path);
    }
}

int _MAIN(int argc, arg_char* argv[])
{
    if (argc <= 0) {
        std::cerr << "Error: argv[0] is unavailable" << std::endl;
        return 1;
    }

    MnxValidateContext mnxValidateContext(std::filesystem::path(*argv).stem().native());

    std::vector<const arg_char*> args;
    try {
        args = mnxValidateContext.parseOptions(argc, argv);
    } catch (const std::exception& e) {
        mnxValidateContext.logMessage(LogMsg() << e.what(), LogSeverity::Error);
        return 1;
    }

    if (mnxValidateContext.showVersion) {
        std::cout << mnxValidateContext.programName << " " << MNXVALIDATE_VERSION << std::endl;
        return 0;
    }
    if (mnxValidateContext.showHelp) {
        showHelpPage(mnxValidateContext.programName);
        return 0;
    }
    if (mnxValidateContext.showAbout) {
        showAboutPage();
        return 0;
    }

    if (args.empty()) {
        return showHelpPage(mnxValidateContext.programName);
    }

    try {
        std::unordered_set<std::filesystem::path, PathHash> seenPaths;
        for (const auto* arg : args) {
            processInputPathArg(arg, mnxValidateContext, argc, argv, seenPaths);
        }
    } catch (const std::exception& e) {
        mnxValidateContext.logMessage(LogMsg() << e.what(), LogSeverity::Error);
    }

    mnxValidateContext.endLogging();

    return mnxValidateContext.errorOccurred;
}
