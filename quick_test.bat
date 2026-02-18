@echo off
setlocal enabledelayedexpansion

echo ========================================
echo 3D Video Studio - 快速测试执行
echo ========================================
echo.

REM 检查构建目录
if not exist "build" (
    echo [1/5] 创建构建目录...
    mkdir build
    if %errorlevel% neq 0 (
        echo [ERROR] 无法创建构建目录
        pause
        exit /b 1
    )
    echo [OK] 构建目录已创建
) else (
    echo [1/5] 构建目录已存在
)
echo.

REM 检查CMake
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] CMake 未安装或未在PATH中
    echo.
    echo 请先安装 CMake:
    echo   1. 访问 https://cmake.org/download/
    echo   2. 下载 Windows x64 Installer
    echo   3. 运行安装程序
    echo   4. 确保添加到系统PATH
    pause
    exit /b 1
)
echo [2/5] CMake 检查通过
echo.

REM 进入构建目录
cd build

REM 配置CMake
echo [3/5] 配置 CMake 项目...
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
if %errorlevel% neq 0 (
    echo [ERROR] CMake 配置失败
    echo.
    echo 可能的原因:
    echo   1. 缺少 Visual Studio
    echo   2. CMakeLists.txt 中有错误
    echo   3. 依赖库未找到
    echo.
    echo 请检查错误信息并修复
    pause
    exit /b 1
)
echo [OK] CMake 配置成功
echo.

REM 构建测试
echo [4/5] 构建测试套件...
cmake --build . --target v3d_tests --config Debug
if %errorlevel% neq 0 (
    echo [ERROR] 构建失败
    echo.
    echo 可能的原因:
    echo   1. 编译错误
    echo   2. 链接错误
    echo   3. 缺少依赖库
    echo.
    echo 请检查编译错误信息并修复
    pause
    exit /b 1
)
echo [OK] 构建成功
echo.

REM 检查测试可执行文件
if not exist "bin\v3d_tests.exe" (
    echo [ERROR] 测试可执行文件未找到
    echo       预期位置: build\bin\v3d_tests.exe
    pause
    exit /b 1
)

REM 运行测试
echo [5/5] 运行测试...
echo.
echo ========================================
echo 开始执行测试
echo ========================================
echo.

bin\v3d_tests.exe --gtest_color=yes

set TEST_RESULT=%errorlevel%

echo.
echo ========================================
echo 测试执行完成
echo ========================================
echo.

if %TEST_RESULT% equ 0 (
    echo [SUCCESS] 所有测试通过！
) else (
    echo [FAILED] 部分测试失败
    echo.
    echo 请查看上面的测试结果
)

echo.
pause
