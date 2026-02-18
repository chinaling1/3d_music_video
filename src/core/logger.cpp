#include "logger.h"
#include <iostream>
#include <filesystem>
#include <iomanip>

namespace v3d {
namespace core {

LogMessage::LogMessage(LogLevel level, const std::string& loggerName, const std::string& file, int line)
    : level_(level)
    , loggerName_(loggerName)
    , file_(file)
    , line_(line)
    , timestamp_(std::chrono::system_clock::now()) {
}

std::string LogMessage::toString() const {
    std::stringstream ss;

    auto time_t = std::chrono::system_clock::to_time_t(timestamp_);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp_.time_since_epoch()) % 1000;

    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();

    ss << " [" << loggerName_ << "]";

    switch (level_) {
        case LogLevel::Trace: ss << " [TRACE]"; break;
        case LogLevel::Debug: ss << " [DEBUG]"; break;
        case LogLevel::Info: ss << " [INFO]"; break;
        case LogLevel::Warning: ss << " [WARN]"; break;
        case LogLevel::Error: ss << " [ERROR]"; break;
        case LogLevel::Fatal: ss << " [FATAL]"; break;
    }

    if (!file_.empty()) {
        ss << " [" << file_ << ":" << line_ << "]";
    }

    ss << " " << message_;

    return ss.str();
}

LogLevel LogMessage::getLevel() const {
    return level_;
}

std::string LogMessage::getLoggerName() const {
    return loggerName_;
}

std::string LogMessage::getFile() const {
    return file_;
}

int LogMessage::getLine() const {
    return line_;
}

std::chrono::system_clock::time_point LogMessage::getTimestamp() const {
    return timestamp_;
}

std::string LogMessage::getMessage() const {
    return message_;
}

void LogMessage::setMessage(const std::string& message) {
    message_ = message;
}

ConsoleAppender::ConsoleAppender(bool useColor)
    : useColor_(useColor) {
}

void ConsoleAppender::append(const LogMessage& message) {
    if (useColor_) {
        std::cout << getColorCode(message.getLevel());
    }

    std::cout << message.toString() << std::endl;

    if (useColor_) {
        std::cout << "\033[0m";
    }
}

void ConsoleAppender::flush() {
    std::cout.flush();
}

std::string ConsoleAppender::getColorCode(LogLevel level) const {
    switch (level) {
        case LogLevel::Trace: return "\033[90m";
        case LogLevel::Debug: return "\033[36m";
        case LogLevel::Info: return "\033[32m";
        case LogLevel::Warning: return "\033[33m";
        case LogLevel::Error: return "\033[31m";
        case LogLevel::Fatal: return "\033[35m";
        default: return "";
    }
}

FileAppender::FileAppender(const std::string& filePath, bool append) {
    setFilePath(filePath);
}

FileAppender::~FileAppender() {
    if (file_.is_open()) {
        file_.close();
    }
}

void FileAppender::append(const LogMessage& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!file_.is_open()) {
        return;
    }

    file_ << message.toString() << std::endl;
}

void FileAppender::flush() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (file_.is_open()) {
        file_.flush();
    }
}

void FileAppender::setFilePath(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (file_.is_open()) {
        file_.close();
    }

    filePath_ = filePath;

    std::filesystem::path path(filePath);
    std::filesystem::create_directories(path.parent_path());

    file_.open(filePath_, std::ios::out | std::ios::app);
}

RollingFileAppender::RollingFileAppender(const std::string& filePath, size_t maxFileSize, int maxFiles)
    : filePath_(filePath)
    , maxFileSize_(maxFileSize)
    , maxFiles_(maxFiles)
    , currentSize_(0) {

    std::filesystem::path path(filePath);
    std::filesystem::create_directories(path.parent_path());

    file_.open(filePath_, std::ios::out | std::ios::app);

    if (file_.is_open()) {
        file_.seekp(0, std::ios::end);
        currentSize_ = file_.tellp();
    }
}

RollingFileAppender::~RollingFileAppender() {
    if (file_.is_open()) {
        file_.close();
    }
}

void RollingFileAppender::append(const LogMessage& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!file_.is_open()) {
        return;
    }

    std::string messageStr = message.toString();
    size_t messageSize = messageStr.size() + 1;

    if (currentSize_ + messageSize > maxFileSize_) {
        rollFile();
    }

    file_ << messageStr << std::endl;
    currentSize_ += messageSize;
}

void RollingFileAppender::flush() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (file_.is_open()) {
        file_.flush();
    }
}

void RollingFileAppender::rollFile() {
    file_.close();

    for (int i = maxFiles_ - 1; i >= 1; --i) {
        std::string oldFile = getRollFileName(i);
        std::string newFile = getRollFileName(i + 1);

        if (std::filesystem::exists(oldFile)) {
            std::filesystem::rename(oldFile, newFile);
        }
    }

    std::string backupFile = getRollFileName(1);
    if (std::filesystem::exists(filePath_)) {
        std::filesystem::rename(filePath_, backupFile);
    }

    file_.open(filePath_, std::ios::out | std::ios::app);
    currentSize_ = 0;
}

