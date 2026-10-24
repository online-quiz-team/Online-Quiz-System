CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Isrc
LDLIBS = -lssl -lcrypto

TARGET = registration_app
SOURCES = src/main.cpp src/registration.cpp src/authentication.cpp src/quiz.cpp src/quiz_submission.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)

test:
	$(CXX) $(CXXFLAGS) tests/registration_test.cpp src/registration.cpp -o registration_test $(LDLIBS)
	./registration_test
	$(CXX) $(CXXFLAGS) tests/authentication_test.cpp src/authentication.cpp src/registration.cpp -o authentication_test $(LDLIBS)
	./authentication_test
	$(CXX) $(CXXFLAGS) tests/quiz_test.cpp src/quiz.cpp -o quiz_test
	./quiz_test
	rm -f registration_test authentication_test quiz_test

clean:
	rm -f $(TARGET) registration_test authentication_test quiz_test

.PHONY: all test clean
