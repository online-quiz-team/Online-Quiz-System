#include <gtest/gtest.h>
#include "security_log.h"
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

// Helper to read the test log file
std::vector<std::string> readLines(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) lines.push_back(line);
    return lines;
}

TEST(SecurityLogTest, LogsFailedLogin) {
    std::string testLog = "test_gtest_security.log";
    std::remove(testLog.c_str());
    securitylog::setLogFile(testLog);

    EXPECT_TRUE(securitylog::logFailedLogin("testuser"));
    
    auto lines = readLines(testLog);
    ASSERT_EQ(lines.size(), 1);
    EXPECT_NE(lines[0].find("FAILED_LOGIN"), std::string::npos);
    EXPECT_NE(lines[0].find("user=testuser"), std::string::npos);
    
    std::remove(testLog.c_str());
}

TEST(SecurityLogTest, LogsAdminAction) {
    std::string testLog = "test_gtest_security.log";
    std::remove(testLog.c_str());
    securitylog::setLogFile(testLog);

    EXPECT_TRUE(securitylog::logAdminAction("admin1", "DELETE_QUESTION", "quiz=1 q=5"));
    
    auto lines = readLines(testLog);
    ASSERT_EQ(lines.size(), 1);
    EXPECT_NE(lines[0].find("ADMIN_ACTION"), std::string::npos);
    EXPECT_NE(lines[0].find("user=admin1"), std::string::npos);
    
    std::remove(testLog.c_str());
}

TEST(SecurityLogTest, HandlesLogInjection) {
    std::string testLog = "test_gtest_security.log";
    std::remove(testLog.c_str());
    securitylog::setLogFile(testLog);

    EXPECT_TRUE(securitylog::logFailedLogin("bob\n2026-01-01T00:00:00Z | ADMIN_ACTION | user=root"));
    
    auto lines = readLines(testLog);
    ASSERT_EQ(lines.size(), 1); // The newline should be sanitized
    
    std::remove(testLog.c_str());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}