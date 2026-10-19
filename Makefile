CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Isrc
LDLIBS = -lssl -lcrypto

TARGET = registration_app
SOURCES = src/main.cpp src/registration.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET) students.txt

test:
	$(CXX) $(CXXFLAGS) tests/registration_test.cpp src/registration.cpp -o registration_test $(LDLIBS)
	./registration_test
	rm -f registration_test

.PHONY: all clean test
