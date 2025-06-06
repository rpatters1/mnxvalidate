# Minimum version of CMake required
cmake_minimum_required(VERSION 3.22)

# Project name and version
project(mnxvalidate VERSION 2.0)

# Specify C++ standard and minimum macOS version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Prevent in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please use a separate directory for building.\n"
                        "For example:\n"
                        "  cmake -S . -B build\n"
                        "  cmake --build build"
    )
endif()

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    #set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
    #message(STATUS "No build type specified. Defaulting to Release.")
endif()

if (NOT CMAKE_CONFIGURATION_TYPES) # Only applies to single-config generators
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()

message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Configution Types: ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "Generator: ${CMAKE_GENERATOR}")

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        # Build for both arm64 and x86_64 in Release mode
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures for Release" FORCE)
    else()
        # Use CMAKE_SYSTEM_PROCESSOR for Debug mode to match the current architecture
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
            set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "macOS architecture for Debug" FORCE)
        elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64")
            set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "macOS architecture for Debug" FORCE)
        else()
            message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    endif()
    message(STATUS "macOS Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    message(STATUS "macOS Architectures: ${CMAKE_OSX_ARCHITECTURES}")
endif()

if(MSVC)
    add_compile_options(/Zc:__cplusplus) # force MSVC to report correct c++ version
    add_compile_options(/DNOMINMAX) # do not use min and max macros from Win32
    add_compile_options(/bigobj) # stoopid Microsoft can't handle industrial size without holding its hand
else()
    add_compile_options(-Wno-c++20-extensions)
endif()

add_compile_definitions(
    MNXVALIDATE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    MNXVALIDATE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    MNXVALIDATE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    MNXVALIDATE_VERSION="${PROJECT_VERSION}"
)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF)

# Define output directory for executables
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/build")

#define other directories
set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")

# Set output directory for compiled executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# Save the current state and disable deprecation warnings
set(_original_WARN_DEPRECATED ${CMAKE_WARN_DEPRECATED})
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "Suppress deprecation warnings for external projects" FORCE)

# Fetch miniz-cpp
FetchContent_Declare(
    miniz-cpp
    GIT_REPOSITORY https://github.com/rpatters1/miniz-cpp.git  #Use my fork until/unless PR #27 is merged on the upstream repo
    GIT_TAG        3097a4b80528ebad5a51cd9315a39a9b5fa12e54
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    SOURCE_SUBDIR not_a_directory # this is a hack to prevent FetchContent_Declare from running the fetched content cmake.
    # see https://stackoverflow.com/questions/79261625/cmake-fetchcontent-with-header-only-project/79261858#79261858
)
FetchContent_MakeAvailable(miniz-cpp)

include(cmake/Dependencies.cmake) # GitHub branches/tags for MNX and MUSX

# Define a cache variable for the local MNX C++ DOM path relative to the source directory.
set(MNXDOM_LOCAL_PATH "" CACHE STRING "Path to local MNX C++ DOM checkout relative to the source directory (if provided)")

if(MNXDOM_LOCAL_PATH)
    # Combine the source directory with the relative path to get an absolute path.
    get_filename_component(MNXDOM_LOCAL_PATH_ABS "${CMAKE_SOURCE_DIR}/${MNXDOM_LOCAL_PATH}" ABSOLUTE)
    if(EXISTS "${MNXDOM_LOCAL_PATH_ABS}/CMakeLists.txt")
        message(STATUS "Local MNX C++ DOM found at ${MNXDOM_LOCAL_PATH_ABS}")
        # Set the FetchContent source directory to the computed absolute path.
        set(FETCHCONTENT_SOURCE_DIR_MNXDOM "${MNXDOM_LOCAL_PATH_ABS}" CACHE INTERNAL "")
    else()
        message(FATAL_ERROR "Local MNX C++ DOM not found at ${MNXDOM_LOCAL_PATH_ABS}")
    endif()
else()
    message(STATUS "Using GitHub for MNX C++ DOM (${MNXDOM_GIT_TAG_OR_BRANCH})")
endif()

