#include "authentication.h"

#include <openssl/evp.h>
#include <openssl/crypto.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace {
constexpr int SALT_SIZE = 16;
constexpr int HASH_SIZE = 32;
constexpr int ITERATIONS = 600000;

bool fromHex(const std::string& hex, unsigned char* output,
             std::size_t outputSize) {
    if (hex.size() != outputSize * 2) {
        return false;
    }

    for (std::size_t i = 0; i < outputSize; ++i) {
        unsigned int value;
        std::istringstream stream(hex.substr(i * 2, 2));

        stream >> std::hex >> value;
        if (stream.fail()) {
            return false;
        }

        output[i] = static_cast<unsigned char>(value);
    }

    return true;
}
}

Authentication::Authentication(const std::string& filename)
    : storageFile(filename) {}

bool Authentication::login(const std::string& username,
                           const std::string& password) {
    if (username.empty() || password.empty()) {
        return false;
    }

    std::ifstream file(storageFile);
    if (!file) {
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string storedUsername;
        std::string saltHex;
        std::string hashHex;
        std::string extra;

        if (!std::getline(stream, storedUsername, ':') ||
            !std::getline(stream, saltHex, ':') ||
            !std::getline(stream, hashHex, ':')) {
            continue;
        }

        if (std::getline(stream, extra, ':')) {
            continue;
        }

        if (storedUsername != username) {
            continue;
        }

        unsigned char salt[SALT_SIZE];
        unsigned char expectedHash[HASH_SIZE];
        unsigned char actualHash[HASH_SIZE];

        if (!fromHex(saltHex, salt, SALT_SIZE) ||
            !fromHex(hashHex, expectedHash, HASH_SIZE)) {
            return false;
        }

        const bool hashSuccess =
            PKCS5_PBKDF2_HMAC(
                password.c_str(),
                static_cast<int>(password.size()),
                salt,
                SALT_SIZE,
                ITERATIONS,
                EVP_sha256(),
                HASH_SIZE,
                actualHash) == 1;

        const bool matches =
            hashSuccess &&
            CRYPTO_memcmp(actualHash, expectedHash, HASH_SIZE) == 0;

        OPENSSL_cleanse(actualHash, sizeof(actualHash));
        OPENSSL_cleanse(expectedHash, sizeof(expectedHash));

        return matches;
    }

    return false;
}
