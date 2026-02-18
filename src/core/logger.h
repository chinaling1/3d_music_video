/**
 * @file logger.h
 * @brief 日志系统 - 提供灵活的日志记录功能
 * 
 * 该模块实现了完整的日志系统，包括：
 * - 多级别日志（Trace, Debug, Info, Warning, Error, Fatal）
 * - 多种输出目标（控制台、文件、滚动文件）
 * - 线程安全
 * - 日志管理器
 * 
 * @author 3D Video Studio Team
 * @version 1.0
 * @date 2026-02-17
 */

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <mutex>
#include <vector>
#include <functional>
#include <chrono>
#include <iomanip>
#include <unordered_map>

namespace v3d {
namespace core {

/**
 * @enum LogLevel
 * @brief 日志级别枚举
 * 
 * 定义了六个日志级别，从低到高：
 * - Trace: 详细的跟踪信息
 * - Debug: 调试信息
 * - Info: 一般信息
 * - Warning: 警告信息
 * - Error: 错误信息
 * - Fatal: 致命错误
 */
enum class LogLevel {
    Trace,    ///< 跟踪级别 - 最详细的日志
    Debug,    ///< 调试级别 - 开发调试信息
    Info,     ///< 信息级别 - 一般运行信息
    Warning,  ///< 警告级别 - 潜在问题
    Error,    ///< 错误级别 - 运行时错误
    Fatal     ///< 致命级别 - 严重错误
};

/**
 * @class LogMessage
 * @brief 日志消息类 - 封装单条日志消息的所有信息
 */
class LogMessage {
public:
    /**
     * @brief 构造日志消息
     * @param level 日志级别
     * @param loggerName 记录器名称
     * @param file 源文件名
     * @param line 源文件行号
     */
    LogMessage(LogLevel level, const std::string& loggerName, const std::string& file, int line);
    
    /**
     * @brief 转换为字符串格式
     * @return 格式化的日志字符串
     * 
     * 格式: [时间戳] [级别] [记录器] 文件:行号 - 消息
     */
    std::string toString() const;
    
    /**
     * @brief 获取日志级别
     * @return 日志级别
     */
    LogLevel getLevel() const;
    
    /**
     * @brief 获取记录器名称
     * @return 记录器名称
     */
    std::string getLoggerName() const;
    
    /**
     * @brief 获取源文件名
     * @return 源文件名
     */
    std::string getFile() const;
    
    /**
     * @brief 获取源文件行号
     * @return 行号
     */
    int getLine() const;
    
    /**
     * @brief 获取时间戳
     * @return 消息创建时间
     */
    std::chrono::system_clock::time_point getTimestamp() const;
    
    /**
     * @brief 获取消息内容
     * @return 消息内容
     */
    std::string getMessage() const;
    
    /**
     * @brief 设置消息内容
     * @param message 消息内容
     */
    void setMessage(const std::string& message);

private:
    LogLevel level_;                                    ///< 日志级别
    std::string loggerName_;                            ///< 记录器名称
    std::string file_;                                  ///< 源文件名
    int line_;                                          ///< 源文件行号
    std::chrono::system_clock::time_point timestamp_;   ///< 时间戳
    std::string message_;                               ///< 消息内容
};

/**
 * @class LogAppender
 * @brief 日志输出目标基类
 * 
 * 抽象基类，定义了日志输出的接口。
 * 派生类可以实现不同的输出目标（控制台、文件等）。
 */
class LogAppender {
public:
    virtual ~LogAppender() = default;
    
    /**
     * @brief 输出日志消息
     * @param message 日志消息
     */
    virtual void append(const LogMessage& message) = 0;
    
    /**
     * @brief 刷新缓冲区
     */
    virtual void flush() = 0;
};

/**
 * @class ConsoleAppender
 * @brief 控制台日志输出
 * 
 * 将日志输出到标准输出，支持彩色显示。
 * 
 * @example
 * @code
 * auto appender = std::make_shared<ConsoleAppender>(true); // 启用颜色
 * logger->addAppender(appender);
 * @endcode
 */
class ConsoleAppender : public LogAppender {
public:
    /**
     * @brief 构造控制台输出器
     * @param useColor 是否使用彩色输出（默认true）
     */
    explicit ConsoleAppender(bool useColor = true);
    
    void append(const LogMessage& message) override;
    void flush() override;

private:
    /**
     * @brief 获取日志级别对应的颜色代码
     * @param level 日志级别
     * @return ANSI颜色代码
     */
    std::string getColorCode(LogLevel level) const;

