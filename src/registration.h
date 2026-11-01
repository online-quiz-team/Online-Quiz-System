
#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <string>

class Registration {
private:
    std::string storageFile;

public:
    explicit Registration(const std::string& filename);

    // Generic account registration.
    bool registerAccount(const std::string& username,
                         const std::string& password);

    // Existing student registration.
    bool registerStudent(const std::string& username,
                         const std::string& password);

    bool usernameExists(const std::string& username);
};

#endif
