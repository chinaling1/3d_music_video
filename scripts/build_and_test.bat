@echo off
setlocal enabledelayedexpansion

echo ========================================
echo 3D Video Studio - Build and Test
echo ========================================
echo.

REM Add CMake to PATH
set "PATH=C:\Program Files\CMake\bin;%PATH%"

REM Set vcpkg toolchain
set "CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake"

echo [1/5] Checking CMake...
cmake --version
if %errorlevel% neq 0 (
    echo [ERROR] CMake not found
    exit /b 1
)
echo [OK] CMake is available
echo.

echo [2/5] Checking Python...
python --version
if %errorlevel% neq 0 (
    echo [ERROR] Python not found
    exit /b 1
)
echo [OK] Python is available
echo.

echo [3/5] Checking Git...
git --version
if %errorlevel% neq 0 (
    echo [WARN] Git not found (optional)
) else (
    echo [OK] Git is available
)
echo.

echo [4/5] Checking Google Test...
if exist "C:\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test is installed
) else if exist "C:\Program Files\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test is installed (Program Files)
) else if exist "C:\googletest\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test is installed
) else (
    echo [INFO] Google Test not found
    echo       Will try to build without GTest
)
echo.

echo [5/5] Creating build directory...
if not exist "build" (
    mkdir build
    if %errorlevel% neq 0 (
        echo [ERROR] Failed to create build directory
        exit /b 1
    )
)
echo [OK] Build directory ready
echo.

echo ========================================
echo Configuring CMake...
echo ========================================
cd build

cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed
    echo.
    echo Possible reasons:
    echo   1. Missing Visual Studio compiler
    echo   2. CMakeLists.txt has errors
    echo   3. Missing dependencies
    echo.
    echo Try using "Developer Command Prompt for VS 2022"
    exit /b 1
)

echo [OK] CMake configuration successful
echo.

echo ========================================
echo Building tests...
echo ========================================

cmake --build . --target v3d_tests --config Debug
if %errorlevel% neq 0 (
    echo [ERROR] Build failed
    echo.
    echo Check error messages above
    exit /b 1
)

echo [OK] Build successful
echo.

echo ========================================
echo Running tests...
echo ========================================

if exist "bin\v3d_tests.exe" (
    bin\v3d_tests.exe --gtest_color=yes
    set TEST_RESULT=%errorlevel%
    
    echo.
    echo ========================================
    echo Test execution complete
    echo ========================================
    
    if %TEST_RESULT% equ 0 (
        echo [SUCCESS] All tests passed!
    ) else (
        echo [FAILED] Some tests failed
    )
) else (
    echo [ERROR] Test executable not found
    echo       Expected location: build\bin\v3d_tests.exe
)

echo.
echo Build and test process completed.
