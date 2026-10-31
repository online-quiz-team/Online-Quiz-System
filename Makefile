CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Isrc
LDLIBS = -lssl -lcrypto
NETLIBS =
STATICRT =
EXE =
RUN = ./
RM_CMD = rm -f
RM_SUFFIX =

TARGET = registration_app
SERVER = quiz_server
SOURCES = src/main.cpp src/registration.cpp src/authentication.cpp src/quiz.cpp src/quiz_submission.cpp src/quiz_scoring.cpp
TEST_BINS = registration_test$(EXE) authentication_test$(EXE) quiz_test$(EXE) quiz_scoring_test$(EXE) feature_13_14_test$(EXE)

# ---- Platform differences ----
ifeq ($(OS),Windows_NT)
    EXE = .exe
    NETLIBS = -lws2_32
    STATICRT = -static-libgcc -static-libstdc++
    # OpenSSL location. Override with: make OPENSSL_DIR=C:/path/to/openssl
    # (the folder that contains include/ and lib/). Not needed when building
    # inside an MSYS2 shell that already has the openssl package installed.
    OPENSSL_DIR ?= C:/msys64/ucrt64
    ifneq ($(wildcard $(OPENSSL_DIR)/include/openssl/evp.h),)
        CXXFLAGS += -I$(OPENSSL_DIR)/include
        LDLIBS := -L$(OPENSSL_DIR)/lib $(LDLIBS)
    endif
    # Without an sh shell (plain cmd.exe / PowerShell), use cmd built-ins.
    ifneq ($(findstring cmd,$(notdir $(SHELL))),)
        RUN =
        RM_CMD = -del /Q /F
        RM_SUFFIX = 2>nul
    endif
else ifeq ($(shell uname -s),Darwin)
    OPENSSL_PREFIX := $(shell brew --prefix openssl 2>/dev/null)
    ifneq ($(OPENSSL_PREFIX),)
        CXXFLAGS += -I$(OPENSSL_PREFIX)/include
        LDLIBS := -L$(OPENSSL_PREFIX)/lib $(LDLIBS)
    endif
endif

all: $(TARGET)$(EXE) $(SERVER)$(EXE)

$(TARGET)$(EXE): $(SOURCES) src/net_compat.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@ $(LDLIBS) $(NETLIBS) $(STATICRT)

$(SERVER)$(EXE): src/quiz_server.cpp src/net_compat.h
	$(CXX) $(CXXFLAGS) src/quiz_server.cpp -o $@ $(NETLIBS) $(STATICRT)

# $(call run_test,name,sources,libs)
define run_test
	$(CXX) $(CXXFLAGS) $(2) -o $(1)$(EXE) $(3) $(STATICRT)
	$(RUN)$(1)$(EXE)
endef

test:
	$(call run_test,registration_test,tests/registration_test.cpp src/registration.cpp,$(LDLIBS))
	$(call run_test,authentication_test,tests/authentication_test.cpp src/authentication.cpp src/registration.cpp,$(LDLIBS))
	$(call run_test,quiz_test,tests/quiz_test.cpp src/quiz.cpp,)
	$(call run_test,quiz_scoring_test,tests/quiz_scoring_test.cpp src/quiz_scoring.cpp,)
	$(call run_test,feature_13_14_test,tests/feature_13_14_test.cpp src/registration.cpp src/authentication.cpp src/quiz.cpp src/quiz_submission.cpp src/quiz_scoring.cpp,$(LDLIBS) $(NETLIBS))
	$(RM_CMD) $(TEST_BINS) $(RM_SUFFIX)

clean:
	$(RM_CMD) $(TARGET)$(EXE) $(SERVER)$(EXE) $(TEST_BINS) $(RM_SUFFIX)

.PHONY: all test clean
