#pragma once

#include <string>
#include <vector>
#include <Windows.h>

namespace viibe
{
    struct ProcessInfo
    {
        DWORD processId;
        std::string name;
    };

    class ProcessManager
    {
    public:
        static std::vector<ProcessInfo> Enumerate();
        static HANDLE OpenProcessForInspection(DWORD processId);
        static bool InjectDLL(DWORD processId, const std::string& dllPath, std::string& outError);
    };
}
