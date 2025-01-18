# mnxvalidate command line utility

This utility reads one or more `.mnx` or `.json` files and produces a report showing validation errors for MNX music notation files.

Use the `--help` option to get a full list of commands:

```
mnxvalidate --help
```

## Setup Instructions

Clone the GitHub repository and clone all submodules.

### macOS-Specific

Install the latest cmake:

```bash
brew install cmake
brew install ninja
```

---

### Windows-Specific

You must install cmake and xxd. The easiest way is with a package manager such as Choclatey (`choco`).

[Choclatey install instructions](https://chocolatey.org/install)

Install the latest cmake and xxd

```bat
choco install cmake
choco install ninja
choco install xxd
```
---

## Build Instructions


```bash
cmake -P build.cmake
```

or (for Linux or macOS)

```bash
./build.cmake
```
---

You can clean the build directory with

```bash
cmake -P build.cmake -- clean
```

or (for Linux or macOS)

```bash
./build.cmake -- clean
```

## Visual Studio Code Setup

1. Install the following extensions:
   - C/C++ (from Microsoft)
   - C/C++ Extension Pack (from Microsoft)
   - C/C++ Themes (from Microsoft)
   - CMake (from twxs)
   - CMake Tools (from Microsoft)
   - codeLLDB (from Vadim Chugunov)
2. Use the following `.vscode/tasks.json` file:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "cmake --build build",
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]
}
```

3. Use the following `.vscode/launch.json` for debugging on macOS:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "C++ Debug (codeLLDB)",
            "type": "lldb",
            "request": "launch",
            "program": "${workspaceFolder}/build/build/Debug/mnxvalidate",
            "args": [], // specify command line arguments here for testing
            "cwd": "${workspaceFolder}",
            "stopOnEntry": false,
            "env": {},
            "preLaunchTask": "build" // Optional: specify a task to build your program before debugging
        }
    ]
}
```

on Windows:

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "C++ Debug (codeLLDB)",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/build/Debug/mnxvalidate",
            "args":[],  // specify command line arguments here for testing
            "console": "externalTerminal",
            "cwd": "${workspaceFolder}",
            "stopAtEntry": false,
            "environment": [],
            "preLaunchTask": "build" // Optional: specify a task to build your program before debugging
        }
    ]
}
```

on Ubuntu:

```json
{
    "version": "0.2.0",
    "configurations": [
      {
        "name": "Debug",
        "type": "cppdbg",
        "request": "launch",
        "program": "${workspaceFolder}/build/build/Debug/mnxvalidate",
        "args": [],  // specify command line arguments here for testing
        "stopAtEntry": false,
        "cwd": "${workspaceFolder}",
        "environment": [],
        "externalConsole": true,
        "MIMode": "gdb",
        "setupCommands": [
          {
            "description": "Enable pretty-printing for gdb",
            "text": "-enable-pretty-printing",
            "ignoreFailures": true
          }
        ],
        "preLaunchTask": "Build"
      }
    ]
}
```
