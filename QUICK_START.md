# 3D Video Studio - 测试执行快速指南

## 快速开始

如果您已经安装了所有必要的开发工具，可以按照以下步骤快速开始测试：

### 方式一：使用快速测试脚本（推荐）

```cmd
# 1. 验证安装
verify_installation.bat

# 2. 如果所有检查都通过，运行快速测试
quick_test.bat
```

### 方式二：手动执行

```cmd
# 1. 创建构建目录
mkdir build
cd build

# 2. 配置CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

# 3. 构建测试
cmake --build . --target v3d_tests --config Debug

# 4. 运行测试
.\bin\v3d_tests.exe
```

## 如果缺少工具

### 安装Visual Studio 2022 Community

1. 访问 https://visualstudio.microsoft.com/downloads/
2. 下载 "Community 2022" 版本
3. 运行安装程序
4. 选择 "使用C++的桌面开发" 工作负载
5. 点击安装

### 安装CMake

1. 访问 https://cmake.org/download/
2. 下载 Windows x64 Installer
3. 运行安装程序
4. 勾选 "Add CMake to the system PATH"
5. 点击安装

### 安装Python

1. 访问 https://www.python.org/downloads/
2. 下载 Python 3.11 或更高版本
3. 运行安装程序
4. **重要**：勾选 "Add Python 3.x to PATH"
5. 点击安装

## 测试结果

### 成功的测试输出

```
[==========] Running 200 tests from 30 test suites.
[----------] Global test environment set-up.
[----------] 200 tests from CoreTest, ConcurrencyTest, ...
[ RUN      ] CoreTest.VersionControlTest.CommandManager_ExecuteCommand
[       OK ] CoreTest.VersionControlTest.CommandManager_ExecuteCommand (5 ms)
...
[==========] 200 tests from 30 test suites ran. (5234 ms total)
[  PASSED  ] 200 tests.
```

### 失败的测试输出

```
[==========] Running 200 tests from 30 test suites.
[----------] Global test environment set-up.
[----------] 200 tests from CoreTest, ConcurrencyTest, ...
[ RUN      ] CoreTest.VersionControlTest.CommandManager_ExecuteCommand
[  FAILED  ] CoreTest.VersionControlTest.CommandManager_ExecuteCommand
...
[  FAILED  ] 5 tests, listed below:
[  FAILED  ] CoreTest.VersionControlTest.CommandManager_ExecuteCommand
[  FAILED  ] CoreTest.VersionControlTest.CommandManager_UndoRedo
...
[==========] 5 tests from 30 test suites ran. (2341 ms total)
[  PASSED  ] 195 tests.
[  FAILED  ] 5 tests.
```

## 常见问题

### Q1: CMake找不到Visual Studio编译器

**解决方案**：
1. 使用 "开发人员命令提示符 for VS 2022" 而不是普通命令提示符
2. 或在CMake命令中指定生成器：
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

### Q2: 找不到Google Test

**解决方案**：
1. 下载Google Test源代码
2. 解压到 C:\googletest
3. 使用Visual Studio编译
4. 设置CMAKE_PREFIX_PATH环境变量：
   ```cmd
   set CMAKE_PREFIX_PATH=C:\googletest
   ```

### Q3: 编译错误

**解决方案**：
1. 清理构建目录：
   ```cmd
   cd ..
   rmdir /s /q build
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
   ```
2. 重新构建

### Q4: 链接错误

**解决方案**：
1. 确保所有依赖库都已正确安装
2. 检查CMakeLists.txt中的依赖项配置
3. 查看详细的错误日志

## 测试报告

测试执行后，报告将生成在以下位置：

- XML报告: `test_reports/test_results.xml`
- JSON报告: `test_reports/summary.json`
- HTML报告: `test_reports/report.html`
- 文本报告: `test_reports/summary.txt`

## 下一步

1. ✅ 安装所有必需的开发工具
2. ✅ 运行 `verify_installation.bat` 验证安装
3. ✅ 运行 `quick_test.bat` 执行测试
4. ✅ 查看测试报告
5. ✅ 根据测试结果修复问题

## 获取帮助

如果遇到问题：

1. 查看详细安装指南: [INSTALLATION_GUIDE.md](file:///e:/3d/INSTALLATION_GUIDE.md)
2. 查看完整测试报告: [TEST_REPORT.md](file:///e:/3d/TEST_REPORT.md)
3. 查看测试执行状态: [TEST_EXECUTION_STATUS.md](file:///e:/3d/TEST_EXECUTION_STATUS.md)
4. 查看测试指南: [tests/README.md](file:///e:/3d/tests/README.md)

## 总结

测试系统已完全搭建完成，包含200+测试用例。一旦安装好开发环境，即可立即开始测试。

**测试成熟度**: ⭐⭐⭐⭐☆ (4/5)

---

**文档版本**: 1.0  
**最后更新**: 2026-02-17
