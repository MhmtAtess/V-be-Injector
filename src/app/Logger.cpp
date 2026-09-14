#include "app/Logger.hpp"

#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>

namespace
{
    std::ofstream g_logFile;
    std::mutex g_logMutex;

    std::string Timestamp()
    {
        const auto now =
            std::chrono::system_clock::now();

        const auto time =
            std::chrono::system_clock::to_time_t(now);

        std::tm tm{};

        localtime_s(&tm, &time);

        std::ostringstream stream;

        stream << std::put_time(
            &tm,
            "%Y-%m-%d %H:%M:%S"
        );

        return stream.str();
    }
}

namespace viibe
{
    void Logger::Initialize()
    {
        std::filesystem::create_directories("logs");

        g_logFile.open(
            "logs/session.log",
            std::ios::app
        );
    }

    void Logger::Write(
        const char* level,
        const std::string& message
    )
    {
        std::lock_guard<std::mutex> lock(g_logMutex);

        const std::string line =
            "[" + Timestamp() + "] [" +
            level + "] " +
            message;

        if (g_logFile.is_open())
        {
            g_logFile << line << '\n';
            g_logFile.flush();
        }
    }

    void Logger::Info(const std::string& message)
    {
        Write("INFO", message);
    }

    void Logger::Warning(const std::string& message)
    {
        Write("WARNING", message);
    }

    void Logger::Error(const std::string& message)
    {
        Write("ERROR", message);
    }
}
