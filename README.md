# 🛡️ VIIBE Loader - Modern DLL Injector & PE Loader

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![ImGui](https://img.shields.io/badge/ImGui-Custom--Theme-009688.svg)](https://github.com/ocornut/imgui)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11%20(x64)-lightgrey.svg)]()
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-informational.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> **[ 🇬🇧 English ](#-english)** | **[ 🇹🇷 Türkçe ](#-türkçe)**

---

# 🇬🇧 English

> **Sleek, Modern & High-Performance C++20 DLL Injector & PE Loader**
> 
> A lightweight, feature-packed Windows utility featuring a customized ImGui interface, real-time process manager, PE header validation engine, and SHA-256 cryptographic verification.

---

## 📋 Table of Contents
- [Features](#-features)
- [System Requirements](#-system-requirements)
- [Building from Source](#-building-from-source)
- [How to Use](#-how-to-use)
- [Project Structure](#-project-structure)
- [Disclaimer & License](#-disclaimer--license)

---

## ✨ Features

- **🎨 Modern & Dynamic UI**: Custom dark-themed ImGui interface with animated particle background effects and dynamic rainbow title rendering.
- **🔍 Process Manager & Filtering**: Real-time enumeration of running system processes with instant search filtering by process name or PID.
- **🛡️ PE Validation Engine**: Built-in Portable Executable (PE) header parser that verifies DLL integrity, architecture compatibility (x64 / x86 / ARM64), image size, and entry point before injection.
- **🔒 Cryptographic Hashing**: Automated SHA-256 checksum calculation for target DLL verification with one-click clipboard copying.
- **⚡ Standard Injection Engine**: Clean, robust Windows API implementation leveraging `VirtualAllocEx`, `WriteProcessMemory`, and `CreateRemoteThread`.

---

## 💻 System Requirements

| Requirement | Specification |
| :--- | :--- |
| **Operating System** | Windows 10 / 11 (64-bit) |
| **Compiler** | Visual Studio 2022 / MSVC (C++20 standard required) |
| **Build System** | CMake 3.20 or higher |

---

## 🛠️ Building from Source

### 1. Clone Repository with Submodules
```bash
git clone --recursive https://github.com/your-username/VIIBE-Loader.git
cd VIIBE-Loader
