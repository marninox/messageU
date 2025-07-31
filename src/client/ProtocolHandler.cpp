#include "ProtocolHandler.h"
#include <iostream>
#include <cstring>
#include <algorithm>

ProtocolHandler::ProtocolHandler() {
    // Constructor implementation
}

ProtocolHandler::~ProtocolHandler() {
    // Destructor implementation
}

std::vector<uint8_t> ProtocolHandler::createRegistrationRequest(const std::string& username, 
                                                              const std::string& public_key) {
    // Create payload
    std::vector<uint8_t> payload;
    
    // Username: 255 bytes
    auto username_bytes = packString(username, ProtocolSizes::USERNAME_SIZE);
    payload.insert(payload.end(), username_bytes.begin(), username_bytes.end());
    
    // Public key: 160 bytes
    auto public_key_bytes = packString(public_key, ProtocolSizes::PUBLIC_KEY_SIZE);
    payload.insert(payload.end(), public_key_bytes.begin(), public_key_bytes.end());
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    ProtocolHeader header;
    header.version = 1;
    header.code = ProtocolCodes::REGISTRATION_REQUEST;
    header.payload_size = static_cast<uint16_t>(payload.size());
    header.payload_checksum = checksum;
    
    // Serialize header and combine with payload
    std::vector<uint8_t> header_bytes = serializeHeader(header);
    std::vector<uint8_t> result = header_bytes;
    result.insert(result.end(), payload.begin(), payload.end());
    
    return result;
}

std::vector<uint8_t> ProtocolHandler::createLoginRequest(const std::string& username) {
    // Create payload
    std::vector<uint8_t> payload = packString(username, ProtocolSizes::USERNAME_SIZE);
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    ProtocolHeader header;
    header.version = 1;
    header.code = ProtocolCodes::LOGIN_REQUEST;
    header.payload_size = static_cast<uint16_t>(payload.size());
    header.payload_checksum = checksum;
    
    // Serialize header and combine with payload
    std::vector<uint8_t> header_bytes = serializeHeader(header);
    std::vector<uint8_t> result = header_bytes;
    result.insert(result.end(), payload.begin(), payload.end());
    
    return result;
}