std::string RollingFileAppender::getRollFileName(int index) const {
    if (index == 0) {
        return filePath_;
    }

    std::filesystem::path path(filePath_);
    std::string stem = path.stem().string();
    std::string extension = path.extension().string();

    return path.parent_path().string() + "/" + stem + "." + std::to_string(index) + extension;
}

void RollingFileAppender::setMaxFileSize(size_t maxSize) {
    maxFileSize_ = maxSize;
}

void RollingFileAppender::setMaxFiles(int maxFiles) {
    maxFiles_ = maxFiles;
}

Logger::Logger(const std::string& name)
    : name_(name)
    , level_(LogLevel::Info) {
}

Logger::~Logger() {
    flush();
}

void Logger::log(LogLevel level, const std::string& message, const char* file, int line) {
    if (level < level_) {
        return;
    }

    LogMessage logMessage(level, name_, file ? file : "", line);
    logMessage.setMessage(message);

    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& appender : appenders_) {
        appender->append(logMessage);
    }
}

void Logger::trace(const std::string& message, const char* file, int line) {
    log(LogLevel::Trace, message, file, line);
}

void Logger::debug(const std::string& message, const char* file, int line) {
    log(LogLevel::Debug, message, file, line);
}

void Logger::info(const std::string& message, const char* file, int line) {
    log(LogLevel::Info, message, file, line);
}

void Logger::warning(const std::string& message, const char* file, int line) {
    log(LogLevel::Warning, message, file, line);
}

void Logger::error(const std::string& message, const char* file, int line) {
    log(LogLevel::Error, message, file, line);
}

void Logger::fatal(const std::string& message, const char* file, int line) {
    log(LogLevel::Fatal, message, file, line);
}

void Logger::addAppender(std::shared_ptr<LogAppender> appender) {
    std::lock_guard<std::mutex> lock(mutex_);
    appenders_.push_back(appender);
}

void Logger::removeAppender(std::shared_ptr<LogAppender> appender) {
    std::lock_guard<std::mutex> lock(mutex_);
    appenders_.erase(std::remove(appenders_.begin(), appenders_.end(), appender), appenders_.end());
}

void Logger::clearAppenders() {
    std::lock_guard<std::mutex> lock(mutex_);
    appenders_.clear();
}

void Logger::setLevel(LogLevel level) {
    level_ = level;
}

LogLevel Logger::getLevel() const {
    return level_;
}

std::string Logger::getName() const {
    return name_;
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& appender : appenders_) {
        appender->flush();
    }
}

LoggerManager& LoggerManager::getInstance() {
    static LoggerManager instance;
    return instance;
}

LoggerManager::LoggerManager()
    : defaultLevel_(LogLevel::Info) {
}

LoggerManager::~LoggerManager() {
    shutdown();
}

std::shared_ptr<Logger> LoggerManager::getLogger(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = loggers_.find(name);
    if (it != loggers_.end()) {
        return it->second;
    }

    return nullptr;
}

std::shared_ptr<Logger> LoggerManager::getOrCreateLogger(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = loggers_.find(name);
    if (it != loggers_.end()) {
        return it->second;
    }

    auto logger = std::make_shared<Logger>(name);
    logger->setLevel(defaultLevel_);

    for (auto& appender : globalAppenders_) {
        logger->addAppender(appender);
    }

    loggers_[name] = logger;
    return logger;
}

void LoggerManager::setDefaultLevel(LogLevel level) {
    defaultLevel_ = level;
}

LogLevel LoggerManager::getDefaultLevel() const {
    return defaultLevel_;
}

void LoggerManager::addGlobalAppender(std::shared_ptr<LogAppender> appender) {
    std::lock_guard<std::mutex> lock(mutex_);

    globalAppenders_.push_back(appender);

    for (auto& pair : loggers_) {
        pair.second->addAppender(appender);
    }
}

void LoggerManager::removeGlobalAppender(std::shared_ptr<LogAppender> appender) {
    std::lock_guard<std::mutex> lock(mutex_);

    globalAppenders_.erase(std::remove(globalAppenders_.begin(), globalAppenders_.end(), appender),
                          globalAppenders_.end());

    for (auto& pair : loggers_) {
        pair.second->removeAppender(appender);
    }
}

void LoggerManager::clearGlobalAppenders() {
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& appender : globalAppenders_) {
        for (auto& pair : loggers_) {
            pair.second->removeAppender(appender);
        }
    }

    globalAppenders_.clear();
}

void LoggerManager::flushAll() {
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& pair : loggers_) {
        pair.second->flush();
    }
}

void LoggerManager::shutdown() {
    flushAll();

    std::lock_guard<std::mutex> lock(mutex_);
    loggers_.clear();
    globalAppenders_.clear();
}

}
}