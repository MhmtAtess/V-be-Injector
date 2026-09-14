#pragma once

#include <string>

namespace viibe
{
    class Logger
    {
    public:
        static void Initialize();

        static void Info(const std::string& message);
        static void Warning(const std::string& message);
        static void Error(const std::string& message);

    private:
        static void Write(
            const char* level,
            const std::string& message
        );
    };
}
