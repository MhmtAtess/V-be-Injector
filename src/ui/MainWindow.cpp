#define IMGUI_DEFINE_MATH_OPERATORS
#include "ui/MainWindow.hpp"

#include "app/Logger.hpp"
#include "process/ProcessManager.hpp"
#include "validation/PEValidator.hpp"
#include "validation/HashValidator.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_opengl2.h"

#include <Windows.h>
#include <GL/gl.h>
#include <commdlg.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
    HWND g_hwnd = nullptr;
    HDC g_dc = nullptr;
    HGLRC g_glrc = nullptr;

    const int WINDOW_WIDTH = 780;
    const int WINDOW_HEIGHT = 520;

    std::vector<viibe::ProcessInfo> g_processes;
    int g_selectedProcessId = -1;
    std::string g_selectedProcessName = "";
    char g_processSearch[128] = "";

    char g_dllPath[MAX_PATH]{};
    viibe::PEValidationResult g_validation;
    std::string g_hash;

    std::string g_injectMessage = "";
    bool g_injectSuccess = false;

    bool g_running = true;

    const int NUM_PARTICLES = 25;
    ImVec2 g_particlePositions[NUM_PARTICLES];
    ImVec2 g_particleVelocities[NUM_PARTICLES];
    bool g_particlesInitialized = false;

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return 1;

        switch (msg)
        {
        case WM_CLOSE:
            g_running = false;
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }

    bool CreateWindowAndContext()
    {
        WNDCLASSA wc{};
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandleA(nullptr);
        wc.lpszClassName = "VIIBE_Loader_Window";

        if (!RegisterClassA(&wc))
            return false;

        const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        const int posX = (screenWidth - WINDOW_WIDTH) / 2;
        const int posY = (screenHeight - WINDOW_HEIGHT) / 2;

        g_hwnd = CreateWindowExA(
            WS_EX_APPWINDOW,
            wc.lpszClassName,
            "VIIBE Loader",
            WS_POPUP | WS_VISIBLE | WS_MINIMIZEBOX,
            posX,
            posY,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            nullptr,
            nullptr,
            wc.hInstance,
            nullptr
        );

        if (!g_hwnd)
            return false;

        g_dc = GetDC(g_hwnd);

        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;

        const int format = ChoosePixelFormat(g_dc, &pfd);
        SetPixelFormat(g_dc, format, &pfd);

        g_glrc = wglCreateContext(g_dc);
        if (!g_glrc)
            return false;

        wglMakeCurrent(g_dc, g_glrc);
        ShowWindow(g_hwnd, SW_SHOW);

        return true;
    }

    void SetupImGuiStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();

        style.Colors[ImGuiCol_WindowBg] = ImVec4(20 / 255.f, 20 / 255.f, 24 / 255.f, 0.98f);
        style.Colors[ImGuiCol_Border] = ImVec4(48 / 255.f, 38 / 255.f, 65 / 255.f, 0.85f);
        style.Colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.94f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);
        style.Colors[ImGuiCol_Button] = ImVec4(32 / 255.f, 28 / 255.f, 40 / 255.f, 0.85f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(60 / 255.f, 28 / 255.f, 85 / 255.f, 0.90f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(85 / 255.f, 22 / 255.f, 115 / 255.f, 1.00f);
        style.Colors[ImGuiCol_Header] = ImVec4(36 / 255.f, 28 / 255.f, 48 / 255.f, 0.80f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(58 / 255.f, 32 / 255.f, 82 / 255.f, 0.85f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(85 / 255.f, 22 / 255.f, 115 / 255.f, 1.00f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(214 / 255.f, 20 / 255.f, 255 / 255.f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(180 / 255.f, 40 / 255.f, 220 / 255.f, 1.00f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(16 / 255.f, 16 / 255.f, 20 / 255.f, 0.90f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(28 / 255.f, 24 / 255.f, 36 / 255.f, 0.90f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(48 / 255.f, 24 / 255.f, 65 / 255.f, 0.95f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(15 / 255.f, 15 / 255.f, 18 / 255.f, 0.65f);
        style.Colors[ImGuiCol_Separator] = ImVec4(45 / 255.f, 35 / 255.f, 58 / 255.f, 0.80f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(12 / 255.f, 12 / 255.f, 14 / 255.f, 0.60f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(40 / 255.f, 32 / 255.f, 52 / 255.f, 0.80f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(65 / 255.f, 35 / 255.f, 90 / 255.f, 0.85f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(95 / 255.f, 35 / 255.f, 130 / 255.f, 0.95f);

        style.WindowRounding = 9.0f;
        style.ChildRounding = 7.0f;
        style.FrameRounding = 5.0f;
        style.PopupRounding = 5.0f;
        style.ScrollbarRounding = 6.0f;
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.WindowPadding = ImVec2(0, 0);
        style.FramePadding = ImVec2(8, 6);
        style.ItemSpacing = ImVec2(8, 8);
    }

    void RenderParticles(ImDrawList* draw, ImVec2 winPos, ImVec2 winSize)
    {
        if (!g_particlesInitialized)
        {
            for (int i = 0; i < NUM_PARTICLES; ++i)
            {
                g_particlePositions[i] = ImVec2(
                    winPos.x + winSize.x * static_cast<float>(rand()) / RAND_MAX,
                    winPos.y + winSize.y * static_cast<float>(rand()) / RAND_MAX
                );

                g_particleVelocities[i] = ImVec2(
                    static_cast<float>((rand() % 11) - 5),
                    static_cast<float>((rand() % 11) - 5)
                );
            }
            g_particlesInitialized = true;
        }

        ImVec2 cursorPos = ImGui::GetIO().MousePos;
        for (int i = 0; i < NUM_PARTICLES; ++i)
        {
            for (int j = i + 1; j < NUM_PARTICLES; ++j)
            {
                float distance = std::hypotf(
                    g_particlePositions[j].x - g_particlePositions[i].x,
                    g_particlePositions[j].y - g_particlePositions[i].y
                );
                float opacity = 0.40f - (distance / 400.0f);

                if (opacity > 0.0f)
                {
                    ImU32 lineColor = ImGui::GetColorU32(ImVec4(0.80f, 0.15f, 1.0f, opacity));
                    draw->AddLine(g_particlePositions[i], g_particlePositions[j], lineColor, 1.1f);
                }
            }

            float distanceToCursor = std::hypotf(
                cursorPos.x - g_particlePositions[i].x,
                cursorPos.y - g_particlePositions[i].y
            );
            float opacityToCursor = 0.90f - (distanceToCursor / 90.0f);

            if (opacityToCursor > 0.0f)
            {
                ImU32 lineColorToCursor = ImGui::GetColorU32(ImVec4(1.0f, 0.2f, 0.9f, opacityToCursor));
                draw->AddLine(cursorPos, g_particlePositions[i], lineColorToCursor, 1.3f);
            }
        }

        float deltaTime = ImGui::GetIO().DeltaTime * 20.0f;
        for (int i = 0; i < NUM_PARTICLES; ++i)
        {
            g_particlePositions[i].x += g_particleVelocities[i].x * deltaTime;
            g_particlePositions[i].y += g_particleVelocities[i].y * deltaTime;

            if (g_particlePositions[i].x < winPos.x)
                g_particlePositions[i].x = winPos.x + winSize.x;
            else if (g_particlePositions[i].x > winPos.x + winSize.x)
                g_particlePositions[i].x = winPos.x;

            if (g_particlePositions[i].y < winPos.y)
                g_particlePositions[i].y = winPos.y + winSize.y;
            else if (g_particlePositions[i].y > winPos.y + winSize.y)
                g_particlePositions[i].y = winPos.y;

            ImU32 particleColour = ImGui::ColorConvertFloat4ToU32(ImVec4(180 / 255.f, 25 / 255.f, 225 / 255.f, 0.95f));
            draw->AddCircleFilled(g_particlePositions[i], 3.0f, particleColour);
        }
    }

    void RefreshProcesses()
    {
        g_processes = viibe::ProcessManager::Enumerate();
    }

    std::string ToLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return str;
    }

    void BrowseDLL()
    {
        OPENFILENAMEA dialog{};
        dialog.lStructSize = sizeof(dialog);
        dialog.hwndOwner = g_hwnd;
        dialog.lpstrFilter = "Dynamic Link Library (*.dll)\0*.dll\0All Files (*.*)\0*.*\0";
        dialog.lpstrFile = g_dllPath;
        dialog.nMaxFile = MAX_PATH;
        dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileNameA(&dialog))
        {
            g_validation = viibe::PEValidator::Validate(g_dllPath);
            g_hash.clear();

            if (g_validation.valid)
            {
                g_hash = viibe::HashValidator::SHA256(g_dllPath);
            }
        }
    }

    void CopyToClipboard(const std::string& text)
    {
        if (OpenClipboard(g_hwnd))
        {
            EmptyClipboard();
            HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
            if (hGlob)
            {
                memcpy(GlobalLock(hGlob), text.c_str(), text.size() + 1);
                GlobalUnlock(hGlob);
                SetClipboardData(CF_TEXT, hGlob);
            }
            CloseClipboard();
        }
    }

    void RenderUI()
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)));

        ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("##MainRootWindow", nullptr, windowFlags);
        {
            ImDrawList* draw = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetWindowPos();
            ImVec2 size = ImGui::GetWindowSize();

            RenderParticles(draw, pos, size);

            const float headerHeight = 44.0f;
            draw->AddRectFilled(
                pos,
                ImVec2(pos.x + size.x, pos.y + headerHeight),
                ImGui::ColorConvertFloat4ToU32(ImVec4(16 / 255.f, 16 / 255.f, 20 / 255.f, 0.95f)),
                9.0f,
                ImDrawFlags_RoundCornersTop
            );
            draw->AddLine(
                ImVec2(pos.x, pos.y + headerHeight),
                ImVec2(pos.x + size.x, pos.y + headerHeight),
                ImColor(55, 42, 75, 240),
                1.5f
            );

            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::InvisibleButton("##TitleBarDrag", ImVec2(size.x - 70, headerHeight));
            if (ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                ReleaseCapture();
                SendMessageA(g_hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }

            static float rainbowHue = 0.0f;
            rainbowHue += ImGui::GetIO().DeltaTime * 0.35f;
            if (rainbowHue > 1.0f) rainbowHue -= 1.0f;
            float r, g, b;
            ImGui::ColorConvertHSVtoRGB(rainbowHue, 0.85f, 1.0f, r, g, b);
            ImU32 rainbowColor = IM_COL32(static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255), 255);

            const char* titleText = "VIIBE Loader";
            ImVec2 titleSize = ImGui::CalcTextSize(titleText);
            float titleX = pos.x + (size.x - titleSize.x) * 0.5f;
            float titleY = pos.y + (headerHeight - titleSize.y) * 0.5f;

            draw->AddText(ImVec2(titleX, titleY), rainbowColor, titleText);

            ImGui::SetCursorPos(ImVec2(size.x - 68, 8));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(45 / 255.f, 35 / 255.f, 60 / 255.f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(70 / 255.f, 20 / 255.f, 95 / 255.f, 1.0f));

            if (ImGui::Button("-", ImVec2(28, 28)))
            {
                ShowWindow(g_hwnd, SW_MINIMIZE);
            }
            ImGui::SameLine(0, 4);
            if (ImGui::Button("X", ImVec2(28, 28)))
            {
                g_running = false;
            }
            ImGui::PopStyleColor(3);

            const float contentY = headerHeight + 14.0f;
            const float contentHeight = size.y - contentY - 14.0f;
            const float col1Width = 360.0f;
            const float col2Width = size.x - col1Width - 36.0f;

            ImGui::SetCursorPos(ImVec2(16, contentY));
            ImGui::BeginChild("##ProcessesPanel", ImVec2(col1Width, contentHeight), true);
            {
                ImGui::SetCursorPos(ImVec2(12, 12));
                ImGui::TextColored(ImVec4(0.85f, 0.40f, 1.0f, 1.0f), "TARGET PROCESS");
                ImGui::SameLine(col1Width - 85);
                if (ImGui::Button("Refresh", ImVec2(70, 22)))
                {
                    RefreshProcesses();
                }

                ImGui::SetCursorPos(ImVec2(12, 38));
                ImGui::PushItemWidth(col1Width - 24);
                ImGui::InputTextWithHint("##SearchBox", "Search process (e.g. javaw, game)...", g_processSearch, sizeof(g_processSearch));
                ImGui::PopItemWidth();

                std::string searchFilter = ToLower(g_processSearch);
                std::vector<const viibe::ProcessInfo*> filteredProcesses;
                filteredProcesses.reserve(g_processes.size());

                for (const auto& proc : g_processes)
                {
                    if (searchFilter.empty())
                    {
                        filteredProcesses.push_back(&proc);
                    }
                    else
                    {
                        std::string lowerName = ToLower(proc.name);
                        std::string pidStr = std::to_string(proc.processId);
                        if (lowerName.find(searchFilter) != std::string::npos || pidStr.find(searchFilter) != std::string::npos)
                        {
                            filteredProcesses.push_back(&proc);
                        }
                    }
                }

                ImGui::SetCursorPos(ImVec2(12, 70));
                ImGui::TextDisabled("Matches: %d / %d processes", static_cast<int>(filteredProcesses.size()), static_cast<int>(g_processes.size()));

                ImGui::SetCursorPos(ImVec2(12, 92));
                ImGui::BeginChild("##ProcessListBox", ImVec2(col1Width - 24, contentHeight - 146), true);
                {
                    for (const auto* proc : filteredProcesses)
                    {
                        ImGui::PushID(static_cast<int>(proc->processId));
                        const bool isSelected = (g_selectedProcessId == static_cast<int>(proc->processId));

                        std::string itemLabel = proc->name;
                        std::string pidLabel = "PID: " + std::to_string(proc->processId);

                        if (ImGui::Selectable(itemLabel.c_str(), isSelected, ImGuiSelectableFlags_None, ImVec2(0, 22)))
                        {
                            g_selectedProcessId = static_cast<int>(proc->processId);
                            g_selectedProcessName = proc->name;
                        }

                        ImGui::SameLine(col1Width - 120);
                        ImGui::TextDisabled("%s", pidLabel.c_str());

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();

                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                ImGui::SetCursorPos(ImVec2(12, contentHeight - 44));
                if (g_selectedProcessId >= 0)
                {
                    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.5f, 1.0f), "Selected:");
                    ImGui::SameLine();
                    ImGui::Text("%s [%d]", g_selectedProcessName.c_str(), g_selectedProcessId);
                }
                else
                {
                    ImGui::TextDisabled("No process selected");
                }
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(16 + col1Width + 10, contentY));
            ImGui::BeginChild("##DllPanel", ImVec2(col2Width, contentHeight), true);
            {
                ImGui::SetCursorPos(ImVec2(14, 12));
                ImGui::TextColored(ImVec4(0.85f, 0.40f, 1.0f, 1.0f), "DLL / PLUGIN MODULE");

                ImGui::SetCursorPos(ImVec2(14, 38));
                ImGui::PushItemWidth(col2Width - 105);
                ImGui::InputText("##dllpath", g_dllPath, MAX_PATH);
                ImGui::PopItemWidth();

                ImGui::SameLine(col2Width - 85);
                if (ImGui::Button("Browse", ImVec2(72, 24)))
                {
                    BrowseDLL();
                }

                ImGui::SetCursorPos(ImVec2(14, 76));
                ImGui::Separator();

                ImGui::SetCursorPos(ImVec2(14, 90));
                ImGui::TextColored(ImVec4(0.85f, 0.40f, 1.0f, 1.0f), "VALIDATION REPORT");

                ImGui::SetCursorPos(ImVec2(14, 116));
                if (g_validation.valid)
                {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "Status:  VALID PE DLL");
                    ImGui::Spacing();

                    ImGui::Text("Architecture:");
                    ImGui::SameLine(130);
                    ImGui::TextColored(ImVec4(0.8f, 0.5f, 1.0f, 1.0f), "%s", g_validation.architecture.c_str());

                    ImGui::Text("Image Size:");
                    ImGui::SameLine(130);
                    ImGui::Text("0x%lX", g_validation.imageSize);

                    ImGui::Text("Entry Point:");
                    ImGui::SameLine(130);
                    ImGui::Text("0x%lX", g_validation.entryPoint);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    if (!g_hash.empty())
                    {
                        ImGui::Text("SHA-256 Checksum:");
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(12 / 255.f, 12 / 255.f, 16 / 255.f, 0.9f));
                        ImGui::SetNextItemWidth(col2Width - 28);
                        char hashBuf[128];
                        strncpy_s(hashBuf, g_hash.c_str(), sizeof(hashBuf));
                        ImGui::InputText("##shatext", hashBuf, sizeof(hashBuf), ImGuiInputTextFlags_ReadOnly);
                        ImGui::PopStyleColor();

                        if (ImGui::Button("Copy SHA-256 Hash", ImVec2(col2Width - 28, 26)))
                        {
                            CopyToClipboard(g_hash);
                        }
                    }
                }
                else
                {
                    if (strlen(g_dllPath) == 0)
                    {
                        ImGui::TextDisabled("Select a DLL file to inspect and validate headers.");
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Status:  INVALID / NOT A VALID DLL");
                        if (!g_validation.error.empty())
                        {
                            ImGui::Spacing();
                            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Reason: %s", g_validation.error.c_str());
                        }
                    }
                }

                ImGui::SetCursorPos(ImVec2(14, contentHeight - 110));
                ImGui::Separator();
                ImGui::SetCursorPos(ImVec2(14, contentHeight - 96));

                bool canInject = (g_selectedProcessId > 0 && g_validation.valid && strlen(g_dllPath) > 0);

                if (canInject)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(145 / 255.f, 25 / 255.f, 195 / 255.f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(175 / 255.f, 35 / 255.f, 230 / 255.f, 0.95f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(195 / 255.f, 45 / 255.f, 255 / 255.f, 1.00f));
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(35 / 255.f, 32 / 255.f, 45 / 255.f, 0.60f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(35 / 255.f, 32 / 255.f, 45 / 255.f, 0.60f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(35 / 255.f, 32 / 255.f, 45 / 255.f, 0.60f));
                }

                if (ImGui::Button("INJECT DLL", ImVec2(col2Width - 28, 34)))
                {
                    if (canInject)
                    {
                        std::string error;
                        if (viibe::ProcessManager::InjectDLL(static_cast<DWORD>(g_selectedProcessId), g_dllPath, error))
                        {
                            g_injectSuccess = true;
                            g_injectMessage = "DLL injected successfully into " + g_selectedProcessName + " (PID: " + std::to_string(g_selectedProcessId) + ")!";
                            viibe::Logger::Info(g_injectMessage);
                        }
                        else
                        {
                            g_injectSuccess = false;
                            g_injectMessage = error;
                            viibe::Logger::Error("Injection failed: " + error);
                        }
                    }
                    else
                    {
                        if (g_selectedProcessId <= 0)
                        {
                            g_injectSuccess = false;
                            g_injectMessage = "Please select a target process first.";
                        }
                        else if (!g_validation.valid)
                        {
                            g_injectSuccess = false;
                            g_injectMessage = "Please select a valid DLL file first.";
                        }
                    }
                }
                ImGui::PopStyleColor(3);

                if (!g_injectMessage.empty())
                {
                    ImGui::SetCursorPos(ImVec2(14, contentHeight - 56));
                    if (g_injectSuccess)
                    {
                        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "%s", g_injectMessage.c_str());
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", g_injectMessage.c_str());
                    }
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}

namespace viibe
{
    bool MainWindow::Initialize()
    {
        if (!CreateWindowAndContext())
            return false;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        SetupImGuiStyle();

        ImGui_ImplWin32_Init(g_hwnd);
        ImGui_ImplOpenGL2_Init();

        RefreshProcesses();

        Logger::Info("MainWindow initialized with modern UI and particles.");
        return true;
    }

    int MainWindow::Run()
    {
        MSG msg{};

        while (g_running)
        {
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    g_running = false;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            ImGui_ImplOpenGL2_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            RenderUI();

            ImGui::Render();

            glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
            glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            SwapBuffers(g_dc);

            Sleep(10);
        }

        return 0;
    }

    void MainWindow::Shutdown()
    {
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplWin32_Shutdown();

        ImGui::DestroyContext();

        if (g_glrc)
        {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(g_glrc);
            g_glrc = nullptr;
        }

        if (g_hwnd && g_dc)
        {
            ReleaseDC(g_hwnd, g_dc);
            g_dc = nullptr;
        }

        if (g_hwnd)
        {
            DestroyWindow(g_hwnd);
            g_hwnd = nullptr;
        }
    }
}
