cmake_minimum_required(VERSION 3.20)

project(VIIBE_Loader
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_DIR
    ${CMAKE_SOURCE_DIR}/external/imgui
)

add_executable(VIIBE_Loader WIN32
    src/main.cpp

    src/app/Application.cpp
    src/app/Logger.cpp

    src/process/ProcessManager.cpp

    src/validation/PEValidator.cpp
    src/validation/HashValidator.cpp

    src/ui/MainWindow.cpp

    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp

    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp
)

target_include_directories(VIIBE_Loader PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(VIIBE_Loader PRIVATE
    opengl32
    user32
    gdi32
    crypt32
    advapi32
)

if(MSVC)
    target_compile_options(VIIBE_Loader PRIVATE
        /W4
        /permissive-
    )
endif()
