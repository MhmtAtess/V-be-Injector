#pragma once

#include <string>

namespace viibe
{
    class HashValidator
    {
    public:
        static std::string SHA256(
            const std::string& path
        );
    };
}
