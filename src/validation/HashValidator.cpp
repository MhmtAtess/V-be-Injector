#include "validation/HashValidator.hpp"

#include <Windows.h>
#include <wincrypt.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace viibe
{
    std::string HashValidator::SHA256(
        const std::string& path
    )
    {
        HCRYPTPROV provider = 0;
        HCRYPTHASH hash = 0;

        if (!CryptAcquireContextA(
                &provider,
                nullptr,
                nullptr,
                PROV_RSA_AES,
                CRYPT_VERIFYCONTEXT))
        {
            return {};
        }

        if (!CryptCreateHash(
                provider,
                CALG_SHA_256,
                0,
                0,
                &hash))
        {
            CryptReleaseContext(
                provider,
                0
            );

            return {};
        }

        std::ifstream file(
            path,
            std::ios::binary
        );

        if (!file)
        {
            CryptDestroyHash(hash);
            CryptReleaseContext(provider, 0);

            return {};
        }

        std::vector<char> buffer(1024 * 1024);

        while (file)
        {
            file.read(
                buffer.data(),
                buffer.size()
            );

            const auto read =
                file.gcount();

            if (read > 0)
            {
                if (!CryptHashData(
                        hash,
                        reinterpret_cast<
                            const BYTE*
                        >(buffer.data()),
                        static_cast<DWORD>(read),
                        0))
                {
                    CryptDestroyHash(hash);
                    CryptReleaseContext(
                        provider,
                        0
                    );

                    return {};
                }
            }
        }

        DWORD hashSize = 0;
        DWORD hashSizeLength =
            sizeof(hashSize);

        CryptGetHashParam(
            hash,
            HP_HASHSIZE,
            reinterpret_cast<BYTE*>(&hashSize),
            &hashSizeLength,
            0
        );

        std::vector<BYTE> digest(hashSize);

        if (!CryptGetHashParam(
                hash,
                HP_HASHVAL,
                digest.data(),
                &hashSize,
                0))
        {
            CryptDestroyHash(hash);
            CryptReleaseContext(provider, 0);

            return {};
        }

        std::ostringstream output;

        output << std::hex
               << std::setfill('0');

        for (BYTE byte : digest)
        {
            output
                << std::setw(2)
                << static_cast<int>(byte);
        }

        CryptDestroyHash(hash);
        CryptReleaseContext(provider, 0);

        return output.str();
    }
}
