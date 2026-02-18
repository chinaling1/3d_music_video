@echo off
setlocal

echo ========================================
echo 环境检查
echo ========================================
echo.

echo [1] 检查 Python...
python --version 2>nul
if %errorlevel% equ 0 (
    echo [OK] Python 已安装
    python --version
) else (
    echo [FAIL] Python 未安装
)
echo.

echo [2] 检查 Git...
git --version 2>nul
if %errorlevel% equ 0 (
    echo [OK] Git 已安装
    git --version
) else (
    echo [FAIL] Git 未安装
)
echo.

echo [3] 检查 CMake...
cmake --version 2>nul
if %errorlevel% equ 0 (
    echo [OK] CMake 已安装
    cmake --version
) else (
    echo [FAIL] CMake 未安装
)
echo.

echo [4] 检查 Visual Studio 编译器...
cl 2>nul
if %errorlevel% equ 0 (
    echo [OK] Visual Studio 编译器可用
) else (
    echo [INFO] Visual Studio 编译器未在当前PATH中
    echo       可能需要使用 "开发人员命令提示符 for VS 2022"
)
echo.

echo [5] 检查 Google Test...
if exist "C:\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test 已安装
) else if exist "C:\Program Files\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test 已安装
) else (
    echo [INFO] Google Test 未找到
    echo       可以使用 vcpkg 安装: vcpkg install gtest:x64-windows
)
echo.

echo [6] 检查项目文件...
if exist "E:\3d\CMakeLists.txt" (
    echo [OK] CMakeLists.txt 存在
) else (
    echo [FAIL] CMakeLists.txt 不存在
)

if exist "E:\3d\src" (
    echo [OK] src 目录存在
) else (
    echo [FAIL] src 目录 不存在
)

if exist "E:\3d\tests" (
    echo [OK] tests 目录存在
) else (
    echo [FAIL] tests 目录不存在
)
echo.

echo ========================================
echo 检查完成
echo ========================================
echo.

echo 下一步操作：
echo   1. 如果 CMake 已安装但未在 PATH 中，请添加到 PATH
echo   2. 如果需要编译 C++ 项目，请使用 "开发人员命令提示符 for VS 2022"
echo   3. 如果需要 Google Test，请使用 vcpkg 安装: vcpkg install gtest:x64-windows
echo   4. 运行 quick_test.bat 开始构建和测试
echo.

pause
