#include "../ProtocolHandler.h"
#include <iostream>
#include <cassert>

// Simple test function
void testProtocolHandler() {
    std::cout << "Testing C++ Protocol Handler..." << std::endl;
    
    ProtocolHandler handler;
    
    // Test registration request
    std::string username = "testuser";
    std::string public_key = "test_public_key_12345";
    
    std::vector<uint8_t> reg_request = handler.createRegistrationRequest(username, public_key);
    
    // Verify header
    ProtocolHeader header;
    bool header_ok = handler.deserializeHeader(reg_request, header);
    
    if (header_ok && header.code == ProtocolCodes::REGISTRATION_REQUEST) {
        std::cout << "✓ Registration request test passed!" << std::endl;
    } else {
        std::cout << "✗ Registration request test failed!" << std::endl;
    }
    
    // Test header serialization
    ProtocolHeader test_header;
    test_header.version = 1;
    test_header.code = 1000;
    test_header.payload_size = 100;
    test_header.payload_checksum = 12345;
    
    std::vector<uint8_t> header_bytes = handler.serializeHeader(test_header);
    
    if (header_bytes.size() == 8) {
        std::cout << "✓ Header serialization test passed!" << std::endl;
    } else {
        std::cout << "✗ Header serialization test failed!" << std::endl;
    }
    
    std::cout << "C++ Protocol Handler tests completed!" << std::endl;
}

int main() {
    testProtocolHandler();
    return 0;
} 