    bool useColor_;  ///< 是否使用彩色输出
};

/**
 * @class FileAppender
 * @brief 文件日志输出
 * 
 * 将日志输出到指定文件。
 * 
 * @example
 * @code
 * auto appender = std::make_shared<FileAppender>("app.log", true); // 追加模式
 * logger->addAppender(appender);
 * @endcode
 */
class FileAppender : public LogAppender {
public:
    /**
     * @brief 构造文件输出器
     * @param filePath 日志文件路径
     * @param append 是否追加模式（默认false，覆盖模式）
     */
    explicit FileAppender(const std::string& filePath, bool append = false);
    ~FileAppender();
    
    void append(const LogMessage& message) override;
    void flush() override;
    
    /**
     * @brief 设置文件路径
     * @param filePath 新的文件路径
     */
    void setFilePath(const std::string& filePath);

private:
    std::ofstream file_;    ///< 文件输出流
    std::string filePath_;  ///< 文件路径
    std::mutex mutex_;      ///< 线程安全互斥锁
};

/**
 * @class RollingFileAppender
 * @brief 滚动文件日志输出
 * 
 * 支持文件大小限制和滚动备份的文件输出器。
 * 当日志文件达到指定大小时，自动创建新文件并备份旧文件。
 * 
 * @example
 * @code
 * // 最大10MB，保留5个备份文件
 * auto appender = std::make_shared<RollingFileAppender>("app.log", 10*1024*1024, 5);
 * logger->addAppender(appender);
 * @endcode
 */
class RollingFileAppender : public LogAppender {
public:
    /**
     * @brief 构造滚动文件输出器
     * @param filePath 日志文件路径
     * @param maxFileSize 单个文件最大大小（默认10MB）
     * @param maxFiles 最大备份文件数（默认5个）
     */
    RollingFileAppender(const std::string& filePath, size_t maxFileSize = 10 * 1024 * 1024, int maxFiles = 5);
    ~RollingFileAppender();
    
    void append(const LogMessage& message) override;
    void flush() override;
    
    /**
     * @brief 设置单个文件最大大小
     * @param maxSize 最大大小（字节）
     */
    void setMaxFileSize(size_t maxSize);
    
    /**
     * @brief 设置最大备份文件数
     * @param maxFiles 最大文件数
     */
    void setMaxFiles(int maxFiles);

private:
    /**
     * @brief 滚动文件
     * 
     * 关闭当前文件，重命名备份，创建新文件。
     */
    void rollFile();
    
    /**
     * @brief 获取备份文件名
     * @param index 备份索引
     * @return 备份文件名
     */
    std::string getRollFileName(int index) const;

    std::ofstream file_;      ///< 文件输出流
    std::string filePath_;    ///< 文件路径
    size_t maxFileSize_;      ///< 最大文件大小
    int maxFiles_;            ///< 最大备份文件数
    size_t currentSize_;      ///< 当前文件大小
    std::mutex mutex_;        ///< 线程安全互斥锁
};

/**
 * @class Logger
 * @brief 日志记录器
 * 
 * 主要的日志记录类，支持多级别日志和多输出目标。
 * 线程安全。
 * 
 * @example
 * @code
 * auto logger = LoggerManager::getInstance().getOrCreateLogger("MyApp");
 * logger->setLevel(LogLevel::Debug);
 * logger->addAppender(std::make_shared<ConsoleAppender>());
 * 
 * logger->info("Application started");
 * logger->error("Failed to load file: {}", filename);
 * @endcode
 */
class Logger {
public:
    /**
     * @brief 构造日志记录器
     * @param name 记录器名称
     */
    explicit Logger(const std::string& name);
    ~Logger();
    
    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param message 日志消息
     * @param file 源文件名（可选）
     * @param line 源文件行号（可选）
     */
    void log(LogLevel level, const std::string& message, const char* file, int line);
    
