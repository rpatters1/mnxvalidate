@echo off
REM Simple build script for the project

REM Set our working directory
cd /d %~dp0/..

REM Check if the environment variable is set
if "%MNXVALIDATE_PRODUCTION_DEPLOY_PATH%"=="" (
    echo ERROR: MNXVALIDATE_PRODUCTION_DEPLOY_PATH environment variable is not set.
    exit /b 1
)

REM Set default build directory
set BUILD_DIR=build
set LOCAL_DEPLOY_DIR=mnxvalidate

echo Using build directory: %BUILD_DIR%
echo Using local deploy directory: %LOCAL_DEPLOY_DIR%
echo Using production deploy path: %MNXVALIDATE_PRODUCTION_DEPLOY_PATH%
echo Using configuration: Release

REM Clean CMake configuration
cmake -P build.cmake clean
if errorlevel 1 (
    echo CMake clean failed.
    exit /b %errorlevel%
)

REM Run CMake configuration
cmake -S . -B "%BUILD_DIR%"
if errorlevel 1 (
    echo CMake configuration failed.
    exit /b %errorlevel%
)

REM Build the project
cmake --build "%BUILD_DIR%" --config Release
if errorlevel 1 (
    echo Build failed.
    exit /b %errorlevel%
)

REM Copy denigma.exe to production deploy directory
if not exist "%MNXVALIDATE_PRODUCTION_DEPLOY_PATH%" (
    echo Production deploy directory does not exist. Creating it...
    mkdir "%MNXVALIDATE_PRODUCTION_DEPLOY_PATH%"
)

echo Copying mnxvalidate.exe to %MNXVALIDATE_PRODUCTION_DEPLOY_PATH%...
copy "%LOCAL_DEPLOY_DIR%\mnxvalidate.exe" "%MNXVALIDATE_PRODUCTION_DEPLOY_PATH%"
if errorlevel 1 (
    echo Failed to copy mnxvalidate.exe to production deploy directory.
    exit /b %errorlevel%
)

echo Build and deploy completed successfully.
exit /b 0
