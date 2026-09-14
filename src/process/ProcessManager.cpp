#include "process/ProcessManager.hpp"

#include <Windows.h>
#include <TlHelp32.h>

namespace viibe
{
    std::vector<ProcessInfo>
    ProcessManager::Enumerate()
    {
        std::vector<ProcessInfo> result;

        HANDLE snapshot =
            CreateToolhelp32Snapshot(
                TH32CS_SNAPPROCESS,
                0
            );

        if (snapshot == INVALID_HANDLE_VALUE)
            return result;

        PROCESSENTRY32 entry{};
        entry.dwSize = sizeof(entry);

        if (Process32First(snapshot, &entry))
        {
            do
            {
                ProcessInfo info;

                info.processId =
                    entry.th32ProcessID;

                info.name =
                    entry.szExeFile;

                result.push_back(
                    std::move(info)
                );

            } while (Process32Next(
                snapshot,
                &entry
            ));
        }

        CloseHandle(snapshot);

        return result;
    }

    HANDLE ProcessManager::OpenProcessForInspection(
        DWORD processId
    )
    {
        return OpenProcess(
            PROCESS_QUERY_LIMITED_INFORMATION,
            FALSE,
            processId
        );
    }

    bool ProcessManager::InjectDLL(
        DWORD processId,
        const std::string& dllPath,
        std::string& outError
    )
    {
        if (dllPath.empty())
        {
            outError = "DLL path is empty.";
            return false;
        }

        HANDLE hProcess = OpenProcess(
            PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
            PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
            FALSE,
            processId
        );

        if (!hProcess)
        {
            outError = "Failed to open target process. Error code: " + std::to_string(GetLastError());
            return false;
        }

        const size_t pathSize = (dllPath.length() + 1) * sizeof(char);

        LPVOID pRemoteBuf = VirtualAllocEx(
            hProcess,
            nullptr,
            pathSize,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE
        );

        if (!pRemoteBuf)
        {
            outError = "Failed to allocate memory in target process. Error: " + std::to_string(GetLastError());
            CloseHandle(hProcess);
            return false;
        }

        SIZE_T bytesWritten = 0;
        if (!WriteProcessMemory(
                hProcess,
                pRemoteBuf,
                dllPath.c_str(),
                pathSize,
                &bytesWritten) || bytesWritten < pathSize)
        {
            outError = "Failed to write DLL path into target process memory.";
            VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false;
        }

        HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
        if (!hKernel32)
        {
            outError = "Failed to locate kernel32.dll.";
            VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false;
        }

        LPVOID pLoadLibrary = (LPVOID)GetProcAddress(hKernel32, "LoadLibraryA");
        if (!pLoadLibrary)
        {
            outError = "Failed to locate LoadLibraryA.";
            VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false;
        }

        HANDLE hThread = CreateRemoteThread(
            hProcess,
            nullptr,
            0,
            (LPTHREAD_START_ROUTINE)pLoadLibrary,
            pRemoteBuf,
            0,
            nullptr
        );

        if (!hThread)
        {
            outError = "Failed to create remote thread. Error: " + std::to_string(GetLastError());
            VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false;
        }

        WaitForSingleObject(hThread, 4000);

        DWORD exitCode = 0;
        GetExitCodeThread(hThread, &exitCode);

        VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
        CloseHandle(hThread);
        CloseHandle(hProcess);

        if (exitCode == 0)
        {
            outError = "LoadLibraryA failed in remote process (returned NULL / 0). Check bitness (x64/x86) and dependencies.";
            return false;
        }

        return true;
    }
}
