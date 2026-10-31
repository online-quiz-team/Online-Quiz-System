#ifndef SECURITY_LOG_H
#define SECURITY_LOG_H

#include <string>

// Append-only, timestamped (UTC) security event log.
//
// Line format:  <ISO-8601 UTC> | <EVENT> | user=<name> | <detail>
//
// Safety properties:
//  - The API has no password parameter, so credentials cannot be logged.
//  - Newlines and other control characters in any field are replaced, so
//    user-supplied input cannot forge extra log lines (log injection).
//  - Fields are length-limited.
//  - A failed write never throws or crashes the application.
namespace securitylog {

// Default log file is "security.log" in the working directory.
void setLogFile(const std::string& path);

// Returns true if the event was written.
bool logEvent(const std::string& event,
              const std::string& username,
              const std::string& detail = "");

bool logFailedLogin(const std::string& username);

// For administrator functions (OQS-F-015): who did what.
bool logAdminAction(const std::string& adminUsername,
                    const std::string& action,
                    const std::string& detail = "");

}  // namespace securitylog

#endif
