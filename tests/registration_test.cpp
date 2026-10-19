#include "registration.h"

#include <cassert>
#include <cstdio>
#include <iostream>

int main() {
    const char* testFile = "test_students.txt";
    std::remove(testFile);

    Registration registration(testFile);

    assert(registration.registerStudent("alice", "Password123!"));
    assert(registration.usernameExists("alice"));

    assert(!registration.registerStudent("alice", "AnotherPassword123!"));
    assert(!registration.registerStudent("", "Password123!"));
    assert(!registration.registerStudent("bob", ""));

    Registration secondInstance(testFile);
    assert(secondInstance.usernameExists("alice"));
    assert(!secondInstance.registerStudent("alice", "Password123!"));

    std::remove(testFile);

    std::cout << "All registration tests passed.\n";
    return 0;
}
