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
#include <map>
#include <optional>
#include <memory>
#include <chrono>
#include <regex>

#include "mnxvalidate.h"
#include "utils/stringutils.h"

static int showHelpPage(const std::string_view& programName)
{
    std::cout << "Usage: " << programName << " <command> <input-pattern> [--options]" << std::endl;
    std::cout << std::endl;

    // General options
    std::cout << "General options:" << std::endl;
    std::cout << "  --about                         Show acknowledgements and exit" << std::endl;
    std::cout << "  --exclude folder-name           Exclude the specified folder name from recursive searches" << std::endl;
    std::cout << "  --help                          Show this help message and exit" << std::endl;
    std::cout << "  --force                         Overwrite existing file(s)" << std::endl;
    std::cout << "  --part [optional-part-name]     Process named part or first part if name is omitted" << std::endl;
    std::cout << "  --recursive                     Recursively search subdirectories of the input directory" << std::endl;
    std::cout << "  --all-parts                     Process all parts and score" << std::endl;
    std::cout << "  --version                       Show program version and exit" << std::endl;
    std::cout << std::endl;

    std::cout << std::endl;
    std::cout << "By default, if the input is a single file, messages are sent to std::cerr." << std::endl;
    std::cout << "If the input is multiple files, messages are logged in `" << programName << "-logs` in the top-level input directory." << std::endl;
    std::cout << std::endl;
    std::cout << "Logging options:" << std::endl;
    std::cout << "  --log [optional-logfile-path]   Always log messages instead of sending them to std::cerr" << std::endl;
    std::cout << "  --no-log                        Always send messages to std::cerr (overrides any other logging options)" << std::endl;
    std::cout << "  --quiet                         Only display errors and warning messages (overrides --verbose)" << std::endl;
    std::cout << "  --verbose                       Verbose output" << std::endl;
    std::cout << std::endl;
    std::cout << "Any relative path is relative to the parent path of the input file or (for log files) to the top-level input folder." << std::endl;

    return 1;
}

using namespace mnxvalidate;

int _MAIN(int argc, arg_char* argv[])
{
    if (argc <= 0) {
        std::cerr << "Error: argv[0] is unavailable" << std::endl;
        return 1;
    }

    MnxValidateContext mnxvalidateContext(std::filesystem::path(*argv).stem().native());
    
    if (argc < 2) {
        return showHelpPage(mnxvalidateContext.programName);
    }

    std::vector<const arg_char*> args = mnxvalidateContext.parseOptions(argc, argv);

    if (mnxvalidateContext.showVersion) {
        std::cout << mnxvalidateContext.programName << " " << MNXVALIDATE_VERSION << std::endl;
        return 0;
    }
    if (mnxvalidateContext.showHelp) {
        showHelpPage(mnxvalidateContext.programName);
        return 0;
    }
    if (mnxvalidateContext.showAbout) {
        showAboutPage();
        return 0;
    }
    if (args.size() < 2) {
        std::cerr << "Not enough arguments passed" << std::endl;
        return showHelpPage(mnxvalidateContext.programName);
    }

    try {
        const std::filesystem::path rawInputPattern = args[1];
        std::filesystem::path inputFilePattern = rawInputPattern;

        // collect inputs
        const bool isSpecificFileOrDirectory = inputFilePattern.u8string().find('*') == std::string::npos && inputFilePattern.u8string().find('?') == std::string::npos;
        bool isSpecificFile = isSpecificFileOrDirectory && inputFilePattern.has_filename();
        if (std::filesystem::is_directory(inputFilePattern)) {
            isSpecificFile = false;
        }
        std::filesystem::path inputDir = inputFilePattern.parent_path();
        bool inputIsOneFile = std::filesystem::is_regular_file(inputFilePattern);        
        if (!inputIsOneFile && !isSpecificFile && !mnxvalidateContext.logFilePath.has_value()) {
            mnxvalidateContext.logFilePath = "";
        }
        mnxvalidateContext.startLogging(inputDir, argc, argv);

        if (isSpecificFileOrDirectory && !std::filesystem::exists(rawInputPattern) && !mnxvalidateContext.forTestOutput()) {
            throw std::runtime_error("Input path " + inputFilePattern.u8string() + " does not exist or is not a file or directory.");
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
        auto iterate = [&](auto& iterator) {
            for (auto it = iterator; it != std::filesystem::end(iterator); ++it) {
                const auto& entry = *it;
                if constexpr (std::is_same_v<std::remove_reference_t<decltype(iterator)>, std::filesystem::recursive_directory_iterator>) {
                    if (entry.is_directory()) {
                    }
                }
                if (!entry.is_directory()) {
                    mnxvalidateContext.logMessage(LogMsg() << "considered file " << entry.path().u8string(), LogSeverity::Verbose);
                }
                if (entry.is_regular_file() && std::regex_match(entry.path().filename().native(), regex)) {
                    auto inputFilePath = entry.path();
                }    
            }
        };
        if (inputIsOneFile || (mnxvalidateContext.forTestOutput() && isSpecificFile)) {
            pathsToProcess.push_back(inputFilePattern);
        } else if (mnxvalidateContext.recursiveSearch) {
            std::filesystem::recursive_directory_iterator it(inputDir);
            iterate(it);
        } else {
            std::filesystem::directory_iterator it(inputDir);
            iterate(it);
        }
        //for (const auto& path : pathsToProcess) {
          //  mnxvalidateContext.inputFilePath = "";
            //mnxvalidateContext.processFile(currentCommand, path, args);
        //}
    }
    catch (const std::exception& e) {
        mnxvalidateContext.logMessage(LogMsg() << e.what(), LogSeverity::Error);
    }

    mnxvalidateContext.endLogging();

    return mnxvalidateContext.errorOccurred;
}
