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
    // Create payload: username(255) + public_key(1024)
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

std::vector<uint8_t> ProtocolHandler::createRequestPublicKeyRequest(const std::string& client_identifier) {
    // Create payload: client_identifier(255)
    std::vector<uint8_t> payload = packString(client_identifier, ProtocolSizes::USERNAME_SIZE);
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    
    std::vector<uint8_t> result;
    
    // Version (1 byte)
    result.push_back(1);
    
    // Code (2 bytes, little-endian)
    result.push_back(ProtocolCodes::REQUEST_PUBLIC_KEY & 0xFF);
    result.push_back((ProtocolCodes::REQUEST_PUBLIC_KEY >> 8) & 0xFF);
    
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

std::vector<uint8_t> ProtocolHandler::createSendSymmetricKeyRequest(const std::string& recipient, const std::vector<uint8_t>& encrypted_key) {
    // Create payload: recipient(255) + key_length(4) + encrypted_key
    std::vector<uint8_t> payload;
    
    auto recipient_bytes = packString(recipient, ProtocolSizes::USERNAME_SIZE);
    payload.insert(payload.end(), recipient_bytes.begin(), recipient_bytes.end());
    
    // Key length (4 bytes, little-endian)
    uint32_t key_length = static_cast<uint32_t>(encrypted_key.size());
    payload.push_back(key_length & 0xFF);
    payload.push_back((key_length >> 8) & 0xFF);
    payload.push_back((key_length >> 16) & 0xFF);
    payload.push_back((key_length >> 24) & 0xFF);
    
    // Encrypted key content
    payload.insert(payload.end(), encrypted_key.begin(), encrypted_key.end());
    
    // Create header
    uint32_t checksum = calculateChecksum(payload);
    
    std::vector<uint8_t> result;
    
    // Version (1 byte)
    result.push_back(1);
    
    // Code (2 bytes, little-endian)
    result.push_back(ProtocolCodes::SEND_SYMMETRIC_KEY & 0xFF);
    result.push_back((ProtocolCodes::SEND_SYMMETRIC_KEY >> 8) & 0xFF);
    
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

bool ProtocolHandler::isPublicKeyReceived() const {
    if (receive_buffer_.size() < 9) return false;
    
    uint16_t code = static_cast<uint16_t>(receive_buffer_[1]) | (static_cast<uint16_t>(receive_buffer_[2]) << 8);
    return code == ProtocolCodes::PUBLIC_KEY_RESPONSE;
}

bool ProtocolHandler::isMessagesReceived() const {
    if (receive_buffer_.size() < 9) return false;
    
    uint16_t code = static_cast<uint16_t>(receive_buffer_[1]) | (static_cast<uint16_t>(receive_buffer_[2]) << 8);
    return code == ProtocolCodes::MESSAGES_RESPONSE;
}

bool ProtocolHandler::isSendMessageSuccess() const {
    if (receive_buffer_.size() < 9) return false;
    
    uint16_t code = static_cast<uint16_t>(receive_buffer_[1]) | (static_cast<uint16_t>(receive_buffer_[2]) << 8);
    return code == ProtocolCodes::SEND_MESSAGE_SUCCESS;
}

bool ProtocolHandler::isSymmetricKeyReceived() const {
    if (receive_buffer_.size() < 9) return false;
    
    uint16_t code = static_cast<uint16_t>(receive_buffer_[1]) | (static_cast<uint16_t>(receive_buffer_[2]) << 8);
    return code == ProtocolCodes::SYMMETRIC_KEY_RESPONSE;
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
    if (receive_buffer_.size() < 9) return {};
    
    // Extract payload
    uint16_t payload_size = static_cast<uint16_t>(receive_buffer_[3]) | (static_cast<uint16_t>(receive_buffer_[4]) << 8);
    if (receive_buffer_.size() < 9 + payload_size) return {};
    
    std::vector<std::string> users;
    
    // Parse number of users (4 bytes)
    if (payload_size < 4) return {};
    uint32_t num_users = static_cast<uint32_t>(receive_buffer_[9]) |
                        (static_cast<uint32_t>(receive_buffer_[10]) << 8) |
                        (static_cast<uint32_t>(receive_buffer_[11]) << 16) |
                        (static_cast<uint32_t>(receive_buffer_[12]) << 24);
    
    size_t offset = 13;  // Start after number of users
    
    // Parse each user: client_id(16) + name(255)
    for (uint32_t i = 0; i < num_users; i++) {
        if (offset + 16 + 255 > receive_buffer_.size()) break;
        
        // Extract client ID (16 bytes)
        std::string client_id = unpackString(receive_buffer_, offset, 16);
        offset += 16;
        
        // Extract name (255 bytes)
        std::string name = unpackString(receive_buffer_, offset, 255);
        offset += 255;
        
        // Format: "Name (ID: client_id)"
        std::string user_info = name + " (ID: " + client_id + ")";
        users.push_back(user_info);
    }
    
    return users;
}

std::pair<std::string, std::string> ProtocolHandler::getPublicKeyData() const {
    if (receive_buffer_.size() < 9) return {"", ""};
    
    // Extract payload
    uint16_t payload_size = static_cast<uint16_t>(receive_buffer_[3]) | (static_cast<uint16_t>(receive_buffer_[4]) << 8);
    if (receive_buffer_.size() < 9 + payload_size) return {"", ""};
    
    // Parse: client_id(16) + public_key(1024)
    if (payload_size < 16 + 1024) return {"", ""};
    
    // Extract client ID (16 bytes)
    std::string client_id = unpackString(receive_buffer_, 9, 16);
    
    // Extract public key (1024 bytes)
    std::string public_key = unpackString(receive_buffer_, 9 + 16, 1024);
    
    return {client_id, public_key};
}

std::vector<std::tuple<std::string, uint32_t, uint8_t, std::string>> ProtocolHandler::getMessagesData() const {
    if (receive_buffer_.size() < 9) return {};
    
    // Extract payload
    uint16_t payload_size = static_cast<uint16_t>(receive_buffer_[3]) | (static_cast<uint16_t>(receive_buffer_[4]) << 8);
    if (receive_buffer_.size() < 9 + payload_size) return {};
    
    std::vector<std::tuple<std::string, uint32_t, uint8_t, std::string>> messages;
    
    // Parse number of messages (4 bytes)
    if (payload_size < 4) return {};
    uint32_t num_messages = static_cast<uint32_t>(receive_buffer_[9]) |
                           (static_cast<uint32_t>(receive_buffer_[10]) << 8) |
                           (static_cast<uint32_t>(receive_buffer_[11]) << 16) |
                           (static_cast<uint32_t>(receive_buffer_[12]) << 24);
    
    size_t offset = 13;  // Start after number of messages
    
    // Parse each message: from_client_id(16) + message_id(4) + message_type(1) + content_size(4) + content
    for (uint32_t i = 0; i < num_messages; i++) {
        if (offset + 16 + 4 + 1 + 4 > receive_buffer_.size()) break;
        
        // Extract from client ID (16 bytes)
        std::string from_client_id = unpackString(receive_buffer_, offset, 16);
        offset += 16;
        
        // Extract message ID (4 bytes)
        if (offset + 4 > receive_buffer_.size()) break;
        uint32_t message_id = static_cast<uint32_t>(receive_buffer_[offset]) |
                             (static_cast<uint32_t>(receive_buffer_[offset + 1]) << 8) |
                             (static_cast<uint32_t>(receive_buffer_[offset + 2]) << 16) |
                             (static_cast<uint32_t>(receive_buffer_[offset + 3]) << 24);
        offset += 4;
        
        // Extract message type (1 byte)
        if (offset >= receive_buffer_.size()) break;
        uint8_t message_type = receive_buffer_[offset];
        offset += 1;
        
        // Extract content size (4 bytes)
        if (offset + 4 > receive_buffer_.size()) break;
        uint32_t content_size = static_cast<uint32_t>(receive_buffer_[offset]) |
                               (static_cast<uint32_t>(receive_buffer_[offset + 1]) << 8) |
                               (static_cast<uint32_t>(receive_buffer_[offset + 2]) << 16) |
                               (static_cast<uint32_t>(receive_buffer_[offset + 3]) << 24);
        offset += 4;
        
        // Extract content
        if (offset + content_size > receive_buffer_.size()) break;
        std::string content;
        for (uint32_t j = 0; j < content_size; j++) {
            content += static_cast<char>(receive_buffer_[offset + j]);
        }
        offset += content_size;
        
        // Add message to list
        messages.push_back(std::make_tuple(from_client_id, message_id, message_type, content));
    }
    
    return messages;
}

std::pair<std::string, std::vector<uint8_t>> ProtocolHandler::getSymmetricKeyData() const {
    if (receive_buffer_.size() < 9) return {"", std::vector<uint8_t>()};
    
    // Extract payload
    uint16_t payload_size = static_cast<uint16_t>(receive_buffer_[3]) | (static_cast<uint16_t>(receive_buffer_[4]) << 8);
    if (receive_buffer_.size() < 9 + payload_size) return {"", std::vector<uint8_t>()};
    
    // Parse: sender_id(16) + key_length(4) + encrypted_key
    if (payload_size < 16 + 4) return {"", std::vector<uint8_t>()};
    
    // Extract sender ID (16 bytes)
    std::string sender_id = unpackString(receive_buffer_, 9, 16);
    
    // Extract key length (4 bytes)
    uint32_t key_length = static_cast<uint32_t>(receive_buffer_[25]) |
                         (static_cast<uint32_t>(receive_buffer_[26]) << 8) |
                         (static_cast<uint32_t>(receive_buffer_[27]) << 16) |
                         (static_cast<uint32_t>(receive_buffer_[28]) << 24);
    
    // Extract encrypted key
    if (payload_size < 16 + 4 + key_length) return {"", std::vector<uint8_t>()};
    
    std::vector<uint8_t> encrypted_key;
    for (uint32_t i = 0; i < key_length; i++) {
        encrypted_key.push_back(receive_buffer_[29 + i]);
    }
    
    return {sender_id, encrypted_key};
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

std::string ProtocolHandler::unpackString(const std::vector<uint8_t>& data, size_t offset, size_t fixed_size) const {
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