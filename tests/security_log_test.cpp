#include "security_log.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
const char* TEST_LOG = "test_security.log";

std::vector<std::string> readLines() {
    std::ifstream file(TEST_LOG);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) lines.push_back(line);
    return lines;
}

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}
}  // namespace

int main() {
    std::remove(TEST_LOG);
    securitylog::setLogFile(TEST_LOG);

    // Failed login is logged with timestamp, event name and username.
    assert(securitylog::logFailedLogin("alice"));
    auto lines = readLines();
    assert(lines.size() == 1);
    assert(contains(lines[0], "FAILED_LOGIN"));
    assert(contains(lines[0], "user=alice"));
    // ISO-8601 UTC timestamp: YYYY-MM-DDTHH:MM:SSZ
    assert(lines[0].size() > 20 && lines[0][4] == '-' && lines[0][10] == 'T' &&
           lines[0][19] == 'Z');

    // Admin action is logged with who/what.
    assert(securitylog::logAdminAction("admin1", "DELETE_QUESTION", "quiz=1 q=5"));
    lines = readLines();
    assert(lines.size() == 2);
    assert(contains(lines[1], "ADMIN_ACTION"));
    assert(contains(lines[1], "user=admin1"));
    assert(contains(lines[1], "DELETE_QUESTION: quiz=1 q=5"));

    // Log injection: newlines must not create extra log lines.
    assert(securitylog::logFailedLogin("bob\n2026-01-01T00:00:00Z | ADMIN_ACTION | user=root"));
    assert(securitylog::logFailedLogin("carol\r\nFAKE"));
    lines = readLines();
    assert(lines.size() == 4);
    assert(!contains(lines[2], "\n") && !contains(lines[2], "\r"));
    // The forged text stays inside the username field: a genuine entry has
    // exactly three " | " separators (time | event | user | detail), no more.
    auto separators = [](const std::string& l) {
        std::size_t n = 0, pos = 0;
        while ((pos = l.find(" | ", pos)) != std::string::npos) { ++n; pos += 3; }
        return n;
    };
    assert(separators(lines[2]) == 3);
    assert(lines[2].find("FAILED_LOGIN") != std::string::npos);

    // Overlong input is truncated.
    assert(securitylog::logFailedLogin(std::string(5000, 'x')));
    lines = readLines();
    assert(lines.size() == 5);
    assert(lines[4].size() < 400);

    // Appends across calls (existing entries are never overwritten).
    assert(readLines()[0].find("user=alice") != std::string::npos);

    // An unwritable log location returns false and does not crash.
    securitylog::setLogFile("no_such_directory_xyz/security.log");
    assert(!securitylog::logFailedLogin("dave"));

    std::remove(TEST_LOG);
    std::cout << "All security log tests passed.\n";
    return 0;
}
