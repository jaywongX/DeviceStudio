/**
 * @file logger.h
 * @brief 日志系统接口
 * @author DeviceStudio Team
 * @date 2025-02-14
 */

#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace DeviceStudio {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    Trace,      ///< 跟踪信息
    Debug,      ///< 调试信息
    Info,       ///< 一般信息
    Warning,    ///< 警告信息
    Error       ///< 错误信息
};

/**
 * @brief 日志系统类（单例模式）
 * 
 * 封装 spdlog 日志库，提供统一的日志接口
 */
class Logger {
public:
    /**
     * @brief 获取单例实例
     */
    static Logger& instance();
    
    /**
     * @brief 初始化日志系统
     * @param logPath 日志文件路径
     * @param level 日志级别
     * @param maxFileSize 最大文件大小（字节）
     * @param maxFiles 最大文件数量
     */
    void init(const std::string& logPath, 
              LogLevel level = LogLevel::Info,
              size_t maxFileSize = 1048576 * 100,  // 100 MB
              size_t maxFiles = 10);
    
    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param message 日志消息
     */
    void log(LogLevel level, const std::string& message);
    
    /**
     * @brief 记录日志（支持 fmt 格式化）
     * @tparam Args 格式化参数类型
     * @param level 日志级别
     * @param fmt 格式化字符串
     * @param args 格式化参数
     */
    template<typename... Args>
    void log(LogLevel level, spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        if (!logger_) {
            return;
        }
        switch (level) {
            case LogLevel::Trace:   logger_->trace(fmt, std::forward<Args>(args)...); break;
            case LogLevel::Debug:   logger_->debug(fmt, std::forward<Args>(args)...); break;
            case LogLevel::Info:    logger_->info(fmt, std::forward<Args>(args)...); break;
            case LogLevel::Warning: logger_->warn(fmt, std::forward<Args>(args)...); break;
            case LogLevel::Error:   logger_->error(fmt, std::forward<Args>(args)...); break;
        }
    }
    
    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void setLevel(LogLevel level);
    
    /**
     * @brief 获取日志级别
     */
    LogLevel getLevel() const;
    
    /**
     * @brief 刷新日志缓冲
     */
    void flush();
    
    /**
     * @brief 获取底层 spdlog logger
     */
    std::shared_ptr<spdlog::logger> getLogger() const { return logger_; }

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    /**
     * @brief 将 LogLevel 转换为 spdlog::level::level_enum
     */
    spdlog::level::level_enum toSpdlogLevel(LogLevel level) const;
    
    /**
     * @brief 将 spdlog::level::level_enum 转换为 LogLevel
     */
    LogLevel fromSpdlogLevel(spdlog::level::level_enum level) const;
    
    std::shared_ptr<spdlog::logger> logger_;  ///< spdlog logger 实例
    LogLevel currentLevel_ = LogLevel::Info;  ///< 当前日志级别
};

} // namespace DeviceStudio

// 日志宏定义（支持 fmt 格式化）
#define DS_LOG_TRACE(fmt, ...)   DeviceStudio::Logger::instance().log(DeviceStudio::LogLevel::Trace, fmt, ##__VA_ARGS__)
#define DS_LOG_DEBUG(fmt, ...)   DeviceStudio::Logger::instance().log(DeviceStudio::LogLevel::Debug, fmt, ##__VA_ARGS__)
#define DS_LOG_INFO(fmt, ...)    DeviceStudio::Logger::instance().log(DeviceStudio::LogLevel::Info, fmt, ##__VA_ARGS__)
#define DS_LOG_WARN(fmt, ...)    DeviceStudio::Logger::instance().log(DeviceStudio::LogLevel::Warning, fmt, ##__VA_ARGS__)
#define DS_LOG_WARNING(fmt, ...) DeviceStudio::Logger::instance().log(DeviceStudio::LogLevel::Warning, fmt, ##__VA_ARGS__)
#define DS_LOG_ERROR(fmt, ...)   DeviceStudio::Logger::instance().log(DeviceStudio::LogLevel::Error, fmt, ##__VA_ARGS__)
