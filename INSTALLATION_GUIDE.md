# 3D Video Studio - 开发环境安装指南

## 概述

本文档提供了在Windows系统上安装3D Video Studio开发环境所需的全部工具和依赖项的详细步骤。

## 系统要求

- **操作系统**: Windows 10/11 (64位)
- **内存**: 至少8GB RAM（推荐16GB）
- **磁盘空间**: 至少10GB可用空间
- **网络连接**: 需要下载安装包

## 必需工具清单

| 工具 | 版本要求 | 用途 | 下载链接 |
|------|---------|------|---------|
| **Visual Studio** | 2022 Community | C++编译器和IDE | [下载](https://visualstudio.microsoft.com/downloads/) |
| **CMake** | 3.20+ | 构建系统 | [下载](https://cmake.org/download/) |
| **Git** | 2.x+ | 版本控制 | [下载](https://git-scm.com/download/win) |
| **Google Test** | 1.11.0+ | 测试框架 | [下载](https://github.com/google/googletest/releases) |
| **Python** | 3.6+ | 报告生成 | [下载](https://www.python.org/downloads/) |

## 详细安装步骤

### 步骤1：安装Visual Studio 2022 Community

#### 1.1 下载安装程序

1. 访问 https://visualstudio.microsoft.com/downloads/
2. 点击"Community 2022"版本的"免费下载"按钮
3. 下载 `vs_Community.exe` 安装程序

#### 1.2 运行安装程序

1. 双击运行 `vs_Community.exe`
2. 等待安装程序启动
3. 点击"继续"按钮

#### 1.3 选择工作负载

在"工作负载"选项卡中，选择以下工作负载：

- ✅ **使用C++的桌面开发** (Desktop development with C++)
  - 这将自动选择：
    - MSVC v143 - VS 2022 C++ x64/x86 生成工具
    - CMake工具
    - Windows 10 SDK
    - Windows 11 SDK

#### 1.4 开始安装

1. 点击右下角的"安装"按钮
2. 等待安装完成（可能需要30-60分钟）
3. 安装完成后，点击"重新启动"按钮

#### 1.5 验证安装

打开"开发人员命令提示符"（Developer Command Prompt for VS 2022）：

```cmd
cl
```

应该看到编译器版本信息。

### 步骤2：安装CMake

#### 2.1 下载CMake

1. 访问 https://cmake.org/download/
2. 下载Windows x64 Installer（如：`cmake-3.28.0-windows-x86_64.msi`）
3. 保存到下载文件夹

#### 2.2 安装CMake

1. 双击运行下载的MSI安装程序
2. 点击"Next"按钮
3. 接受许可协议，点击"Next"
4. 选择安装路径（默认：`C:\Program Files\CMake\bin`）
5. 勾选"Add CMake to the system PATH for all users"
6. 点击"Next"按钮
7. 点击"Install"按钮
8. 等待安装完成
9. 点击"Finish"按钮

#### 2.3 验证安装

打开新的命令提示符窗口：

```cmd
cmake --version
```

应该看到类似输出：
```
cmake version 3.28.0

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

### 步骤3：安装Git

#### 3.1 下载Git

1. 访问 https://git-scm.com/download/win
2. 下载64位Git for Windows Setup（如：`Git-2.45.0-64-bit.exe`）

#### 3.2 安装Git

1. 双击运行安装程序
2. 点击"Next"按钮
3. 选择安装路径（默认：`C:\Program Files\Git`）
4. 选择组件（保持默认）
5. 选择开始菜单文件夹（默认）
6. 选择PATH环境变量（选择"Git from the command line and also from 3rd-party software"）
7. 选择HTTPS传输后端（选择"Use the OpenSSL library"）
8. 选择行结束符转换（选择"Checkout Windows-style, commit Unix-style line endings"）
9. 选择终端模拟器（选择"Use MinTTY"）
10. 选择"git pull"行为（选择"Default"）
11. 选择凭证助手（选择"Git Credential Manager"）
12. 选择额外选项（保持默认）
13. 点击"Install"按钮
14. 等待安装完成
15. 点击"Finish"按钮

#### 3.3 验证安装

```cmd
git --version
```

应该看到类似输出：
```
git version 2.45.0.windows.1
```

### 步骤4：安装Google Test

#### 4.1 下载Google Test

1. 访问 https://github.com/google/googletest/releases
2. 下载最新版本的源代码压缩包（如：`googletest-1.14.0.zip`）
3. 解压到 `C:\googletest` 目录

#### 4.2 编译Google Test

打开"开发人员命令提示符 for VS 2022"：

```cmd
cd C:\googletest
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### 4.3 安装Google Test

```cmd
cmake --install . --config Release
```

或者手动复制文件：

```cmd
mkdir C:\googletest\include
mkdir C:\googletest\lib

xcopy /E /I ..\include C:\googletest\include
xcopy /E /I ..\lib\Release C:\googletest\lib
```

### 步骤5：安装Python

#### 5.1 下载Python

1. 访问 https://www.python.org/downloads/
2. 下载Python 3.11或更高版本的Windows installer（64位）
3. 保存到下载文件夹

#### 5.2 安装Python

1. 双击运行安装程序
2. **重要**：勾选"Add Python 3.x to PATH"
3. 点击"Install Now"按钮
4. 等待安装完成
5. 点击"Close"按钮

#### 5.3 验证安装

```cmd
python --version
```

应该看到类似输出：
```
Python 3.11.8
```

## 环境变量配置

### 检查PATH环境变量

1. 右键点击"此电脑"，选择"属性"
2. 点击"高级系统设置"
3. 点击"环境变量"
4. 在"系统变量"部分，找到并编辑"Path"变量

### 确保以下路径在PATH中：

```
C:\Program Files\CMake\bin
C:\Program Files\Git\cmd
C:\Program Files\Python311\Scripts\
C:\Program Files\Python311\
C:\googletest\lib
```

### 设置CMAKE_PREFIX_PATH（可选）

如果Google Test安装在非标准位置，设置：

```
CMAKE_PREFIX_PATH=C:\googletest
```

## 验证安装

### 创建测试脚本

创建文件 `verify_installation.bat`：

```batch
@echo off
echo ========================================
echo 验证开发环境安装
echo ========================================
echo.

echo [1/6] 检查 Visual Studio...
where cl >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Visual Studio 已安装
    cl 2>&1 | findstr /C:"Microsoft (R) C/C++ Optimizing Compiler Version"
) else (
    echo [FAIL] Visual Studio 未安装或未在PATH中
)
echo.

echo [2/6] 检查 CMake...
where cmake >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] CMake 已安装
    cmake --version
) else (
    echo [FAIL] CMake 未安装或未在PATH中
)
echo.

echo [3/6] 检查 Git...
where git >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Git 已安装
    git --version
) else (
    echo [FAIL] Git 未安装或未在PATH中
)
echo.

echo [4/6] 检查 Python...
where python >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Python 已安装
    python --version
) else (
    echo [FAIL] Python 未安装或未在PATH中
)
echo.

echo [5/6] 检查 Google Test...
if exist "C:\googletest\include\gtest\gtest.h" (
    echo [OK] Google Test 已安装
) else (
    echo [FAIL] Google Test 未找到
)
echo.

echo [6/6] 检查项目目录...
if exist "E:\3d\CMakeLists.txt" (
    echo [OK] 项目目录存在
) else (
    echo [FAIL] 项目目录不存在
)
echo.

echo ========================================
echo 验证完成
echo ========================================
pause
```

### 运行验证脚本

```cmd
cd E:\3d
verify_installation.bat
```

## 构建和运行测试

### 步骤1：配置项目

```cmd
cd E:\3d
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
```

### 步骤2：构建测试

```cmd
cmake --build . --target v3d_tests --config Debug
```

### 步骤3：运行测试

```cmd
.\bin\v3d_tests.exe
```

### 步骤4：生成覆盖率报告（可选）

```cmd
cmake --build . --target test_coverage
```

## 常见问题

### Q1: CMake找不到Visual Studio编译器

**解决方案**：
1. 使用"开发人员命令提示符 for VS 2022"而不是普通命令提示符
2. 或在CMake命令中指定生成器：
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

### Q2: 找不到Google Test

**解决方案**：
1. 设置CMAKE_PREFIX_PATH环境变量
2. 或在CMake命令中指定GTest路径：
   ```cmd
   cmake .. -DGTest_ROOT=C:\googletest
   ```

### Q3: Python脚本执行失败

**解决方案**：
1. 确保Python已添加到PATH
2. 使用完整Python路径：
   ```cmd
   C:\Python311\python.exe script.py
   ```

### Q4: 编译错误

**解决方案**：
1. 清理构建目录：
   ```cmd
   cd E:\3d
   rmdir /s /q build
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
   ```
2. 重新构建

### Q5: 链接错误

**解决方案**：
1. 确保所有依赖库都已正确安装
2. 检查CMakeLists.txt中的依赖项配置
3. 查看详细的错误日志

## 快速安装脚本

如果网络连接正常，可以使用以下PowerShell脚本自动安装：

### 自动安装脚本（install_dev_env.ps1）

```powershell
# 设置执行策略
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser -Force

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "请以管理员身份运行此脚本" -ForegroundColor Red
    exit 1
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "3D Video Studio - 开发环境自动安装"
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 1. 安装Chocolatey（如果未安装）
Write-Host "[1/5] 检查 Chocolatey..." -ForegroundColor Yellow
if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
    Write-Host "安装 Chocolatey..." -ForegroundColor Green
    Set-ExecutionPolicy Bypass -Scope Process -Force
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
    Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    Write-Host "Chocolatey 安装完成" -ForegroundColor Green
} else {
    Write-Host "Chocolatey 已安装" -ForegroundColor Green
}
Write-Host ""

# 2. 安装CMake
Write-Host "[2/5] 安装 CMake..." -ForegroundColor Yellow
choco install cmake -y
Write-Host "CMake 安装完成" -ForegroundColor Green
Write-Host ""

# 3. 安装Git
Write-Host "[3/5] 安装 Git..." -ForegroundColor Yellow
choco install git -y
Write-Host "Git 安装完成" -ForegroundColor Green
Write-Host ""

# 4. 安装Python
Write-Host "[4/5] 安装 Python..." -ForegroundColor Yellow
choco install python -y
Write-Host "Python 安装完成" -ForegroundColor Green
Write-Host ""

# 5. 刷新环境变量
Write-Host "[5/5] 刷新环境变量..." -ForegroundColor Yellow
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
Write-Host "环境变量已刷新" -ForegroundColor Green
Write-Host ""

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "安装完成！"
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "请手动安装 Visual Studio 2022 Community：" -ForegroundColor Yellow
Write-Host "https://visualstudio.microsoft.com/downloads/" -ForegroundColor Cyan
Write-Host ""
Write-Host "请手动下载并编译 Google Test：" -ForegroundColor Yellow
Write-Host "https://github.com/google/googletest/releases" -ForegroundColor Cyan
Write-Host ""
Write-Host "安装完成后，请运行 verify_installation.bat 验证安装" -ForegroundColor Green
```

### 运行自动安装脚本

1. 将上述脚本保存为 `install_dev_env.ps1`
2. 右键点击脚本，选择"使用PowerShell运行"
3. 如果提示权限问题，右键选择"以管理员身份运行"

## 下一步

安装完成后，请按照以下顺序操作：

1. ✅ 运行 `verify_installation.bat` 验证所有工具已正确安装
2. ✅ 配置CMake项目
3. ✅ 构建测试套件
4. ✅ 运行测试
5. ✅ 查看测试报告

## 技术支持

如果遇到问题：

1. 查看本文档的"常见问题"部分
2. 检查各工具的官方文档
3. 查看项目README文件
4. 联系开发团队

## 附录

### A. 推荐的IDE

- **Visual Studio Code**（推荐用于日常开发）
  - 下载：https://code.visualstudio.com/
  - 推荐插件：
    - C/C++ Extension Pack
    - CMake Tools
    - Python

- **Visual Studio 2022**（推荐用于大型项目）
  - 下载：https://visualstudio.microsoft.com/downloads/

### B. 有用的命令

```cmd
# 清理构建目录
rmdir /s /q build

# 重新配置CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

# 构建特定目标
cmake --build . --target v3d_tests

# 清理构建
cmake --build . --target clean

# 查看CMake缓存
cmake .. -LAH
```

### C. 环境变量参考

| 变量名 | 值 | 说明 |
|--------|-----|------|
| PATH | 包含所有工具的bin目录 | 系统路径 |
| CMAKE_PREFIX_PATH | C:\googletest | CMake搜索路径 |
| PYTHONPATH | C:\Python311\Lib\site-packages | Python模块路径 |

---

**文档版本**: 1.0  
**最后更新**: 2026-02-17  
**适用系统**: Windows 10/11 (64位)
