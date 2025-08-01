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

std::vector<uint8_t> ProtocolHandler::createRegistrationRequest(const std::string& username, const std::string& public_key) {
    // Create payload: username(255) + public_key(160)
    std::vector<uint8_t> payload;
    
    auto username_bytes = packString(username, ProtocolSizes::USERNAME_SIZE);
    payload.insert(payload.end(), username_bytes.begin(), username_bytes.end());
    
    auto public_key_bytes = packString(public_key, ProtocolSizes::PUBLIC_KEY_SIZE);
    payload.insert(payload.end(), public_key_bytes.begin(), public_key_bytes.end());
    
    // Create header: version(1) + code(2) + payload_size(2) + checksum(4) = 9 bytes
    uint32_t checksum = calculateChecksum(payload);
    
    std::vector<uint8_t> result;
    
    // Version (1 byte)
    result.push_back(1);
    
    // Code (2 bytes, little-endian)
    result.push_back(ProtocolCodes::REGISTRATION_REQUEST & 0xFF);
    result.push_back((ProtocolCodes::REGISTRATION_REQUEST >> 8) & 0xFF);
    
    // Payload size (2 bytes, little-endian)
    uint16_t payload_size = static_cast<uint16_t>(payload.size());
    result.push_back(payload_size & 0xFF);
    result.push_back((payload_size >> 8) & 0xFF);
    
    // Checksum (4 bytes, little-endian)
    result.push_back(checksum & 0xFF);
    result.push_back((checksum >> 8) & 0xFF);
    result.push_back((checksum >> 16) & 0xFF);
    result.push_back((checksum >> 24) & 0xFF);
    
    // Add payload
    result.insert(result.end(), payload.begin(), payload.end());
    
    return result;
}

std::vector<uint8_t> ProtocolHandler::createLoginRequest(const std::string& username) {
    // Create payload: username(255)
    std::vector<uint8_t> payload = packString(username, ProtocolSizes::USERNAME_SIZE);
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    
    std::vector<uint8_t> result;
    
    // Version (1 byte)
    result.push_back(1);
    
    // Code (2 bytes, little-endian)
    result.push_back(ProtocolCodes::LOGIN_REQUEST & 0xFF);
    result.push_back((ProtocolCodes::LOGIN_REQUEST >> 8) & 0xFF);
    
    // Payload size (2 bytes, little-endian)
    uint16_t payload_size = static_cast<uint16_t>(payload.size());
    result.push_back(payload_size & 0xFF);
    result.push_back((payload_size >> 8) & 0xFF);
    
    // Checksum (4 bytes, little-endian)
    result.push_back(checksum & 0xFF);
    result.push_back((checksum >> 8) & 0xFF);
    result.push_back((checksum >> 16) & 0xFF);
    result.push_back((checksum >> 24) & 0xFF);
    
    // Add payload
    result.insert(result.end(), payload.begin(), payload.end());
    
    return result;
}

std::vector<uint8_t> ProtocolHandler::createSendMessageRequest(const std::string& recipient, const std::vector<uint8_t>& message) {
    // Create payload: recipient(255) + message_length(4) + message
    std::vector<uint8_t> payload;
    
    auto recipient_bytes = packString(recipient, ProtocolSizes::USERNAME_SIZE);
    payload.insert(payload.end(), recipient_bytes.begin(), recipient_bytes.end());
    
    // Message length (4 bytes, little-endian)
    uint32_t message_length = static_cast<uint32_t>(message.size());
    payload.push_back(message_length & 0xFF);
    payload.push_back((message_length >> 8) & 0xFF);
    payload.push_back((message_length >> 16) & 0xFF);
    payload.push_back((message_length >> 24) & 0xFF);
    
    // Message content
    payload.insert(payload.end(), message.begin(), message.end());
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    
    std::vector<uint8_t> result;
    
    // Version (1 byte)
    result.push_back(1);
    
    // Code (2 bytes, little-endian)
    result.push_back(ProtocolCodes::SEND_MESSAGE_REQUEST & 0xFF);
    result.push_back((ProtocolCodes::SEND_MESSAGE_REQUEST >> 8) & 0xFF);
    
    // Payload size (2 bytes, little-endian)
    uint16_t payload_size = static_cast<uint16_t>(payload.size());
    result.push_back(payload_size & 0xFF);
    result.push_back((payload_size >> 8) & 0xFF);
    
    // Checksum (4 bytes, little-endian)
    result.push_back(checksum & 0xFF);
    result.push_back((checksum >> 8) & 0xFF);
    result.push_back((checksum >> 16) & 0xFF);
    result.push_back((checksum >> 24) & 0xFF);
    
    // Add payload
    result.insert(result.end(), payload.begin(), payload.end());
    
    return result;
}

std::vector<uint8_t> ProtocolHandler::createRequestMessagesRequest() {
    // Empty payload
    std::vector<uint8_t> payload;
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    
    std::vector<uint8_t> result;
    
    // Version (1 byte)
    result.push_back(1);
    
    // Code (2 bytes, little-endian)
    result.push_back(ProtocolCodes::REQUEST_MESSAGES & 0xFF);
    result.push_back((ProtocolCodes::REQUEST_MESSAGES >> 8) & 0xFF);
    
    // Payload size (2 bytes, little-endian)
    result.push_back(0);  // payload_size = 0
    result.push_back(0);
    
    // Checksum (4 bytes, little-endian)
    result.push_back(checksum & 0xFF);
    result.push_back((checksum >> 8) & 0xFF);
    result.push_back((checksum >> 16) & 0xFF);
    result.push_back((checksum >> 24) & 0xFF);
    
    return result;
}

