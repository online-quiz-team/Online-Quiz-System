CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Isrc
LDLIBS = -lssl -lcrypto
NETLIBS =

TARGET = registration_app
SERVER = quiz_server
SOURCES = src/main.cpp src/registration.cpp src/authentication.cpp src/quiz.cpp src/quiz_submission.cpp src/quiz_scoring.cpp

# Platform differences
ifeq ($(OS),Windows_NT)
    NETLIBS = -lws2_32
else ifeq ($(shell uname -s),Darwin)
    OPENSSL_PREFIX := $(shell brew --prefix openssl 2>/dev/null)
    ifneq ($(OPENSSL_PREFIX),)
        CXXFLAGS += -I$(OPENSSL_PREFIX)/include
        LDLIBS := -L$(OPENSSL_PREFIX)/lib $(LDLIBS)
    endif
endif

all: $(TARGET) $(SERVER)

$(TARGET): $(SOURCES) src/net_compat.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS) $(NETLIBS)

$(SERVER): src/quiz_server.cpp src/net_compat.h
	$(CXX) $(CXXFLAGS) src/quiz_server.cpp -o $(SERVER) $(NETLIBS)

test:
	$(CXX) $(CXXFLAGS) tests/registration_test.cpp src/registration.cpp -o registration_test $(LDLIBS)
	./registration_test
	$(CXX) $(CXXFLAGS) tests/authentication_test.cpp src/authentication.cpp src/registration.cpp -o authentication_test $(LDLIBS)
	./authentication_test
	$(CXX) $(CXXFLAGS) tests/quiz_test.cpp src/quiz.cpp -o quiz_test
	./quiz_test
	$(CXX) $(CXXFLAGS) tests/quiz_scoring_test.cpp src/quiz_scoring.cpp -o quiz_scoring_test
	./quiz_scoring_test
	$(CXX) $(CXXFLAGS) tests/feature_13_14_test.cpp src/registration.cpp src/authentication.cpp src/quiz.cpp src/quiz_submission.cpp src/quiz_scoring.cpp -o feature_13_14_test $(LDLIBS) $(NETLIBS)
	./feature_13_14_test
	rm -f registration_test authentication_test quiz_test quiz_scoring_test feature_13_14_test

clean:
	rm -f $(TARGET) $(SERVER) registration_test authentication_test quiz_test quiz_scoring_test feature_13_14_test

.PHONY: all test clean
