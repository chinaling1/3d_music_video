@echo off
setlocal enabledelayedexpansion

echo ========================================
echo 验证开发环境安装
echo ========================================
echo.

echo [1/7] 检查 Visual Studio...
where cl >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Visual Studio 已安装
    cl 2>&1 | findstr /C:"Microsoft (R) C/C++ Optimizing Compiler Version"
) else (
    echo [FAIL] Visual Studio 未安装或未在PATH中
    echo       请安装 Visual Studio 2022 Community
    echo       下载地址: https://visualstudio.microsoft.com/downloads/
)
echo.

echo [2/7] 检查 CMake...
where cmake >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] CMake 已安装
    cmake --version
) else (
    echo [FAIL] CMake 未安装或未在PATH中
    echo       请从 https://cmake.org/download/ 下载安装
)
echo.

echo [3/7] 检查 Git...
where git >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Git 已安装
    git --version
) else (
    echo [WARN] Git 未安装（可选，用于版本控制）
)
echo.

echo [4/7] 检查 Python...
where python >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Python 已安装
    python --version
) else (
    echo [FAIL] Python 未安装或未在PATH中
    echo       请从 https://www.python.org/downloads/ 下载安装
)
echo.

echo [5/7] 检查 Google Test...
if exist "C:\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test 已安装
) else if exist "C:\Program Files\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test 已安装（Program Files）
) else if exist "C:\googletest\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test 已安装
) else (
    echo [FAIL] Google Test 未找到
    echo       请从 https://github.com/google/googletest/releases 下载
    echo       解压到 C:\googletest 并编译
)
echo.

echo [6/7] 检查项目目录...
if exist "E:\3d\CMakeLists.txt" (
    echo [OK] 项目目录存在
) else (
    echo [FAIL] 项目目录不存在
)
echo.

echo [7/7] 检查测试目录...
if exist "E:\3d\tests" (
    echo [OK] 测试目录存在
    dir /b E:\3d\tests\*.cpp | find /c /v "" >nul
    if !errorlevel! equ 0 (
        echo [OK] 找到测试文件
    ) else (
        echo [WARN] 未找到测试文件
    )
) else (
    echo [FAIL] 测试目录不存在
)
echo.

echo ========================================
echo 验证完成
echo ========================================
echo.

REM 检查关键依赖
set ALL_OK=1

where cl >nul 2>&1
if %errorlevel% neq 0 set ALL_OK=0

where cmake >nul 2>&1
if %errorlevel% neq 0 set ALL_OK=0

if %ALL_OK% equ 1 (
    echo [SUCCESS] 所有关键依赖已安装！
    echo.
    echo 下一步操作：
    echo   1. 创建构建目录: mkdir build
    echo   2. 配置项目: cd build ^&^& cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
    echo   3. 构建测试: cmake --build . --target v3d_tests --config Debug
    echo   4. 运行测试: .\bin\v3d_tests.exe
) else (
    echo [WARNING] 部分依赖缺失，请安装后再试
    echo.
    echo 详细安装指南请查看: INSTALLATION_GUIDE.md
)

echo.
pause