std::vector<uint8_t> ProtocolHandler::createSendMessageRequest(const std::string& recipient, 
                                                             const std::vector<uint8_t>& message) {
    // Create payload
    std::vector<uint8_t> payload;
    
    // Recipient: 255 bytes
    auto recipient_bytes = packString(recipient, ProtocolSizes::USERNAME_SIZE);
    payload.insert(payload.end(), recipient_bytes.begin(), recipient_bytes.end());
    
    // Message type: 1 byte
    payload.push_back(0); // Default message type
    
    // Message length: 4 bytes (little-endian)
    uint32_t message_length = static_cast<uint32_t>(message.size());
    payload.push_back(static_cast<uint8_t>(message_length & 0xFF));
    payload.push_back(static_cast<uint8_t>((message_length >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>((message_length >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((message_length >> 24) & 0xFF));
    
    // Message content
    payload.insert(payload.end(), message.begin(), message.end());
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    ProtocolHeader header;
    header.version = 1;
    header.code = ProtocolCodes::SEND_MESSAGE_REQUEST;
    header.payload_size = static_cast<uint16_t>(payload.size());
    header.payload_checksum = checksum;
    
    // Serialize header and combine with payload
    std::vector<uint8_t> header_bytes = serializeHeader(header);
    std::vector<uint8_t> result = header_bytes;
    result.insert(result.end(), payload.begin(), payload.end());
    
    return result;
}

std::vector<uint8_t> ProtocolHandler::createRequestMessagesRequest() {
    // No payload for this request
    std::vector<uint8_t> payload;
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    ProtocolHeader header;
    header.version = 1;
    header.code = ProtocolCodes::REQUEST_MESSAGES;
    header.payload_size = 0;
    header.payload_checksum = checksum;
    
    // Serialize header
    return serializeHeader(header);
}

std::vector<uint8_t> ProtocolHandler::createRequestUsersRequest() {
    // No payload for this request
    std::vector<uint8_t> payload;
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    ProtocolHeader header;
    header.version = 1;
    header.code = ProtocolCodes::REQUEST_USERS;
    header.payload_size = 0;
    header.payload_checksum = checksum;
    
    // Serialize header
    return serializeHeader(header);
}

std::vector<uint8_t> ProtocolHandler::createLogoutRequest() {
    // No payload for this request
    std::vector<uint8_t> payload;
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    ProtocolHeader header;
    header.version = 1;
    header.code = ProtocolCodes::LOGOUT_REQUEST;
    header.payload_size = 0;
    header.payload_checksum = checksum;
    
    // Serialize header
    return serializeHeader(header);
}

bool ProtocolHandler::parseResponse(const std::vector<uint8_t>& data) {
    // Parse response logic will be implemented here
    return false;
}

bool ProtocolHandler::isRegistrationSuccess() const {
    // Check if registration was successful
    return false;
}

bool ProtocolHandler::isLoginSuccess() const {
    // Check if login was successful
    return false;
}

bool ProtocolHandler::isMessageReceived() const {
    // Check if message was received
    return false;
}

bool ProtocolHandler::isUsersListReceived() const {
    // Check if users list was received
    return false;
}

std::string ProtocolHandler::getErrorMessage() const {
    // Get error message from response
    return "";
}

std::vector<std::string> ProtocolHandler::getUsersList() const {
    // Get users list from response
    return std::vector<std::string>();
}

std::vector<std::pair<std::string, std::vector<uint8_t>>> ProtocolHandler::getMessages() const {
    // Get messages from response
    return std::vector<std::pair<std::string, std::vector<uint8_t>>>();
}

uint32_t ProtocolHandler::calculateChecksum(const std::vector<uint8_t>& data) {
    uint32_t checksum = 0;
    for (uint8_t byte : data) {
        checksum += byte;
    }
    return checksum;
}

bool ProtocolHandler::validateHeader(const ProtocolHeader& header) {
    // Validate protocol header
    return header.version == 1 && header.payload_size <= 65535;
}

std::vector<uint8_t> ProtocolHandler::serializeHeader(const ProtocolHeader& header) {
    std::vector<uint8_t> result;
    
    // Version: 1 byte
    result.push_back(header.version);
    
    // Code: 1 byte
    result.push_back(header.code);
    
    // Payload size: 2 bytes (little-endian)
    result.push_back(static_cast<uint8_t>(header.payload_size & 0xFF));
    result.push_back(static_cast<uint8_t>((header.payload_size >> 8) & 0xFF));
    
    // Checksum: 4 bytes (little-endian)
    result.push_back(static_cast<uint8_t>(header.payload_checksum & 0xFF));
    result.push_back(static_cast<uint8_t>((header.payload_checksum >> 8) & 0xFF));
    result.push_back(static_cast<uint8_t>((header.payload_checksum >> 16) & 0xFF));
    result.push_back(static_cast<uint8_t>((header.payload_checksum >> 24) & 0xFF));
    
    return result;
}

bool ProtocolHandler::deserializeHeader(const std::vector<uint8_t>& data, ProtocolHeader& header) {
    if (data.size() < ProtocolSizes::HEADER_SIZE) {
        return false;
    }
    
    header.version = data[0];
    header.code = data[1];
    header.payload_size = static_cast<uint16_t>(data[2]) | (static_cast<uint16_t>(data[3]) << 8);
    header.payload_checksum = static_cast<uint32_t>(data[4]) |
                             (static_cast<uint32_t>(data[5]) << 8) |
                             (static_cast<uint32_t>(data[6]) << 16) |
                             (static_cast<uint32_t>(data[7]) << 24);
    
    return true;
}

std::vector<uint8_t> ProtocolHandler::packString(const std::string& str, size_t fixed_size) {
    std::vector<uint8_t> result(fixed_size, 0);
    size_t copy_size = std::min(str.length(), fixed_size);
    std::copy(str.begin(), str.begin() + copy_size, result.begin());
    return result;
}

std::string ProtocolHandler::unpackString(const std::vector<uint8_t>& data, size_t offset, size_t fixed_size) {
    if (offset + fixed_size > data.size()) {
        return "";
    }
    
    std::string result;
    for (size_t i = 0; i < fixed_size; ++i) {
        if (data[offset + i] == 0) {
            break;
        }
        result += static_cast<char>(data[offset + i]);
    }
    return result;
} 