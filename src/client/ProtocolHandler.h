#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <vector>
#include <string>
#include <cstdint>

// Protocol constants and structures
struct ProtocolHeader {
    uint8_t version;
    uint8_t code;
    uint16_t payload_size;
    uint32_t payload_checksum;
};

class ProtocolHandler {
private:
    // Protocol state and buffers
    std::vector<uint8_t> send_buffer_;
    std::vector<uint8_t> receive_buffer_;
    
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
    std::vector<uint8_t> createLogoutRequest();
    
    // Protocol message parsing
    bool parseResponse(const std::vector<uint8_t>& data);
    bool isRegistrationSuccess() const;
    bool isLoginSuccess() const;
    bool isMessageReceived() const;
    bool isUsersListReceived() const;
    
    // Data extraction
    std::string getErrorMessage() const;
    std::vector<std::string> getUsersList() const;
    std::vector<std::pair<std::string, std::vector<uint8_t>>> getMessages() const;
    
    // Protocol utilities
    uint32_t calculateChecksum(const std::vector<uint8_t>& data);
    bool validateHeader(const ProtocolHeader& header);
    std::vector<uint8_t> serializeHeader(const ProtocolHeader& header);
    bool deserializeHeader(const std::vector<uint8_t>& data, ProtocolHeader& header);
};

#endif // PROTOCOL_HANDLER_H 