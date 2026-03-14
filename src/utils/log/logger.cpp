/**
 * @file logger.cpp
 * @brief 日志系统实现
 * @author DeviceStudio Team
 * @date 2026-03-14
 */

#include "logger.h"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>

namespace DeviceStudio {

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& logPath, 
                  LogLevel level, 
                  size_t maxFileSize, 
                  size_t maxFiles)
{
    try {
        // 创建 sinks 列表
        std::vector<spdlog::sink_ptr> sinks;
        
        // 控制台输出 sink
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
        sinks.push_back(consoleSink);
        
        // 文件输出 sink（滚动日志）
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logPath + "/devicestudio.log",
            maxFileSize,
            maxFiles
        );
        fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");
        sinks.push_back(fileSink);
        
        // 创建 logger
        logger_ = std::make_shared<spdlog::logger>("DeviceStudio", sinks.begin(), sinks.end());
        logger_->set_level(toSpdlogLevel(level));
        
        // 设置立即刷新（错误级别及以上）
        logger_->flush_on(spdlog::level::err);
        
        // 注册为默认 logger
        spdlog::register_logger(logger_);
        spdlog::set_default_logger(logger_);
        
        currentLevel_ = level;
        
        log(LogLevel::Info, "Logger initialized successfully");
        
    } catch (const spdlog::spdlog_ex& ex) {
        qDebug() << "Logger initialization failed:" << ex.what();
    }
}

void Logger::log(LogLevel level, const std::string& message)
{
    if (!logger_) {
        return;
    }
    
    switch (level) {
        case LogLevel::Debug:
            logger_->debug(message);
            break;
        case LogLevel::Info:
            logger_->info(message);
            break;
        case LogLevel::Warning:
            logger_->warn(message);
            break;
        case LogLevel::Error:
            logger_->error(message);
            break;
    }
}

void Logger::setLevel(LogLevel level)
{
    if (logger_) {
        logger_->set_level(toSpdlogLevel(level));
        currentLevel_ = level;
    }
}

LogLevel Logger::getLevel() const
{
    return currentLevel_;
}

void Logger::flush()
{
    if (logger_) {
        logger_->flush();
    }
}

spdlog::level::level_enum Logger::toSpdlogLevel(LogLevel level) const
{
    switch (level) {
        case LogLevel::Debug:   return spdlog::level::debug;
        case LogLevel::Info:    return spdlog::level::info;
        case LogLevel::Warning: return spdlog::level::warn;
        case LogLevel::Error:   return spdlog::level::err;
        default:                return spdlog::level::info;
    }
}

LogLevel Logger::fromSpdlogLevel(spdlog::level::level_enum level) const
{
    switch (level) {
        case spdlog::level::debug: return LogLevel::Debug;
        case spdlog::level::info:  return LogLevel::Info;
        case spdlog::level::warn:  return LogLevel::Warning;
        case spdlog::level::err:   return LogLevel::Error;
        default:                   return LogLevel::Info;
    }
}

} // namespace DeviceStudio
