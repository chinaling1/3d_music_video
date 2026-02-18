# V3D Studio

A powerful 3D modeling and animation software.

---

**中文版本** | [Chinese version](README.md)

## Table of Contents

- [Introduction](#introduction)
- [Quick Start](#quick-start)
- [Features](#features)
- [Environment Setup](#environment-setup)
  - [System Requirements](#system-requirements)
  - [Install Python Environment](#install-python-environment)
  - [Install C++ Compiler](#install-c-compiler)
  - [Install CMake](#install-cmake)
  - [Install Dependencies](#install-dependencies)
- [Building the Project](#building-the-project)
  - [Clone the Code](#clone-the-code)
  - [Install Dependencies with vcpkg](#install-dependencies-with-vcpkg)
  - [CMake Configuration](#cmake-configuration)
  - [Compile the Project](#compile-the-project)
- [Running the Program](#running-the-program)
- [Quick Start Tutorial](#quick-start-tutorial)
  - [Create Your First Project](#create-your-first-project)
  - [Add Geometry](#add-geometry)
  - [Edit Mesh](#edit-mesh)
  - [Add Material](#add-material)
  - [Set Up Lighting](#set-up-lighting)
- [Project Structure](#project-structure)
- [FAQ](#faq)
- [License](#license)
- [Author](#author)

## Introduction

V3D Studio is a professional 3D modeling and animation software that integrates 3D modeling, animation, audio processing, and AI integration. Whether you're a beginner or a professional, you can use V3D Studio to quickly create amazing 3D works.

## Quick Start

**No compilation needed - run directly!** The project already includes pre-compiled executable files:

| Version | Path | Size | Description |
|---------|------|------|-------------|
| **Release** | `build\bin\Release\v3d_studio.exe` | 86 KB | Recommended, optimized version |
| **Debug** | `build\bin\Debug\v3d_studio.exe` | - | With debug info |

Simply double-click `build\bin\Release\v3d_studio.exe` to launch the program!

## Features

### 3D Modeling
- Basic geometry creation (cube, sphere, cylinder, cone, torus, plane, etc.)
- Advanced mesh editing (vertex/edge/face operations, extrude, inset, bevel, etc.)
- Subdivision surface modeling
- UV editor

### Materials and Textures
- Material management system
- PBR material support
- Texture import and management

### Lighting System
- Directional light, point light, spot light
- Shadow configuration

### Animation System
- Skeletal animation
- Keyframe editing
- Animation state machine
- IK solver

### Audio System
- Audio engine
- Audio effects
- Waveform editor
- Format conversion and compression

### Project Management
- New, open, save project
- Auto-save
- Version control
- History management with preview

### AI Integration
- AI plugin system
- OpenAI, Claude, Stable Diffusion integration
- AI assistant panel
- Model and texture generation

### Model Library
- Model classification storage
- Preview and search
- Quick import

## Environment Setup

### System Requirements

- **OS**: Windows 10 (64-bit) or higher
- **Memory**: At least 8GB RAM (16GB recommended)
- **Disk Space**: At least 5GB free space
- **GPU**: Graphics card supporting OpenGL 1.1

### Install Python Environment

Although V3D Studio is primarily developed in C++, a Python environment is essential for running AI plugins, automation scripts, and other features.

#### Step 1: Download Python

Visit the [Python official website](https://www.python.org/downloads/) to download the Windows version of the Python installer.

Recommended version: **Python 3.10 or 3.11**

#### Step 2: Install Python

1. Run the downloaded installer
2. **Important**: Check the "Add Python to PATH" option
3. Click "Install Now" to complete installation

#### Step 3: Verify Installation

Open Command Prompt (CMD) or PowerShell, and enter:

```bash
python --version
```

If the Python version number is displayed, the installation was successful.

#### Step 4: Create a Virtual Environment (Recommended)

```bash
python -m venv venv
```

Activate the virtual environment:

```bash
venv\Scripts\activate
```

### Install C++ Compiler

#### Method 1: Visual Studio (Recommended)

1. Visit the [Visual Studio download page](https://visualstudio.microsoft.com/downloads/)
2. Download **Visual Studio Community 2022** (free version)
3. Run the installer and check the "Desktop development with C++" workload
4. Ensure the following components are selected:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - Windows 10 SDK (or Windows 11 SDK)
5. Click Install

#### Method 2: MinGW-w64

1. Download [MinGW-w64](https://www.mingw-w64.org/downloads/)
2. Extract to a specified directory
3. Add the bin directory to your system PATH environment variable

### Install CMake

#### Method 1: Official Installer

1. Visit the [CMake official website](https://cmake.org/download/)
2. Download the Windows x64 Installer
3. Run the installer and select "Add CMake to the system PATH"
4. Complete the installation

#### Method 2: Using Chocolatey

If you have Chocolatey package manager installed:

```bash
choco install cmake
```

#### Verify Installation

```bash
cmake --version
```

### Install Dependencies

We use vcpkg to manage C++ dependencies.

#### Step 1: Install vcpkg

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

#### Step 2: Integrate vcpkg with CMake

```bash
.\vcpkg integrate install
```

#### Step 3: Install Dependencies

```bash
.\vcpkg install glm:x64-windows
.\vcpkg install nlohmann-json:x64-windows
```

## Building the Project

### Clone the Code

If you don't have the code yet, clone the project first:

```bash
git clone <your repository address>
cd 3d
```

### Install Dependencies with vcpkg

If you haven't installed dependencies before, run in the project directory:

```bash
# Assuming vcpkg is installed at C:\vcpkg
set VCPKG_ROOT=C:\vcpkg
```

### CMake Configuration

Create a build folder in the project root directory and configure:

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
```

Or use the provided script:

```bash
.\scripts\check_env.bat
```

### Compile the Project

#### Using CMake Commands

```bash
cmake --build . --config Release
```

#### Using Visual Studio

1. Find `v3d_studio.sln` in the build directory
2. Open it with Visual Studio
3. Select Release configuration
4. Press F7 or click "Build" -> "Build Solution"

#### Using Batch Script

```bash
..\scripts\build_and_test.bat
```

## Running the Program

After successful compilation, the executable file is located at:

```
build\bin\Release\v3d_studio.exe
```

Double-click to run directly or execute from the command line:

```bash
.\build\bin\Release\v3d_studio.exe
```

## Quick Start Tutorial

### Create Your First Project

1. Launch V3D Studio
2. Click menu "File" -> "New Project"
3. Enter project name, select save location
4. Click "Create"

### Add Geometry

1. In the right "Modeling Tools" panel
2. Click the "Cube" button
3. Click and drag in the 3D view to create a cube
4. Adjust the cube's dimensions in the properties panel

### Edit Mesh

1. Click the "Edit Mode" button in the top toolbar
2. Select vertices, edges, or faces in the 3D view
3. Use tools from the right panel for operations:
   - Extrude: Stretch selected faces
   - Inset: Create new faces inside selected faces
   - Bevel: Add rounded corners to selected edges

### Add Material

1. Open the "Materials" panel
2. Click "New Material"
3. Set material properties like color, roughness, metallic, etc.
4. Drag and drop the material onto the model in the 3D view

### Set Up Lighting

1. Open the "Lighting" panel
2. Click "Add Directional Light"
3. Adjust the light direction and intensity
4. You can add multiple lights to create complex lighting effects

## Project Structure

```
v3d_studio/
├── app/                      # Main application
│   ├── ai/                   # AI plugin system
│   │   └── ai_plugin_system.h
│   ├── core/                 # Core features
│   │   ├── project_manager.h
│   │   └── audio_converter.h
│   ├── modeling/             # Modeling module
│   │   ├── geometry_factory.h
│   │   ├── mesh_editor.h
│   │   ├── material_manager.h
│   │   ├── lighting_system.h
│   │   └── model_library.h
│   ├── panels/               # UI panels
│   │   ├── panel_3d_view.h
│   │   ├── panel_timeline.h
│   │   ├── panel_properties.h
│   │   ├── panel_asset_browser.h
│   │   ├── panel_modeling_tools.h
│   │   └── panel_ai_assistant.h
│   └── ui/                   # UI framework
│       ├── main_window.h
│       └── gl_renderer.h
├── src/                      # Core libraries
│   ├── animation/            # Animation system
│   ├── audio/                # Audio system
│   ├── concurrency/          # Concurrency processing
│   ├── core/                 # Core utilities
│   ├── io/                   # Input/output
│   └── modeling/             # Modeling library
├── tests/                    # Test code
├── scripts/                  # Script files
│   ├── build_and_test.bat    # Build script
│   ├── check_env.bat         # Environment check script
│   ├── push_to_gitee.bat     # Push to Gitee
│   ├── push_to_github.bat    # Push to GitHub
│   └── ...
├── build/                    # Build output directory
├── CMakeLists.txt            # CMake build configuration
├── package.json              # npm build scripts
├── README.md                 # Chinese documentation
├── README_EN.md              # English documentation
├── LICENSE                   # Chinese license
└── LICENSE_EN                # English license
```

## FAQ

### Q: Compilation error: cannot find glm or nlohmann_json?

A: Make sure you've installed the dependencies using vcpkg and correctly set CMAKE_TOOLCHAIN_FILE.

### Q: Program missing DLL when running?

A: Add vcpkg's installed\x64-windows\bin directory to your PATH environment variable, or copy the required DLLs to the same directory as the executable.

### Q: How to enable AI features?

A: Configure your API Key (OpenAI, Claude, or Stable Diffusion) in the AI Assistant panel, then you can use the AI generation features.

### Q: What 3D model formats are supported for import?

A: Currently OBJ and FBX formats are supported. More formats will be added in the future.

### Q: How to submit issues or suggestions?

A: Please submit an Issue in the GitHub repository, we will respond as soon as possible.

## License

MIT License - See [LICENSE_EN](LICENSE_EN) file

## Author

chinaling1
