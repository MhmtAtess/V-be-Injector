# VIIBE Loader

[ English ] | [ Türkçe ]

---

## 🇬🇧 English

A sleek, modern, high-performance DLL Injector and Portable Executable (PE) Loader built with C++20 and a customized ImGui interface.

### Features

- **Modern & Dynamic UI**: Custom dark-themed ImGui user interface featuring animated particle background effects and dynamic rainbow titles.
- **Process Manager & Filtering**: Real-time enumeration of running system processes with instant search by process name or PID.
- **PE Validation Engine**: Built-in PE header parser that verifies target DLL integrity, checking architecture compatibility (x64/x86/ARM64), image size, and entry point before injection.
- **Cryptographic Hashing**: Automatic SHA-256 checksum calculation for DLL verification with one-click copy to clipboard.
- **Standard Injection**: Clean Windows API implementation using `VirtualAllocEx`, `WriteProcessMemory`, and `CreateRemoteThread`.

### System Requirements

- **OS**: Windows 10 / 11 (x64)
- **Compiler**: Visual Studio 2022 / MSVC with C++20 standard support
- **Build System**: CMake 3.20 or higher

### Building from Source

1. Clone the repository along with submodules:
   ```bash
   git clone --recursive https://github.com/your-username/VIIBE-Loader.git
   cd VIIBE-Loader
   ```

2. Generate build files:
   ```bash
   cmake -B build -S .
   ```

3. Build the project in Release mode:
   ```bash
   cmake --build build --config Release
   ```

   The compiled binary will be available in `build/Release/VIIBE_Loader.exe`.

### How to Use

1. Launch `VIIBE_Loader.exe`.
2. **Select Process**: Locate your target process from the left panel or filter using the search bar (e.g. `javaw`, `game`).
3. **Select DLL**: Click **Browse** in the right panel and choose your target `.dll` file.
4. **Inspect Validation**: Check the Validation Report to confirm DLL architecture, size, and SHA-256 hash.
5. **Inject**: Click **INJECT DLL** to execute the payload.

### Project Structure

```
VIIBE-Loader/
├── assets/          # Icons and visual assets
├── external/        # Third-party dependencies (ImGui)
├── src/             # Source code & Headers
│   ├── app/         # Application bootstrap & logging system
│   ├── process/     # Process enumeration & DLL injection engine
│   ├── ui/          # ImGui interface & rendering loop
│   └── validation/  # PE header validator & SHA-256 calculation
├── CMakeLists.txt   # CMake configuration
└── README.md        # Project documentation
```

### Disclaimer

This tool is created strictly for educational purposes, software security research, and personal debugging. The author assumes no responsibility for any misuse or violation of software terms.

---

## 🇹🇷 Türkçe

C++20 ve özel olarak özelleştirilmiş ImGui arayüzü ile geliştirilmiş, şık, modern ve yüksek performanslı DLL Enjektörü ve Portable Executable (PE) Doğrulayıcı.

### Özellikler

- **Modern ve Dinamik Arayüz**: Animasyonlu parçacık arka plan efektleri ve dinamik gökkuşağı başlığı içeren özel karanlık tema ImGui arayüzü.
- **Süreç Yöneticisi ve Filtreleme**: Çalışan sistem süreçlerinin anlık olarak listelenmesi ve işlem adı ya da PID'ye göre hızlı arama.
- **PE Doğrulama Motoru**: Hedef DLL dosyasının mimarisini (x64/x86/ARM64), imaj boyutunu ve giriş noktasını enjeksiyon öncesi kontrol eden yerleşik PE başlık ayrıştırıcısı.
- **Kriptografik Hashing**: DLL doğrulaması için otomatik SHA-256 özeti hesaplama ve tek tıkla panoya kopyalama özelliği.
- **Standart Enjeksiyon**: `VirtualAllocEx`, `WriteProcessMemory` ve `CreateRemoteThread` kullanılarak hazırlanmış güvenli Windows API enjeksiyon altyapısı.

### Sistem Gereksinimleri

- **İşletim Sistemi**: Windows 10 / 11 (x64)
- **Derleyici**: Visual Studio 2022 / MSVC (C++20 standardı gereklidir)
- **Derleme Sistemi**: CMake 3.20 veya üzeri

### Kaynak Koddan Derleme

1. Depoyu alt modülleriyle birlikte klonlayın:
   ```bash
   git clone --recursive https://github.com/kullanici-adiniz/VIIBE-Loader.git
   cd VIIBE-Loader
   ```

2. Derleme dosyalarını oluşturun:
   ```bash
   cmake -B build -S .
   ```

3. Projeyi Release modunda derleyin:
   ```bash
   cmake --build build --config Release
   ```

   Derlenen çalıştırılabilir dosya `build/Release/VIIBE_Loader.exe` konumunda yer alacaktır.

### Nasıl Kullanılır?

1. `VIIBE_Loader.exe` dosyasını çalıştırın.
2. **Süreç Seçimi**: Sol paneldeki listeden hedef süreci seçin veya arama kutusunu kullanarak filtreleyin (ör. `javaw`, `game`).
3. **DLL Seçimi**: Sağ paneldeki **Browse** butonuna tıklayarak hedef `.dll` dosyanızı seçin.
4. **Doğrulama İncelemesi**: Doğrulama Raporu (Validation Report) alanından DLL mimarisini, boyutunu ve SHA-256 özetini kontrol edin.
5. **Enjekte Et**: Enjeksiyonu gerçekleştirmek için **INJECT DLL** butonuna tıklayın.

### Proje Yapısı

```
VIIBE-Loader/
├── assets/          # İkonlar ve görsel varlıklar
├── external/        # Üçüncü taraf kütüphaneler (ImGui)
├── src/             # Kaynak Kodlar ve Başlık Dosyaları
│   ├── app/         # Uygulama başlatıcı ve loglama sistemi
│   ├── process/     # Süreç listeleme ve DLL enjeksiyon motoru
│   ├── ui/          # ImGui arayüzü ve render döngüsü
│   └── validation/  # PE başlık doğrulayıcı ve SHA-256 hesaplayıcı
├── CMakeLists.txt   # CMake yapılandırma dosyası
└── README.md        # Proje dokümantasyonu
```

### Sorumluluk Reddi

Bu araç yalnızca eğitim, yazılım güvenliği araştırmaları ve kişisel hata ayıklama (debugging) amacıyla geliştirilmiştir. Kötüye kullanım veya üçüncü taraf yazılım şartlarının ihlali durumunda sorumluluk kullanıcıya aittir.
