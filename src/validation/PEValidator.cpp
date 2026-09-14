#include "validation/PEValidator.hpp"

#include <Windows.h>

#include <fstream>
#include <vector>

namespace viibe
{
    PEValidationResult
    PEValidator::Validate(
        const std::string& path
    )
    {
        PEValidationResult result;

        std::ifstream file(
            path,
            std::ios::binary
        );

        if (!file)
        {
            result.error =
                "File could not be opened.";

            return result;
        }

        file.seekg(
            0,
            std::ios::end
        );

        const auto size =
            static_cast<size_t>(
                file.tellg()
            );

        if (size < sizeof(IMAGE_DOS_HEADER))
        {
            result.error =
                "File is too small.";

            return result;
        }

        file.seekg(0);

        std::vector<char> buffer(size);

        file.read(
            buffer.data(),
            static_cast<std::streamsize>(size)
        );

        auto* dos =
            reinterpret_cast<
                IMAGE_DOS_HEADER*
            >(buffer.data());

        if (dos->e_magic != IMAGE_DOS_SIGNATURE)
        {
            result.error =
                "Invalid DOS signature.";

            return result;
        }

        if (
            dos->e_lfanew < 0 ||
            static_cast<size_t>(dos->e_lfanew) >= size
        )
        {
            result.error =
                "Invalid PE offset.";

            return result;
        }

        auto* nt =
            reinterpret_cast<
                IMAGE_NT_HEADERS*
            >(buffer.data() + dos->e_lfanew);

        if (nt->Signature != IMAGE_NT_SIGNATURE)
        {
            result.error =
                "Invalid NT signature.";

            return result;
        }

        result.isDll =
            (nt->FileHeader.Characteristics &
             IMAGE_FILE_DLL) != 0;

        if (!result.isDll)
        {
            result.error =
                "File is not a DLL.";

            return result;
        }

        switch (nt->FileHeader.Machine)
        {
        case IMAGE_FILE_MACHINE_AMD64:
            result.architecture = "x64";
            break;

        case IMAGE_FILE_MACHINE_I386:
            result.architecture = "x86";
            break;

        case IMAGE_FILE_MACHINE_ARM64:
            result.architecture = "ARM64";
            break;

        default:
            result.architecture = "Unknown";
            break;
        }

#ifdef _WIN64
        if (
            nt->FileHeader.Machine !=
            IMAGE_FILE_MACHINE_AMD64
        )
        {
            result.error =
                "Architecture mismatch.";

            return result;
        }
#endif

        result.imageSize =
            nt->OptionalHeader.SizeOfImage;

        result.entryPoint =
            nt->OptionalHeader.AddressOfEntryPoint;

        result.valid = true;

        return result;
    }
}
