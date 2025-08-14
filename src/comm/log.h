/**
* @file log.h
* @brief 日志, 单例，有任务队列独立线程
* @author liushisheng
* @date 2025-08-14
*/

#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>

enum class LogLevel
{
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
};

#define LOG_DEBUG(msg) Log::getInstance().log(LogLevel::LOG_DEBUG, msg, __FILE__, __LINE__, __func__)
#define LOG_INFO(msg) Log::getInstance().log(LogLevel::LOG_INFO, msg, __FILE__, __LINE__, __func__)
#define LOG_WARN(msg) Log::getInstance().log(LogLevel::LOG_WARN, msg, __FILE__, __LINE__, __func__)
#define LOG_ERROR(msg) Log::getInstance().log(LogLevel::LOG_ERROR, msg, __FILE__, __LINE__, __func__)
#define LOG_FATAL(msg) Log::getInstance().log(LogLevel::LOG_FATAL, msg, __FILE__, __LINE__, __func__)
#define LOG_LEVEL(level) Log::getInstance().setLevel(level)

class Log
{
public:
    static Log& getInstance();

    void setLevel(LogLevel level);
    void log(LogLevel level, const std::string& message,
        const char* file, int line, const char* func);

private:
    Log();
    ~Log();
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    void processQueue();
    void writeToFile(const std::string& msg);
    std::string LevelToString(LogLevel level);
    std::string getTime();
    std::string getDate();

private:
    LogLevel m_level;
    std::ofstream m_logFile;
    std::string m_logFileName;
    std::string m_currentDate;

    std::mutex m_queueMutex;
    
    std::queue<std::string> m_queue;
    std::condition_variable m_cv;

    std::thread m_worker;
    bool m_exit;
};

#endif // LOG_H
