# MessageU Client Makefile
# Builds the C++ client with Boost.Asio and Crypto++

# Library paths for macOS Apple Silicon (Homebrew)
BOOST_INC = -I/opt/homebrew/opt/boost/include
BOOST_LIB = -L/opt/homebrew/opt/boost/lib
CRYPTOPP_INC = -I/opt/homebrew/opt/cryptopp/include
CRYPTOPP_LIB = -L/opt/homebrew/opt/cryptopp/lib
LIBS = -lboost_system -lcryptopp

# Compiler settings
CXX = clang++
CXXFLAGS = -std=c++11 -Wall -Wextra -g
INCLUDES = -I./src/client $(BOOST_INC) $(CRYPTOPP_INC)
LDFLAGS = $(BOOST_LIB) $(CRYPTOPP_LIB) $(LIBS)

# Source files
CLIENT_SOURCES = src/client/main.cpp \
                 src/client/MessageUClient.cpp \
                 src/client/ClientNetwork.cpp \
                 src/client/ClientCrypto.cpp \
                 src/client/ProtocolHandler.cpp

# Targets
all: client

client: $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o messageu_client $(CLIENT_SOURCES) $(LDFLAGS)

clean:
	rm -f messageu_client
	rm -rf *.dSYM

.PHONY: all client clean 