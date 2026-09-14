#pragma once

#include <string>

namespace viibe
{
    struct PEValidationResult
    {
        bool valid{ false };
        bool isDll{ false };
        std::string architecture;
        unsigned long imageSize{ 0 };
        unsigned long entryPoint{ 0 };
        std::string error;
    };

    class PEValidator
    {
    public:
        static PEValidationResult Validate(
            const std::string& path
        );
    };
}
