@echo off
setlocal

echo ========================================
echo Environment Check
echo ========================================
echo.

echo [1] Checking Python...
python --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Python is installed
    python --version
) else (
    echo [FAIL] Python is not installed
)
echo.

echo [2] Checking Git...
git --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Git is installed
    git --version
) else (
    echo [FAIL] Git is not installed
)
echo.

echo [3] Checking CMake...
cmake --version >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] CMake is installed
    cmake --version
) else (
    echo [FAIL] CMake is not installed
)
echo.

echo [4] Checking Visual Studio Compiler...
cl >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Visual Studio compiler is available
) else (
    echo [INFO] Visual Studio compiler not in current PATH
    echo       You may need to use "Developer Command Prompt for VS 2022"
)
echo.

echo [5] Checking Google Test...
if exist "C:\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test is installed
) else if exist "C:\Program Files\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test is installed (Program Files)
) else if exist "C:\googletest\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test is installed
) else (
    echo [INFO] Google Test not found
    echo       You can install it using vcpkg: vcpkg install gtest:x64-windows
)
echo.

echo [6] Checking Project Files...
if exist "E:\3d\CMakeLists.txt" (
    echo [OK] CMakeLists.txt exists
) else (
    echo [FAIL] CMakeLists.txt does not exist
)

if exist "E:\3d\src" (
    echo [OK] src directory exists
) else (
    echo [FAIL] src directory does not exist
)

if exist "E:\3d\tests" (
    echo [OK] tests directory exists
) else (
    echo [FAIL] tests directory does not exist
)
echo.

echo ========================================
echo Check Complete
echo ========================================
echo.

echo Next steps:
echo   1. If CMake is installed but not in PATH, add it to PATH
echo   2. If you need to compile C++ projects, use "Developer Command Prompt for VS 2022"
echo   3. If you need Google Test, install it using: vcpkg install gtest:x64-windows
echo   4. Run quick_test.bat to start building and testing
echo.

pause
