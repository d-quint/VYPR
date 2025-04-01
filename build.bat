@echo off
echo Building Vypr compiler...

:: Create build directory if it doesn't exist
if not exist build mkdir build

:: Navigate to build directory
cd build

:: Configure with CMake
echo Configuring with CMake...
cmake .. -G "MinGW Makefiles"

:: Build the project
echo Building project...
cmake --build .

:: Check if build was successful
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo.
    echo To compile a Vypr program, use:
    echo vypr.exe path\to\your\program.vy
    echo.
    echo Example:
    echo vypr.exe ..\examples\hello_world.vy
) else (
    echo Build failed!
    exit /b 1
) 