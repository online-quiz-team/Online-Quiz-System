#include "security_log.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace securitylog {
namespace {

constexpr std::size_t MAX_FIELD_LENGTH = 128;

std::mutex logMutex;
std::string logPath = "security.log";

// Replace control characters and the field separator, and cap the length.
std::string sanitize(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    for (unsigned char c : input) {
        if (c < 0x20 || c == 0x7f || c == '|') {
            out.push_back('?');
        } else {
            out.push_back(static_cast<char>(c));
        }
    }
    if (out.size() > MAX_FIELD_LENGTH) {
        out.resize(MAX_FIELD_LENGTH);
        out += "...";
    }
    return out;
}

// Caller must hold logMutex (std::gmtime uses a shared buffer).
std::string utcTimestamp() {
    const std::time_t now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const std::tm* tm = std::gmtime(&now);
    std::ostringstream out;
    if (tm) {
        out << std::put_time(tm, "%Y-%m-%dT%H:%M:%SZ");
    } else {
        out << "unknown-time";
    }
    return out.str();
}

}  // namespace

void setLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(logMutex);
    logPath = path;
}

bool logEvent(const std::string& event,
              const std::string& username,
              const std::string& detail) {
    try {
        std::lock_guard<std::mutex> lock(logMutex);

        std::error_code ec;
        [[maybe_unused]] const bool existedBefore =
            std::filesystem::exists(logPath, ec);

        std::ofstream file(logPath, std::ios::app);
        if (!file) {
            return false;
        }

        file << utcTimestamp() << " | " << sanitize(event)
             << " | user=" << sanitize(username);
        if (!detail.empty()) {
            file << " | " << sanitize(detail);
        }
        file << '\n';
        file.flush();
        const bool ok = static_cast<bool>(file);
        file.close();

#ifndef _WIN32
        // New log file: owner read/write only. (On Windows the default
        // folder ACL applies.)
        if (ok && !existedBefore) {
            std::filesystem::permissions(
                logPath,
                std::filesystem::perms::owner_read |
                    std::filesystem::perms::owner_write,
                std::filesystem::perm_options::replace, ec);
        }
#endif
        return ok;
    } catch (...) {
        return false;
    }
}

bool logFailedLogin(const std::string& username) {
    return logEvent("FAILED_LOGIN", username, "invalid username or password");
}

bool logAdminAction(const std::string& adminUsername,
                    const std::string& action,
                    const std::string& detail) {
    return logEvent("ADMIN_ACTION", adminUsername,
                    detail.empty() ? action : action + ": " + detail);
}

}  // namespace securitylog
