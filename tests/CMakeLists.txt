# Only configure tests if mnxvalidate_BUILD_TESTING is ON
if(mnxvalidate_BUILD_TESTING)

    # Enable testing in this directory
    enable_testing()

    # Collect all .cpp files in the source directory
    file(GLOB MNXVALIDATE_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp"
                            "${CMAKE_SOURCE_DIR}/src/utils/*.cpp")

    # Add an executable for the test suite
    add_executable(mnxvalidate_tests
        mnxvalidatetests.cpp
        ${MNXVALIDATE_SOURCES}
    )

    if (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
        # Extra warnings for Clang/AppleClang
        target_compile_options(mnxvalidate_tests PRIVATE -Wall -Wextra -Werror -Wpedantic)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        # Extra warnings for MSVC
        target_compile_options(mnxvalidate_tests PRIVATE /W4 /WX)
    endif()

    # Set the test app's output directory
    set_target_properties(mnxvalidate_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )

    # Include the necessary directories
    target_include_directories(mnxvalidate_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src       # Source files
        ${FETCHCONTENT_BASE_DIR}/miniz-cpp-src
        ${GENERATED_DIR}              # Generated files
    )

    # Link libraries used by mnxvalidate
    target_link_libraries(mnxvalidate_tests PRIVATE
        mnxvalidate_pch                   # Precompiled headers
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator
    )

    # Define testing-specific preprocessor macro
    target_compile_definitions(mnxvalidate_tests PRIVATE MNXVALIDATE_TEST)

    # Add test dependencies
    add_dependencies(mnxvalidate_tests GenerateMnxSchemaXxd)

    # Add Google Test as a dependency
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/release-1.12.1.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    # Link Google Test libraries
    target_link_libraries(mnxvalidate_tests PRIVATE gtest gmock gtest_main)

    # Discover tests
    include(GoogleTest)
    gtest_discover_tests(mnxvalidate_tests
        DISCOVERY_TIMEOUT 60 # Set timeout to 60 seconds
        PROPERTIES
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
            ENVIRONMENT "GTEST_COLOR=1"
        EXTRA_ARGS --gtest_color=yes
    )

endif()
