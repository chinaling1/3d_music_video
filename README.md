# V3D Studio

一个功能强大的3D建模和动画制作软件。

---

**English version** | [英文版本](README_EN.md)

## 目录

- [简介](#简介)
- [功能特性](#功能特性)
- [环境准备](#环境准备)
  - [系统要求](#系统要求)
  - [安装 Python 环境](#安装-python-环境)
  - [安装 C++ 编译器](#安装-c-编译器)
  - [安装 CMake](#安装-cmake)
  - [安装依赖库](#安装依赖库)
- [构建项目](#构建项目)
  - [克隆代码](#克隆代码)
  - [使用 vcpkg 安装依赖](#使用-vcpkg-安装依赖)
  - [CMake 配置](#cmake-配置)
  - [编译项目](#编译项目)
- [运行程序](#运行程序)
- [快速开始教程](#快速开始教程)
  - [创建第一个项目](#创建第一个项目)
  - [添加几何体](#添加几何体)
  - [编辑网格](#编辑网格)
  - [添加材质](#添加材质)
  - [设置光照](#设置光照)
- [项目结构](#项目结构)
- [常见问题](#常见问题)
- [许可证](#许可证)
- [作者](#作者)

## 简介

V3D Studio 是一款专业的 3D 建模和动画制作软件，集 3D 建模、动画制作、音频处理和 AI 集成于一体。无论是初学者还是专业人士，都能使用 V3D Studio 快速创建精彩的 3D 作品。

## 功能特性

### 3D建模
- 基础几何体创建（立方体、球体、圆柱体、圆锥体、圆环、平面等）
- 高级网格编辑（顶点/边/面操作、挤出、内插、倒角等）
- 细分曲面建模
- UV编辑器

### 材质与纹理
- 材质管理系统
- PBR材质支持
- 纹理导入与管理

### 光照系统
- 定向光、点光源、聚光灯
- 阴影配置

### 动画系统
- 骨骼动画
- 关键帧编辑
- 动画状态机
- IK解算器

### 音频系统
- 音频引擎
- 音频效果器
- 波形编辑器
- 格式转换与压缩

### 项目管理
- 新建、打开、保存项目
- 自动保存
- 版本控制
- 回溯管理与预览

### AI集成
- AI插件系统
- OpenAI、Claude、Stable Diffusion集成
- AI助手面板
- 模型与纹理生成

### 模型库
- 模型分类存储
- 预览与检索
- 快速导入

## 环境准备

### 系统要求

- **操作系统**: Windows 10 (64位) 或更高版本
- **内存**: 至少 8GB RAM（推荐 16GB）
- **硬盘空间**: 至少 5GB 可用空间
- **显卡**: 支持 OpenGL 1.1 的显卡

### 安装 Python 环境

虽然 V3D Studio 主要使用 C++ 开发，但 Python 环境对于运行 AI 插件、自动化脚本等功能非常重要。

#### 步骤 1：下载 Python

访问 [Python 官方网站](https://www.python.org/downloads/) 下载 Windows 版本的 Python 安装包。

推荐版本：**Python 3.10 或 3.11**

#### 步骤 2：安装 Python

1. 运行下载的安装程序
2. **重要**：勾选 "Add Python to PATH" 选项
3. 点击 "Install Now" 完成安装

#### 步骤 3：验证安装

打开命令提示符（CMD）或 PowerShell，输入：

```bash
python --version
```

如果显示 Python 版本号，说明安装成功。

#### 步骤 4：创建虚拟环境（推荐）

```bash
python -m venv venv
```

激活虚拟环境：

```bash
venv\Scripts\activate
```

### 安装 C++ 编译器

#### 方法 1：Visual Studio（推荐）

1. 访问 [Visual Studio 下载页面](https://visualstudio.microsoft.com/downloads/)
2. 下载 **Visual Studio Community 2022**（免费版）
3. 运行安装程序，勾选 "使用 C++ 的桌面开发" 工作负载
4. 确保已选择以下组件：
   - MSVC v143 - VS 2022 C++ x64/x86 生成工具
   - Windows 10 SDK（或 Windows 11 SDK）
5. 点击安装

#### 方法 2：MinGW-w64

1. 下载 [MinGW-w64](https://www.mingw-w64.org/downloads/)
2. 解压到指定目录
3. 将 bin 目录添加到系统 PATH 环境变量

### 安装 CMake

#### 方法 1：官方安装包

1. 访问 [CMake 官方网站](https://cmake.org/download/)
2. 下载 Windows x64 Installer
3. 运行安装程序，选择 "Add CMake to the system PATH"
4. 完成安装

#### 方法 2：使用 Chocolatey

如果你已安装 Chocolatey 包管理器：

```bash
choco install cmake
```

#### 验证安装

```bash
cmake --version
```

### 安装依赖库

我们使用 vcpkg 来管理 C++ 依赖库。

#### 步骤 1：安装 vcpkg

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

#### 步骤 2：集成 vcpkg 到 CMake

```bash
.\vcpkg integrate install
```

#### 步骤 3：安装依赖库

```bash
.\vcpkg install glm:x64-windows
.\vcpkg install nlohmann-json:x64-windows
```

## 构建项目

### 克隆代码

如果你还没有代码，请先克隆项目：

```bash
git clone <你的仓库地址>
cd 3d
```

### 使用 vcpkg 安装依赖

如果之前没有安装依赖，在项目目录下运行：

```bash
# 假设 vcpkg 安装在 C:\vcpkg
set VCPKG_ROOT=C:\vcpkg
```

### CMake 配置

在项目根目录下创建 build 文件夹并配置：

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
```

或者使用项目提供的脚本：

```bash
.\check_env.bat
```

### 编译项目

#### 使用 CMake 命令

```bash
cmake --build . --config Release
```

#### 使用 Visual Studio

1. 在 build 目录下找到 `v3d_studio.sln`
2. 用 Visual Studio 打开
3. 选择 Release 配置
4. 按 F7 或点击 "生成" -> "生成解决方案"

#### 使用批处理脚本

```bash
..\build_and_test.bat
```

## 运行程序

编译成功后，可执行文件位于：

```
build\bin\Release\v3d_studio.exe
```

直接双击运行或在命令行中执行：

```bash
.\build\bin\Release\v3d_studio.exe
```

## 快速开始教程

### 创建第一个项目

1. 启动 V3D Studio
2. 点击菜单栏 "文件" -> "新建项目"
3. 输入项目名称，选择保存位置
4. 点击 "创建"

### 添加几何体

1. 在右侧 "建模工具" 面板中
2. 点击 "立方体" 按钮
3. 在 3D 视图中点击并拖动来创建立方体
4. 可以在属性面板中调整立方体的尺寸

### 编辑网格

1. 点击顶部工具栏的 "编辑模式" 按钮
2. 在 3D 视图中选择顶点、边或面
3. 使用右侧面板的工具进行操作：
   - 挤出：拉伸选中的面
   - 内插：在选中的面内部创建新面
   - 倒角：给选中的边添加圆角

### 添加材质

1. 打开 "材质" 面板
2. 点击 "新建材质"
3. 设置材质的颜色、粗糙度、金属度等属性
4. 将材质拖拽到 3D 视图中的模型上

### 设置光照

1. 打开 "光照" 面板
2. 点击 "添加定向光"
3. 调整光源的方向和强度
4. 可以添加多个光源来创建复杂的光照效果

## 项目结构

```
v3d_studio/
├── app/                      # 主应用程序
│   ├── ai/                   # AI插件系统
│   │   └── ai_plugin_system.h
│   ├── core/                 # 核心功能
│   │   ├── project_manager.h
│   │   └── audio_converter.h
│   ├── modeling/             # 建模模块
│   │   ├── geometry_factory.h
│   │   ├── mesh_editor.h
│   │   ├── material_manager.h
│   │   ├── lighting_system.h
│   │   └── model_library.h
│   ├── panels/               # UI面板
│   │   ├── panel_3d_view.h
│   │   ├── panel_timeline.h
│   │   ├── panel_properties.h
│   │   ├── panel_asset_browser.h
│   │   ├── panel_modeling_tools.h
│   │   └── panel_ai_assistant.h
│   └── ui/                   # UI框架
│       ├── main_window.h
│       └── gl_renderer.h
├── src/                      # 核心库
│   ├── animation/            # 动画系统
│   ├── audio/                # 音频系统
│   ├── concurrency/          # 并发处理
│   ├── core/                 # 核心工具
│   ├── io/                   # 输入输出
│   └── modeling/             # 建模库
├── tests/                    # 测试代码
├── build/                    # 构建输出目录
├── CMakeLists.txt            # CMake 构建配置
├── README.md                 # 中文说明文档
├── README_EN.md              # 英文说明文档
├── LICENSE                   # 中文许可证
├── LICENSE_EN                # 英文许可证
└── build_and_test.bat        # 构建脚本
```

## 常见问题

### Q: 编译时提示找不到 glm 或 nlohmann_json？

A: 确保已使用 vcpkg 安装了依赖库，并且正确设置了 CMAKE_TOOLCHAIN_FILE。

### Q: 运行程序时提示缺少 DLL？

A: 将 vcpkg 的 installed\x64-windows\bin 目录添加到 PATH 环境变量中，或者将需要的 DLL 复制到可执行文件同级目录。

### Q: 如何启用 AI 功能？

A: 在 AI 助手面板中配置你的 API Key（OpenAI、Claude 或 Stable Diffusion），然后就可以使用 AI 生成功能了。

### Q: 支持导入哪些 3D 模型格式？

A: 目前支持 OBJ 和 FBX 格式，未来会添加更多格式支持。

### Q: 如何提交问题或建议？

A: 请在 GitHub 仓库提交 Issue，我们会尽快回复。

## 许可证

MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 作者

chinaling1
