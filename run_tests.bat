@echo off
setlocal enabledelayedexpansion

echo ========================================
echo 3D Video Studio - Test Execution Script
echo ========================================

set BUILD_DIR=build
set TEST_OUTPUT_DIR=test_output
set TEST_REPORT_DIR=test_reports

echo.
echo Step 1: Creating build directory...
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

echo.
echo Step 2: Configuring CMake...
cd %BUILD_DIR%
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

echo.
echo Step 3: Building tests...
cmake --build . --target v3d_tests --config Debug -j

echo.
echo Step 4: Creating output directories...
cd ..
if not exist "%TEST_OUTPUT_DIR%" (
    mkdir "%TEST_OUTPUT_DIR%"
)
if not exist "%TEST_REPORT_DIR%" (
    mkdir "%TEST_REPORT_DIR%"
)

echo.
echo Step 5: Running tests...
cd %BUILD_DIR%
bin\v3d_tests.exe --gtest_output=xml:../%TEST_REPORT_DIR%/test_results.xml

echo.
echo Step 6: Test execution completed!
echo.
echo Check %TEST_REPORT_DIR%\test_results.xml for detailed results.

endlocal
