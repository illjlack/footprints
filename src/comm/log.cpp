/**
* @file log.cpp
* @brief 日志, 单例，有任务队列独立线程
* @author liushisheng
* @date 2025-08-14
*/

#include "log.h"
#include <sstream>
#include <chrono>
#include <ctime>
#include <thread>

Log& Log::getInstance()
{
    static Log instance;
    return instance;
}

Log::Log()
    :m_level(LogLevel::LOG_FATAL),
    m_exit(false)
{
    m_worker = std::thread([this](){this->processQueue();});
}

Log::~Log()
{
    m_exit = true;
    m_cv.notify_one();
    if(m_worker.joinable())
        m_worker.join();
    if(m_logFile.is_open())
        m_logFile.close();
}

void Log::setLevel(LogLevel level)
{
    m_level = level;
}

void Log::log(LogLevel level, const std::string& message,
    const char* file, int line, const char*func)
{
    if(level < m_level) return;

    std::ostringstream oss;
    oss << "[" << getTime() << "] ";
    oss << "[" << std::this_thread::get_id() << "] ";
    oss << LevelToString(level) << " ";
    oss << "(" << file << ":" << line << " " << func << ") ";
    oss << message;

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_queue.push(oss.str());
    }
    m_cv.notify_one();
}

void Log::processQueue()
{
    while(!m_exit)
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_cv.wait(lock, [this](){return !m_queue.empty() || m_exit;});

        while (!m_queue.empty()) 
        {
            std::string msg = m_queue.front();
            m_queue.pop();
            lock.unlock();

            writeToFile(msg);
            std::cout << msg << std::endl;
            
            lock.lock();
        }
    }

    while(!m_queue.empty())
    {
        std::string msg = m_queue.front();
        m_queue.pop();
        writeToFile(msg);
        std::cout << msg << std::endl;
    }
}

void Log::writeToFile(const std::string& msg)
{
    std::string today = getDate();
    if(today != m_currentDate) 
    {
        if(m_logFile.is_open())
        {
            m_logFile.close();   
        }

        m_currentDate = today;

        std::string fileName = m_currentDate + ".log";
        
#ifdef LOG_FILES_PATH
        std::string folder = LOG_FILES_PATH;
        fileName= folder + '/' + fileName;
#endif
        m_logFile.open(fileName, std::ios::app | std::ios::binary);
    }

    if(m_logFile.is_open())
    {
        m_logFile << msg << std::endl;
        m_logFile.flush();
    }
}

std::string Log::LevelToString(LogLevel level)
{
    switch(level)
    {
        case LogLevel::LOG_DEBUG:   return "[DEBUG]";
        case LogLevel::LOG_INFO:    return "[INFO ]";
        case LogLevel::LOG_WARN:    return "[WARN ]";
        case LogLevel::LOG_ERROR:   return "[ERROR]";
        case LogLevel::LOG_FATAL:   return "[FATAL]";
        default:                    return "[UNKN ]";
    }
}

std::string Log::getTime() 
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));
    return buf;
}

std::string Log::getDate()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return buf;
}