    /**
     * @brief 记录Trace级别日志
     */
    void trace(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief 记录Debug级别日志
     */
    void debug(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief 记录Info级别日志
     */
    void info(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief 记录Warning级别日志
     */
    void warning(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief 记录Error级别日志
     */
    void error(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief 记录Fatal级别日志
     */
    void fatal(const std::string& message, const char* file = nullptr, int line = 0);
    
    /**
     * @brief 添加输出目标
     * @param appender 输出目标
     */
    void addAppender(std::shared_ptr<LogAppender> appender);
    
    /**
     * @brief 移除输出目标
     * @param appender 输出目标
     */
    void removeAppender(std::shared_ptr<LogAppender> appender);
    
    /**
     * @brief 清除所有输出目标
     */
    void clearAppenders();
    
    /**
     * @brief 设置日志级别
     * @param level 日志级别
     * 
     * 低于此级别的日志将被忽略。
     */
    void setLevel(LogLevel level);
    
    /**
     * @brief 获取日志级别
     * @return 当前日志级别
     */
    LogLevel getLevel() const;
    
    /**
     * @brief 获取记录器名称
     * @return 记录器名称
     */
    std::string getName() const;
    
    /**
     * @brief 刷新所有输出目标
     */
    void flush();

private:
    std::string name_;                                  ///< 记录器名称
    LogLevel level_;                                    ///< 日志级别
    std::vector<std::shared_ptr<LogAppender>> appenders_; ///< 输出目标列表
    std::mutex mutex_;                                  ///< 线程安全互斥锁
};

/**
 * @class LoggerManager
 * @brief 日志管理器
 * 
 * 单例模式，管理所有日志记录器和全局配置。
 * 
 * @example
 * @code
 * auto& manager = LoggerManager::getInstance();
 * 
 * // 添加全局输出目标
 * manager.addGlobalAppender(std::make_shared<ConsoleAppender>());
 * 
 * // 获取或创建记录器
 * auto logger = manager.getOrCreateLogger("MyModule");
 * 
 * // 设置默认级别
 * manager.setDefaultLevel(LogLevel::Debug);
 * @endcode
 */
class LoggerManager {
public:
    /**
     * @brief 获取单例实例
     * @return LoggerManager的单例引用
     */
    static LoggerManager& getInstance();
    
    /**
     * @brief 获取日志记录器
     * @param name 记录器名称
     * @return 记录器（如果不存在返回nullptr）
     */
    std::shared_ptr<Logger> getLogger(const std::string& name);
    
    /**
     * @brief 获取或创建日志记录器
     * @param name 记录器名称
     * @return 记录器
     */
    std::shared_ptr<Logger> getOrCreateLogger(const std::string& name);
    
    /**
     * @brief 设置默认日志级别
     * @param level 日志级别
     */
    void setDefaultLevel(LogLevel level);
    
    /**
     * @brief 获取默认日志级别
     * @return 默认日志级别
     */
    LogLevel getDefaultLevel() const;
    
    /**
     * @brief 添加全局输出目标
     * @param appender 输出目标
     * 
     * 全局输出目标会自动添加到所有新创建的记录器。
     */
    void addGlobalAppender(std::shared_ptr<LogAppender> appender);
    
    /**
     * @brief 移除全局输出目标
     * @param appender 输出目标
     */
    void removeGlobalAppender(std::shared_ptr<LogAppender> appender);
    
    /**
     * @brief 清除所有全局输出目标
     */
    void clearGlobalAppenders();
    
    /**
     * @brief 刷新所有记录器
     */
    void flushAll();
    
    /**
     * @brief 关闭日志系统
     */
    void shutdown();

private:
    LoggerManager();
    ~LoggerManager();

    std::unordered_map<std::string, std::shared_ptr<Logger>> loggers_; ///< 记录器映射
    std::vector<std::shared_ptr<LogAppender>> globalAppenders_;        ///< 全局输出目标
    LogLevel defaultLevel_;                                            ///< 默认日志级别
    std::mutex mutex_;                                                 ///< 线程安全互斥锁
};

/// @brief 便捷宏 - 记录Trace级别日志
#define LOG_TRACE(logger, message) logger->trace(message, __FILE__, __LINE__)

/// @brief 便捷宏 - 记录Debug级别日志
#define LOG_DEBUG(logger, message) logger->debug(message, __FILE__, __LINE__)

/// @brief 便捷宏 - 记录Info级别日志
#define LOG_INFO(logger, message) logger->info(message, __FILE__, __LINE__)

/// @brief 便捷宏 - 记录Warning级别日志
#define LOG_WARNING(logger, message) logger->warning(message, __FILE__, __LINE__)

/// @brief 便捷宏 - 记录Error级别日志
#define LOG_ERROR(logger, message) logger->error(message, __FILE__, __LINE__)

/// @brief 便捷宏 - 记录Fatal级别日志
#define LOG_FATAL(logger, message) logger->fatal(message, __FILE__, __LINE__)

/// @brief 便捷宏 - 获取或创建日志记录器
#define GET_LOGGER(name) v3d::core::LoggerManager::getInstance().getOrCreateLogger(name)

}
}
