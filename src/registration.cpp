
#include "registration.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <limits>

namespace {

constexpr int SALT_SIZE = 16;
constexpr int HASH_SIZE = 32;
constexpr int ITERATIONS = 600000;

// Convert binary data into hexadecimal representation.
std::string toHex(const unsigned char* data, int length) {
    std::ostringstream result;
    result << std::hex << std::setfill('0');

    for (int i = 0; i < length; ++i) {
        result << std::setw(2)
               << static_cast<int>(data[i]);
    }

    return result.str();
}

// Generate a salted PBKDF2 password hash.
bool hashPassword(const std::string& password,
                  std::string& saltHex,
                  std::string& hashHex) {

    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];

    if (password.size() >
        static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return false;
    }

    // Generate a cryptographically secure random salt.
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        return false;
    }

    // Derive the password hash using PBKDF2-HMAC-SHA256.
    if (PKCS5_PBKDF2_HMAC(
            password.c_str(),
            static_cast<int>(password.size()),
            salt,
            sizeof(salt),
            ITERATIONS,
            EVP_sha256(),
            HASH_SIZE,
            hash) != 1) {

        OPENSSL_cleanse(hash, sizeof(hash));
        return false;
    }

    saltHex = toHex(salt, SALT_SIZE);
    hashHex = toHex(hash, HASH_SIZE);

    // Clear sensitive binary data from memory.
    OPENSSL_cleanse(hash, sizeof(hash));
    OPENSSL_cleanse(salt, sizeof(salt));

    return true;
}

}

// Constructor.
Registration::Registration(const std::string& filename)
    : storageFile(filename) {}

// Check whether an account username already exists.
bool Registration::usernameExists(const std::string& username) {
    std::ifstream file(storageFile);

    std::string line;
    std::string storedUsername;

    while (std::getline(file, line)) {
        std::istringstream stream(line);

        if (std::getline(stream, storedUsername, ':') &&
            storedUsername == username) {
            return true;
        }
    }

    return false;
}

// Generic registration for students and administrators.
bool Registration::registerAccount(const std::string& username,
                                   const std::string& password) {

    // Validate input.
    if (username.empty() || password.empty() ||
        username.find(':') != std::string::npos ||
        username.find('\n') != std::string::npos ||
        username.find('\r') != std::string::npos ||
        password.find('\n') != std::string::npos ||
        password.find('\r') != std::string::npos ||
        usernameExists(username)) {

        return false;
    }

    std::string saltHex;
    std::string hashHex;

    // Generate password hash and salt.
    if (!hashPassword(password, saltHex, hashHex)) {
        return false;
    }

    // Store the account in the configured file.
    std::ofstream file(storageFile, std::ios::app);

    if (!file.is_open()) {
        return false;
    }

    file << username << ':'
         << saltHex << ':'
         << hashHex << '\n';

    return file.good();
}

// Existing student registration delegates to generic registration.
bool Registration::registerStudent(const std::string& username,
                                   const std::string& password) {

    return registerAccount(username, password);
}
