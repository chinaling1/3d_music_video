# 3D Video Studio - 使用文档

## 目录

1. [项目概述](#1-项目概述)
2. [环境配置要求](#2-环境配置要求)
3. [安装步骤](#3-安装步骤)
4. [基本功能说明](#4-基本功能说明)
5. [详细使用方法](#5-详细使用方法)
6. [常见问题解答](#6-常见问题解答)
7. [注意事项](#7-注意事项)

---

## 1. 项目概述

### 1.1 项目简介

3D Video Studio 是一个功能强大的 3D 视频制作工作室软件，提供完整的 3D 建模、动画、音频处理和文件管理功能。该项目采用模块化设计，各模块可独立使用或组合使用。

### 1.2 核心功能

| 模块 | 功能描述 |
|------|---------|
| **Core** | 核心功能：版本控制、日志系统、事件系统、内存管理、对象管理 |
| **Concurrency** | 并发处理：线程池、任务调度、并行算法 |
| **Animation** | 动画系统：骨骼动画、IK求解、动画状态机、曲线编辑 |
| **Audio** | 音频处理：音频引擎、效果处理、MIDI处理、人声合成 |
| **Modeling** | 3D建模：网格操作、UV编辑、材质管理、细分曲面 |
| **IO** | 文件处理：文件加载/保存、格式转换、资产管理 |

### 1.3 技术特点

- **C++20** 标准
- **模块化设计**：各模块独立编译，可单独使用
- **线程安全**：所有公共API都是线程安全的
- **跨平台**：支持 Windows、Linux、macOS
- **高性能**：使用内存池、对象池等优化技术

---

## 2. 环境配置要求

### 2.1 操作系统

| 操作系统 | 最低版本 | 推荐版本 |
|---------|---------|---------|
| Windows | Windows 10 | Windows 11 |
| Linux | Ubuntu 20.04 | Ubuntu 22.04 |
| macOS | macOS 11.0 | macOS 14.0 |

### 2.2 编译器要求

| 编译器 | 最低版本 | 推荐版本 |
|--------|---------|---------|
| MSVC | Visual Studio 2022 | Visual Studio 2022 17.8+ |
| GCC | 11.0 | 13.0+ |
| Clang | 14.0 | 16.0+ |

### 2.3 依赖库

| 依赖库 | 版本 | 用途 |
|--------|------|------|
| CMake | 3.20+ | 构建系统 |
| vcpkg | 最新 | 包管理器 |
| GLM | 0.9.9+ | 数学库 |
| assimp | 5.0+ | 3D模型导入导出 |
| nlohmann_json | 3.11+ | JSON处理 |
| GTest | 1.11+ | 单元测试 |
| zlib | 1.2+ | 压缩 |

### 2.4 硬件要求

| 组件 | 最低配置 | 推荐配置 |
|------|---------|---------|
| CPU | 4核心 | 8核心+ |
| 内存 | 8GB | 16GB+ |
| 硬盘 | 10GB可用空间 | SSD 20GB+ |
| GPU | OpenGL 4.5支持 | 独立显卡 |

---

## 3. 安装步骤

### 3.1 安装 vcpkg

```bash
# 克隆 vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg

# 运行引导脚本
cd C:\vcpkg
.\bootstrap-vcpkg.bat

# 设置环境变量（可选）
setx VCPKG_ROOT "C:\vcpkg"
```

### 3.2 安装依赖

```bash
# 安装所需库
vcpkg install glm:x64-windows
vcpkg install assimp:x64-windows
vcpkg install nlohmann-json:x64-windows
vcpkg install gtest:x64-windows
vcpkg install zlib:x64-windows
```

### 3.3 克隆项目

```bash
git clone https://github.com/your-org/3d-video-studio.git
cd 3d-video-studio
```

### 3.4 构建项目

#### Windows (Visual Studio)

```bash
# 配置 CMake
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

# 构建
cmake --build build --config Debug

# 或使用提供的脚本
.\build_and_test.bat
```

#### Linux/macOS

```bash
# 配置 CMake
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# 构建
cmake --build build --config Debug

# 或使用提供的脚本
./build_and_test.sh
```

### 3.5 运行测试

```bash
cd build
ctest --output-on-failure
```

---

## 4. 基本功能说明

### 4.1 核心模块 (Core)

#### 4.1.1 版本控制系统

版本控制系统提供完整的撤销/重做功能：

```cpp
#include "core/version_control.h"

using namespace v3d::core;

// 自定义命令
class MoveCommand : public Command {
    Object* obj_;
    glm::vec3 oldPos_, newPos_;
public:
    MoveCommand(Object* obj, glm::vec3 newPos) 
        : obj_(obj), newPos_(newPos), oldPos_(obj->getPosition()) {}
    
    void execute() override { obj_->setPosition(newPos_); }
    void undo() override { obj_->setPosition(oldPos_); }
    void redo() override { execute(); }
    std::string getDescription() const override { return "Move Object"; }
    size_t getMemorySize() const override { return sizeof(*this); }
    Command* clone() const override { return new MoveCommand(*this); }
};

// 使用
auto& cmdMgr = CommandManager::getInstance();
cmdMgr.executeCommand(std::make_unique<MoveCommand>(obj, newPos));

// 撤销
if (cmdMgr.canUndo()) {
    cmdMgr.undo();
}

// 重做
if (cmdMgr.canRedo()) {
    cmdMgr.redo();
}
```

#### 4.1.2 日志系统

```cpp
#include "core/logger.h"

using namespace v3d::core;

// 获取日志记录器
auto logger = LoggerManager::getInstance().getOrCreateLogger("MyApp");

// 配置输出目标
logger->addAppender(std::make_shared<ConsoleAppender>());
logger->addAppender(std::make_shared<FileAppender>("app.log"));
logger->addAppender(std::make_shared<RollingFileAppender>("app.log", 10*1024*1024, 5));

// 设置日志级别
logger->setLevel(LogLevel::Debug);

// 记录日志
logger->info("Application started");
logger->debug("Loading file: {}", filename);
logger->warning("Memory usage high: {} MB", memUsage);
logger->error("Failed to load resource: {}", resourceId);

// 使用便捷宏
LOG_INFO(logger, "Processing item " << itemId);
```

#### 4.1.3 事件系统

```cpp
#include "core/event_system.h"

using namespace v3d::core;

// 定义自定义事件
class PlayerEvent : public Event {
public:
    int playerId;
    std::string action;
    
    std::string getType() const override { return "PlayerEvent"; }
    size_t getSize() const override { return sizeof(PlayerEvent); }
};

// 使用 EventBus
auto& bus = EventBus::getInstance();

// 订阅事件
bus.subscribe("game_events", [](const Event& e) {
    const PlayerEvent& pe = static_cast<const PlayerEvent&>(e);
    std::cout << "Player " << pe.playerId << " " << pe.action << std::endl;
});

// 发布事件
PlayerEvent event{1, "jumped"};
bus.publish("game_events", event);

// 使用 EventDispatcher
auto& dispatcher = EventDispatcher::getInstance();

dispatcher.addListener("PlayerEvent", [](const Event& e) {
    // 处理事件
});

dispatcher.dispatch(event);
```

### 4.2 并发模块 (Concurrency)

#### 4.2.1 线程池

```cpp
#include "concurrency/thread_pool.h"

using namespace v3d::concurrency;

// 创建线程池
ThreadPool pool(4);  // 4个工作线程
pool.start();

// 提交任务
auto future = pool.submit([]() {
    // 执行耗时任务
    return computeResult();
});

// 获取结果
auto result = future.get();

// 停止线程池
pool.stop();
```

#### 4.2.2 并行算法

```cpp
#include "concurrency/thread_pool.h"

using namespace v3d::concurrency;

// 并行 for 循环
std::vector<int> data(1000);

parallel_for(0, data.size(), [&data](int i) {
    data[i] = processData(data[i]);
}, &pool);

// 等待所有任务完成
pool.waitAll();
```

### 4.3 动画模块 (Animation)

#### 4.3.1 骨骼动画

```cpp
#include "animation/skeleton.h"
#include "animation/animation_clip.h"

using namespace v3d::animation;

// 创建骨骼
Skeleton skeleton;
skeleton.loadFromBVH("character.bvh");

// 加载动画
auto clip = std::make_shared<AnimationClip>();
clip->load("walk.anim");

// 播放动画
skeleton.playAnimation(clip);

// 更新
float deltaTime = 0.016f;
skeleton.update(deltaTime);

// 获取骨骼变换
auto transforms = skeleton.getBoneTransforms();
```

#### 4.3.2 IK 求解器

```cpp
#include "animation/ik_solver.h"

using namespace v3d::animation;

// 创建 IK 求解器
IKSolver solver;
solver.setChain(bones);  // 设置骨骼链
solver.setTarget(targetPosition);  // 设置目标位置
solver.setIterations(10);  // 迭代次数

// 求解
solver.solve();

// 获取结果
auto result = solver.getResult();
```

### 4.4 音频模块 (Audio)

#### 4.4.1 音频引擎

```cpp
#include "audio/audio_engine.h"

using namespace v3d::audio;

// 初始化音频引擎
AudioEngine engine;
engine.initialize();

// 加载音频
auto buffer = engine.loadSound("background.wav");

// 播放
auto source = engine.createSource();
source.setBuffer(buffer);
source.play();

// 设置音量和位置
source.setVolume(0.8f);
source.setPosition(glm::vec3(0, 0, 0));

// 更新
engine.update();
```

#### 4.4.2 音频效果

```cpp
#include "audio/audio_effect.h"

using namespace v3d::audio;

// 创建效果链
EffectChain chain;

// 添加效果
chain.addEffect(std::make_unique<ReverbEffect>());
chain.addEffect(std::make_unique<EchoEffect>(0.5f, 0.3f));

// 应用效果
buffer = chain.process(buffer);
```

### 4.5 建模模块 (Modeling)

#### 4.5.1 网格操作

```cpp
#include "modeling/mesh.h"

using namespace v3d::modeling;

// 创建网格
Mesh mesh;

// 添加顶点
auto v1 = mesh.addVertex(glm::vec3(0, 0, 0));
auto v2 = mesh.addVertex(glm::vec3(1, 0, 0));
auto v3 = mesh.addVertex(glm::vec3(0, 1, 0));

// 添加面
mesh.addFace({v1, v2, v3});

// 计算法线
mesh.calculateNormals();

// 细分
mesh.subdivide(SubdivisionType::CatmullClark, 2);

// 导出
mesh.save("output.obj");
```

#### 4.5.2 材质系统

```cpp
#include "modeling/material.h"

using namespace v3d::modeling;

// 创建材质
Material material;
material.setDiffuseColor(glm::vec3(0.8f, 0.2f, 0.2f));
material.setSpecularColor(glm::vec3(1.0f));
material.setRoughness(0.5f);
material.setMetallic(0.0f);

// 加载纹理
material.setDiffuseTexture("diffuse.png");
material.setNormalTexture("normal.png");

// 应用到网格
mesh.setMaterial(material);
```

### 4.6 IO模块 (IO)

#### 4.6.1 文件加载

```cpp
#include "io/file_loader.h"

using namespace v3d::io;

// 同步加载
auto data = FileLoader::loadBinary("model.bin");

// 异步加载
AsyncFileLoader loader;
loader.loadFile("model.bin", [](const std::vector<uint8_t>& data) {
    // 处理加载完成的数据
});
```

#### 4.6.2 资产管理

```cpp
#include "io/asset_manager.h"

using namespace v3d::io;

auto& manager = AssetManager::getInstance();

// 加载资产
auto mesh = manager.loadAsset<MeshAsset>("character.fbx");
auto audio = manager.loadAsset<AudioAsset>("sound.wav");

// 获取资产
auto loadedMesh = manager.getAsset<MeshAsset>("character.fbx");

// 卸载资产
manager.unloadAsset("character.fbx");
```

---

## 5. 详细使用方法

### 5.1 创建自定义命令

```cpp
#include "core/version_control.h"

class TransformCommand : public Command {
private:
    Object* object_;
    glm::mat4 oldTransform_;
    glm::mat4 newTransform_;
    std::string description_;

public:
    TransformCommand(Object* obj, const glm::mat4& newTransform)
        : object_(obj)
        , newTransform_(newTransform)
        , oldTransform_(obj->getTransform())
        , description_("Transform Object")
    {}

    void execute() override {
        object_->setTransform(newTransform_);
    }

    void undo() override {
        object_->setTransform(oldTransform_);
    }

    void redo() override {
        execute();
    }

    std::string getDescription() const override {
        return description_;
    }

    size_t getMemorySize() const override {
        return sizeof(TransformCommand);
    }

    Command* clone() const override {
        return new TransformCommand(*this);
    }

    // 可选：支持命令合并
    bool isMergeableWith(const Command* other) const override {
        auto transform = dynamic_cast<const TransformCommand*>(other);
        return transform != nullptr && transform->object_ == object_;
    }

    void mergeWith(Command* other) override {
        auto transform = static_cast<TransformCommand*>(other);
        newTransform_ = transform->newTransform_;
    }
};
```

### 5.2 创建自定义日志输出器

```cpp
#include "core/logger.h"

class NetworkAppender : public LogAppender {
private:
    std::string serverUrl_;
    std::mutex mutex_;

public:
    explicit NetworkAppender(const std::string& url) : serverUrl_(url) {}

    void append(const LogMessage& message) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 发送日志到服务器
        nlohmann::json json;
        json["level"] = static_cast<int>(message.getLevel());
        json["message"] = message.getMessage();
        json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            message.getTimestamp().time_since_epoch()).count();
        
        sendToServer(serverUrl_, json.dump());
    }

    void flush() override {
        // 刷新网络缓冲区
    }
};
```

### 5.3 创建自定义事件

```cpp
#include "core/event_system.h"

// 自定义事件类
class LevelLoadedEvent : public Event {
public:
    std::string levelName;
    int playerCount;
    float loadTime;

    LevelLoadedEvent(const std::string& name, int count, float time)
        : levelName(name), playerCount(count), loadTime(time) {}

    std::string getType() const override { return "LevelLoadedEvent"; }
    size_t getSize() const override { return sizeof(LevelLoadedEvent); }
};

// 使用
auto& bus = EventBus::getInstance();

bus.subscribe("game", [](const Event& e) {
    if (e.getType() == "LevelLoadedEvent") {
        const LevelLoadedEvent& event = static_cast<const LevelLoadedEvent&>(e);
        std::cout << "Level " << event.levelName << " loaded in " 
                  << event.loadTime << " seconds" << std::endl;
    }
});

LevelLoadedEvent event("Level1", 4, 2.5f);
bus.publish("game", event);
```

### 5.4 创建动画状态机

```cpp
#include "animation/animation_state_machine.h"

using namespace v3d::animation;

// 创建状态机
AnimationStateMachine stateMachine;

// 添加状态
auto idleState = stateMachine.addState("Idle");
idleState->setAnimation(idleClip);

auto walkState = stateMachine.addState("Walk");
walkState->setAnimation(walkClip);

auto runState = stateMachine.addState("Run");
runState->setAnimation(runClip);

// 添加过渡
stateMachine.addTransition("Idle", "Walk", [](const AnimationContext& ctx) {
    return ctx.speed > 0.1f;
});

stateMachine.addTransition("Walk", "Run", [](const AnimationContext& ctx) {
    return ctx.speed > 5.0f;
});

stateMachine.addTransition("Run", "Walk", [](const AnimationContext& ctx) {
    return ctx.speed <= 5.0f;
});

stateMachine.addTransition("Walk", "Idle", [](const AnimationContext& ctx) {
    return ctx.speed <= 0.1f;
});

// 更新
AnimationContext ctx;
ctx.speed = playerSpeed;
stateMachine.update(ctx, deltaTime);
```

### 5.5 创建音频效果链

```cpp
#include "audio/audio_effect.h"
#include "audio/audio_buffer.h"

using namespace v3d::audio;

// 创建效果链
EffectChain chain;

// 添加均衡器
auto eq = std::make_unique<EqualizerEffect>();
eq->setBand(0, 100.0f, -3.0f);   // 低频
eq->setBand(1, 1000.0f, 0.0f);  // 中频
eq->setBand(2, 10000.0f, 2.0f); // 高频
chain.addEffect(std::move(eq));

// 添加压缩器
auto compressor = std::make_unique<CompressorEffect>();
compressor->setThreshold(-20.0f);
compressor->setRatio(4.0f);
compressor->setAttack(10.0f);
compressor->setRelease(100.0f);
chain.addEffect(std::move(compressor));

// 添加混响
auto reverb = std::make_unique<ReverbEffect>();
reverb->setRoomSize(0.7f);
reverb->setDamping(0.5f);
reverb->setWetLevel(0.3f);
chain.addEffect(std::move(reverb));

// 处理音频
AudioBuffer input = loadAudio("input.wav");
AudioBuffer output = chain.process(input);
output.save("output.wav");
```

---

## 6. 常见问题解答

### Q1: 编译时找不到 GLM 库

**问题**: 编译报错 `Cannot find glm`

**解决方案**:
```bash
# 安装 GLM
vcpkg install glm:x64-windows

# 确保使用 vcpkg 工具链
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Q2: 链接时找不到 assimp 库

**问题**: 链接错误 `LNK1104: cannot open file 'assimp.lib'`

**解决方案**:
```bash
# 安装 assimp
vcpkg install assimp:x64-windows

# 清理并重新构建
rm -rf build
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Debug
```

### Q3: 测试无法运行

**问题**: 运行测试时提示 GTest 相关错误

**解决方案**:
```bash
# 安装 GTest
vcpkg install gtest:x64-windows

# 确保使用 Debug 配置运行测试
cmake --build build --config Debug
cd build
ctest -C Debug --output-on-failure
```

### Q4: 内存泄漏警告

**问题**: 运行时出现内存泄漏警告

**解决方案**:
- 确保使用智能指针管理对象生命周期
- 使用对象池而非频繁创建/销毁对象
- 检查事件订阅是否正确取消

### Q5: 线程安全问题

**问题**: 多线程访问时出现崩溃

**解决方案**:
- 所有公共 API 都是线程安全的
- 回调函数中避免阻塞操作
- 使用 `std::lock_guard` 保护自定义数据

---

## 7. 注意事项

### 7.1 性能优化建议

1. **使用对象池**: 对于频繁创建的对象，使用 `ObjectPool` 减少内存分配开销

```cpp
ObjectPool<Particle> particlePool(1000);
auto particle = particlePool.acquire();
// 使用 particle
particlePool.release(particle);
```

2. **批量操作**: 对于大量数据操作，使用并行算法

```cpp
parallel_for(0, particles.size(), [&](int i) {
    particles[i].update(deltaTime);
}, &threadPool);
```

3. **异步加载**: 对于大型资源，使用异步加载避免阻塞主线程

```cpp
asyncLoader.loadFile("large_model.fbx", [](const std::vector<uint8_t>& data) {
    // 在后台线程处理
    auto mesh = parseMesh(data);
    // 回到主线程
    runOnMainThread([mesh]() {
        scene->addMesh(mesh);
    });
});
```

### 7.2 内存管理

1. **智能指针**: 优先使用 `std::unique_ptr` 和 `std::shared_ptr`
2. **避免循环引用**: 使用 `std::weak_ptr` 打破循环引用
3. **资源释放**: 及时释放不再使用的资源

### 7.3 线程安全

1. **避免死锁**: 使用 `std::lock` 同时锁定多个互斥量
2. **避免竞态条件**: 使用原子操作或互斥量保护共享数据
3. **回调函数**: 确保回调函数是线程安全的

### 7.4 错误处理

1. **使用异常**: 对于严重错误，抛出异常
2. **使用日志**: 记录所有重要操作和错误
3. **使用断言**: 对于内部一致性检查，使用断言

### 7.5 版本兼容性

1. **API 稳定性**: 主版本号变化表示不兼容的 API 变更
2. **数据迁移**: 使用 `VersionedSerializer` 处理数据格式变化
3. **向后兼容**: 保持对旧数据格式的支持

---

## 附录

### A. API 参考

完整的 API 参考文档请参阅各模块的头文件，所有公共 API 都有详细的 Doxygen 注释。

### B. 示例代码

示例代码位于 `examples/` 目录：

- `example_version_control.cpp` - 版本控制示例
- `example_logger.cpp` - 日志系统示例
- `example_animation.cpp` - 动画系统示例
- `example_audio.cpp` - 音频系统示例
- `example_modeling.cpp` - 建模系统示例

### C. 联系方式

- **项目主页**: https://github.com/your-org/3d-video-studio
- **问题反馈**: https://github.com/your-org/3d-video-studio/issues
- **文档**: https://your-org.github.io/3d-video-studio

---

**文档版本**: 1.0  
**最后更新**: 2026-02-17  
**作者**: 3D Video Studio Team
