# Makefile for MessageU project

# Library paths for macOS with Apple Silicon Homebrew
BOOST_INC = -I/opt/homebrew/opt/boost/include
BOOST_LIB = -L/opt/homebrew/opt/boost/lib
CRYPTOPP_INC = -I/opt/homebrew/opt/cryptopp/include
CRYPTOPP_LIB = -L/opt/homebrew/opt/cryptopp/lib
LIBS = -lboost_system -lcryptopp

# Compiler settings
CXX = clang++
CXXFLAGS = -std=c++11 -Wall -Wextra -g
INCLUDES = -I./src/client $(BOOST_INC) $(CRYPTOPP_INC)

# Directories
CLIENT_DIR = src/client
SERVER_DIR = src/server

# Source files
CLIENT_SOURCES = $(CLIENT_DIR)/main.cpp \
                 $(CLIENT_DIR)/MessageUClient.cpp \
                 $(CLIENT_DIR)/ClientNetwork.cpp \
                 $(CLIENT_DIR)/ClientCrypto.cpp \
                 $(CLIENT_DIR)/ProtocolHandler.cpp

# Test files
CLIENT_TEST_SOURCES = $(CLIENT_DIR)/tests/test_protocol_handler.cpp \
                      $(CLIENT_DIR)/ProtocolHandler.cpp

# Targets
all: client server

client: $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o messageu_client $(CLIENT_SOURCES) $(BOOST_LIB) $(CRYPTOPP_LIB) $(LIBS)

test_client: $(CLIENT_TEST_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o test_protocol_handler $(CLIENT_TEST_SOURCES) $(BOOST_LIB) $(CRYPTOPP_LIB) $(LIBS)
	./test_protocol_handler

test_server:
	cd $(SERVER_DIR) && python3 tests/test_protocol_handler.py

test: test_client test_server
	@echo "All tests completed!"

server:
	@echo "Server is implemented in Python. Run with: cd $(SERVER_DIR) && python3 main.py"

clean:
	rm -f messageu_client test_protocol_handler
	rm -rf __pycache__ src/server/__pycache__ src/server/tests/__pycache__

.PHONY: all client server test test_client test_server clean 