option(mnxdom_BUILD_TESTING "Enable testing mnxdom" ON)
# Fetch mnxdom
FetchContent_Declare(
    mnxdom
    GIT_REPOSITORY https://github.com/rpatters1/mnxdom.git
    GIT_TAG        ${MNXDOM_GIT_TAG_OR_BRANCH}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(mnxdom)

FetchContent_GetProperties(mnxdom)
message(STATUS "mnxdom Source Directory: ${mnxdom_SOURCE_DIR}")

# Restore the original state
set(CMAKE_WARN_DEPRECATED ${_original_WARN_DEPRECATED} CACHE BOOL "Restore original deprecation warnings setting" FORCE)

add_custom_target(PrintConfig ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Active build configuration: $<CONFIG>"
)

include("${CMAKE_SOURCE_DIR}/cmake/GenerateLicenseXxd.cmake")

# Add executable target
add_executable(mnxvalidate
    src/main.cpp
    src/mnxvalidate.cpp
    src/about.cpp
    src/utils/ziputils.cpp
)

# For the mnxvalidate target specifically
if (NOT CMAKE_CONFIGURATION_TYPES) # Only applies to single-config generators
    set_target_properties(mnxvalidate PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}
    )
endif()

# Compile definitions
#set(MUSX_USE_TINYXML2 ON CACHE BOOL "Disable tinyxml2 parsing classes" FORCE)
#set(MUSX_USE_RAPIDXML ON CACHE BOOL "Enable rapidxml parsing classes" FORCE)
set(MUSX_USE_PUGIXML ON CACHE BOOL "Enable pugixml parsing classes" FORCE)

# temporary for reference and debugging
set(MUSX_DISPLAY_NODE_NAMES ON CACHE BOOL "Write node names to std::cout as they are processed" FORCE)
#set(MUSX_THROW_ON_UNKNOWN_XML ON CACHE BOOL "Enable throwing on unknown XML" FORCE)
#set(MUSX_THROW_ON_INTEGRITY_CHECK_FAIL ON CACHE BOOL "Enable throwing integrity check failures" FORCE)

# Ensure the include directories are added
add_dependencies(mnxvalidate GenerateLicenseXxd)
target_include_directories(mnxvalidate PRIVATE  "${FETCHCONTENT_BASE_DIR}/ezgz-src")
target_include_directories(mnxvalidate SYSTEM PRIVATE  "${FETCHCONTENT_BASE_DIR}/miniz-cpp-src")
target_include_directories(mnxvalidate PRIVATE ${GENERATED_DIR})
target_include_directories(mnxvalidate PRIVATE ${MUSX_OBJECT_MODEL_DIR})
target_include_directories(mnxvalidate PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Ensure the libraries are added
target_link_libraries(mnxvalidate PRIVATE mnxdom)

# Define an interface library for precompiled headers
add_library(mnxvalidate_pch INTERFACE)

# Add Precompiled Headers (PCH) for the test suite
target_precompile_headers(mnxvalidate_pch INTERFACE
    <algorithm>
    <functional>
    <string>
    <memory>
    <mnxdom.h>
)

# Link the interface library to mnxvalidate
target_link_libraries(mnxvalidate PUBLIC mnxvalidate_pch)

set(DEPLOY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mnxvalidate)

if(MSVC)
    add_custom_command(
        TARGET mnxvalidate
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPLOY_DIR}
        COMMAND cmd.exe /c if "$(Configuration)" == "Release" echo Copying Release build of mnxvalidate to deploy directory
        COMMAND cmd.exe /c if "$(Configuration)" == "Release" "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:mnxvalidate>" "${DEPLOY_DIR}"
    )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring POST_BUILD command for Release.")
    add_custom_command(
        TARGET mnxvalidate
        POST_BUILD
        COMMAND echo Copying Release build of mnxvalidate to deploy directory
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPLOY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:mnxvalidate> ${DEPLOY_DIR}
        VERBATIM
    )
endif()

option(mnxvalidate_BUILD_TESTING "Build the MnxValidate test suite" ON)

if(mnxvalidate_BUILD_TESTING)
    message(STATUS "Configuring tests for mnxvalidate_BUILD_TESTING.")
    add_subdirectory(tests)
else()
    message(STATUS "Testing not enabled for mnxvalidate_BUILD_TESTING.")
endif()