std::vector<uint8_t> ProtocolHandler::createRequestUsersRequest() {
    // Empty payload
    std::vector<uint8_t> payload;
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    
    std::vector<uint8_t> result;
    
    // Version (1 byte)
    result.push_back(1);
    
    // Code (2 bytes, little-endian)
    result.push_back(ProtocolCodes::REQUEST_USERS & 0xFF);
    result.push_back((ProtocolCodes::REQUEST_USERS >> 8) & 0xFF);
    
    // Payload size (2 bytes, little-endian)
    result.push_back(0);  // payload_size = 0
    result.push_back(0);
    
    // Checksum (4 bytes, little-endian)
    result.push_back(checksum & 0xFF);
    result.push_back((checksum >> 8) & 0xFF);
    result.push_back((checksum >> 16) & 0xFF);
    result.push_back((checksum >> 24) & 0xFF);
    
    return result;
}

std::vector<uint8_t> ProtocolHandler::createLogoutRequest() {
    // Empty payload
    std::vector<uint8_t> payload;
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    
    std::vector<uint8_t> result;
    
    // Version (1 byte)
    result.push_back(1);
    
    // Code (2 bytes, little-endian)
    result.push_back(ProtocolCodes::LOGOUT_REQUEST & 0xFF);
    result.push_back((ProtocolCodes::LOGOUT_REQUEST >> 8) & 0xFF);
    
    // Payload size (2 bytes, little-endian)
    result.push_back(0);  // payload_size = 0
    result.push_back(0);
    
    // Checksum (4 bytes, little-endian)
    result.push_back(checksum & 0xFF);
    result.push_back((checksum >> 8) & 0xFF);
    result.push_back((checksum >> 16) & 0xFF);
    result.push_back((checksum >> 24) & 0xFF);
    
    return result;
}

bool ProtocolHandler::parseResponse(const std::vector<uint8_t>& data) {
    // Store response for later parsing
    receive_buffer_ = data;
    return true;
}

bool ProtocolHandler::isRegistrationSuccess() const {
    if (receive_buffer_.size() < 9) return false;
    
    // Parse code from header
    uint16_t code = static_cast<uint16_t>(receive_buffer_[1]) | (static_cast<uint16_t>(receive_buffer_[2]) << 8);
    return code == ProtocolCodes::REGISTRATION_SUCCESS;
}

bool ProtocolHandler::isLoginSuccess() const {
    if (receive_buffer_.size() < 9) return false;
    
    uint16_t code = static_cast<uint16_t>(receive_buffer_[1]) | (static_cast<uint16_t>(receive_buffer_[2]) << 8);
    return code == ProtocolCodes::LOGIN_SUCCESS;
}

bool ProtocolHandler::isMessageReceived() const {
    if (receive_buffer_.size() < 9) return false;
    
    uint16_t code = static_cast<uint16_t>(receive_buffer_[1]) | (static_cast<uint16_t>(receive_buffer_[2]) << 8);
    return code == ProtocolCodes::MESSAGES_RESPONSE;
}

bool ProtocolHandler::isUsersListReceived() const {
    if (receive_buffer_.size() < 9) return false;
    
    uint16_t code = static_cast<uint16_t>(receive_buffer_[1]) | (static_cast<uint16_t>(receive_buffer_[2]) << 8);
    return code == ProtocolCodes::USERS_RESPONSE;
}

std::string ProtocolHandler::getErrorMessage() const {
    if (receive_buffer_.size() < 9) return "";
    
    // Extract payload
    uint16_t payload_size = static_cast<uint16_t>(receive_buffer_[3]) | (static_cast<uint16_t>(receive_buffer_[4]) << 8);
    if (receive_buffer_.size() < 9 + payload_size) return "";
    
    std::string error_msg;
    for (size_t i = 9; i < 9 + payload_size; i++) {
        error_msg += static_cast<char>(receive_buffer_[i]);
    }
    return error_msg;
}

std::vector<std::string> ProtocolHandler::getUsersList() const {
    // Placeholder implementation
    return {};
}

std::vector<std::pair<std::string, std::vector<uint8_t>>> ProtocolHandler::getMessages() const {
    // Placeholder implementation
    return {};
}

std::vector<uint8_t> ProtocolHandler::packString(const std::string& str, size_t fixed_size) {
    std::vector<uint8_t> result(fixed_size, 0);
    size_t copy_size = std::min(str.length(), fixed_size);
    std::memcpy(result.data(), str.c_str(), copy_size);
    return result;
}

std::string ProtocolHandler::unpackString(const std::vector<uint8_t>& data, size_t offset, size_t fixed_size) {
    if (offset + fixed_size > data.size()) return "";
    
    std::string result;
    for (size_t i = offset; i < offset + fixed_size; i++) {
        if (data[i] == 0) break;
        result += static_cast<char>(data[i]);
    }
    return result;
}

uint32_t ProtocolHandler::calculateChecksum(const std::vector<uint8_t>& data) {
    uint32_t checksum = 0;
    for (uint8_t byte : data) {
        checksum += byte;
    }
    return checksum;
} 