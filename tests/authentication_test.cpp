#include "authentication.h"
#include "registration.h"

#include <cassert>
#include <cstdio>
#include <iostream>

int main() {
    const char* testFile = "test_auth_students.txt";
    std::remove(testFile);

    Registration registration(testFile);
    Authentication authentication(testFile);

    assert(registration.registerStudent("alice", "SecurePass123!"));

    assert(authentication.login("alice", "SecurePass123!"));
    assert(!authentication.login("alice", "WrongPass123!"));
    assert(!authentication.login("unknown", "SecurePass123!"));
    assert(!authentication.login("", "SecurePass123!"));
    assert(!authentication.login("alice", ""));

    std::remove(testFile);

    std::cout << "All authentication tests passed.\n";
    return 0;
}
