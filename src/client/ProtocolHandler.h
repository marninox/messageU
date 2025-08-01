#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <vector>
#include <string>
#include <cstdint>

// Protocol constants
namespace ProtocolCodes {
    const uint16_t REGISTRATION_REQUEST = 1000;
    const uint16_t REGISTRATION_SUCCESS = 1001;
    const uint16_t REGISTRATION_FAILURE = 1002;
    const uint16_t LOGIN_REQUEST = 2000;
    const uint16_t LOGIN_SUCCESS = 2001;
    const uint16_t LOGIN_FAILURE = 2002;
    const uint16_t SEND_MESSAGE_REQUEST = 3000;
    const uint16_t SEND_MESSAGE_SUCCESS = 3001;
    const uint16_t SEND_MESSAGE_FAILURE = 3002;
    const uint16_t REQUEST_MESSAGES = 4000;
    const uint16_t MESSAGES_RESPONSE = 4001;
    const uint16_t REQUEST_USERS = 5000;
    const uint16_t USERS_RESPONSE = 5001;
    const uint16_t REQUEST_PUBLIC_KEY = 5002;
    const uint16_t PUBLIC_KEY_RESPONSE = 5003;
    const uint16_t LOGOUT_REQUEST = 6000;
    const uint16_t LOGOUT_SUCCESS = 6001;
}

// Protocol constants for field sizes
namespace ProtocolSizes {
    const uint16_t USERNAME_SIZE = 255;
    const uint16_t PUBLIC_KEY_SIZE = 160;
    const uint16_t HEADER_SIZE = 9;  // version(1) + code(2) + payload_size(2) + checksum(4)
}

class ProtocolHandler {
private:
    std::vector<uint8_t> send_buffer_;
    std::vector<uint8_t> receive_buffer_;
    
    // Helper methods
    std::vector<uint8_t> packString(const std::string& str, size_t fixed_size);
    std::string unpackString(const std::vector<uint8_t>& data, size_t offset, size_t fixed_size) const;
    uint32_t calculateChecksum(const std::vector<uint8_t>& data);
    
public:
    ProtocolHandler();
    ~ProtocolHandler();
    
    // Protocol message creation
    std::vector<uint8_t> createRegistrationRequest(const std::string& username, 
                                                  const std::string& public_key);
    std::vector<uint8_t> createLoginRequest(const std::string& username);
    std::vector<uint8_t> createSendMessageRequest(const std::string& recipient, 
                                                 const std::vector<uint8_t>& message);
    std::vector<uint8_t> createRequestMessagesRequest();
    std::vector<uint8_t> createRequestUsersRequest();
    std::vector<uint8_t> createRequestPublicKeyRequest(const std::string& client_identifier);
    std::vector<uint8_t> createLogoutRequest();
    
    // Protocol message parsing
    bool parseResponse(const std::vector<uint8_t>& data);
    bool isRegistrationSuccess() const;
    bool isLoginSuccess() const;
    bool isMessageReceived() const;
    bool isUsersListReceived() const;
    bool isPublicKeyReceived() const;
    bool isMessagesReceived() const;
    
    // Data extraction
    std::string getErrorMessage() const;
    std::vector<std::string> getUsersList() const;
    std::vector<std::pair<std::string, std::vector<uint8_t>>> getMessages() const;
    std::pair<std::string, std::string> getPublicKeyData() const;  // Returns (client_id, public_key)
    std::vector<std::tuple<std::string, uint32_t, uint8_t, std::string>> getMessagesData() const;  // Returns (from_client_id, message_id, message_type, content)
};

#endif // PROTOCOL_HANDLER_H 