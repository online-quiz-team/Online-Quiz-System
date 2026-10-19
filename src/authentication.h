#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <string>

class Authentication {
public:
    explicit Authentication(const std::string& filename);

    bool login(const std::string& username,
               const std::string& password);

private:
    std::string storageFile;
};

#endif
