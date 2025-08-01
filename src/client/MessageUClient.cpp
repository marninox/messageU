#include "MessageUClient.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

MessageUClient::MessageUClient() 
    : server_port_(0), is_registered_(false), is_connected_(false) {
    // Constructor implementation
}

MessageUClient::~MessageUClient() {
    // Destructor implementation
}

bool MessageUClient::initialize() {
    std::cout << "Initializing MessageU Client..." << std::endl;
    
    // Load server configuration
    if (!loadServerConfig()) {
        std::cerr << "Failed to load server configuration. Exiting." << std::endl;
        return false;
    }
    
    // Load client configuration
    loadClientConfig();
    
    std::cout << "Client initialized successfully!" << std::endl;
    return true;
}

void MessageUClient::run() {
    std::cout << "Welcome to MessageU Client!" << std::endl;
    
    while (true) {
        showMenu();
        int choice = 0;
        
        std::cout << "Enter your choice: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number." << std::endl;
            continue;
        }
        
        if (choice == 7) { // Exit
            exitClient();
            break;
        }
        
        handleMenuChoice(choice);
    }
}

void MessageUClient::shutdown() {
    std::cout << "Shutting down MessageU Client..." << std::endl;
    // Cleanup logic will be implemented here
}

bool MessageUClient::loadServerConfig() {
    std::ifstream file("server.info");
    if (!file.is_open()) {
        std::cerr << "Error: server.info file not found!" << std::endl;
        return false;
    }
    
    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        if (!(iss >> server_ip_ >> server_port_)) {
            std::cerr << "Error: Invalid server.info format!" << std::endl;
            return false;
        }
    } else {
        std::cerr << "Error: Empty server.info file!" << std::endl;
        return false;
    }
    
    std::cout << "Server config loaded: " << server_ip_ << ":" << server_port_ << std::endl;
    return true;
}

bool MessageUClient::loadClientConfig() {
    std::ifstream file("me.info");
    if (!file.is_open()) {
        std::cout << "me.info not found. Registration required before using other features." << std::endl;
        is_registered_ = false;
        return false;
    }
    
    std::string line;
    int line_count = 0;
    std::string private_key;
    
    while (std::getline(file, line) && line_count < 4) {
        switch (line_count) {
            case 0:
                client_name_ = line;
                break;
            case 1:
                client_id_ = line;
                break;
            case 2:
                client_public_key_ = line;
                break;
            case 3:
                private_key = line;
                break;
        }
        line_count++;
    }
    
    if (line_count >= 3) {
        is_registered_ = true;
        std::cout << "Client config loaded: " << client_name_ << " (ID: " << client_id_ << ")" << std::endl;
        
        // Load the private key into the crypto object if available
        if (line_count >= 4 && !private_key.empty()) {
            // Load the private key into the crypto object
            if (crypto_.loadPrivateKeyFromPEM(private_key)) {
                std::cout << "Private key loaded for decryption" << std::endl;
            } else {
                std::cout << "Warning: Failed to load private key for decryption" << std::endl;
            }
        }
        
        return true;
    } else {
        std::cout << "me.info file is incomplete. Registration required." << std::endl;
        is_registered_ = false;
        return false;
    }
}

void MessageUClient::showMenu() {
    std::cout << "\n=== MessageU Client Menu ===" << std::endl;
    std::cout << "1. Register" << std::endl;
    std::cout << "2. Request client list" << std::endl;
    std::cout << "3. Get public key" << std::endl;
    std::cout << "4. Get waiting messages" << std::endl;
    std::cout << "5. Send message" << std::endl;
    std::cout << "6. Send file" << std::endl;
    std::cout << "7. Exit" << std::endl;
    std::cout << "===========================" << std::endl;
}

void MessageUClient::handleMenuChoice(int choice) {
    switch (choice) {
        case 1:
            registerUser();
            break;
        case 2:
            requestClientList();
            break;
        case 3:
            getPublicKey();
            break;
        case 4:
            getWaitingMessages();
            break;
        case 5:
            sendMessage();
            break;
        case 6:
            sendFile();
            break;
        default:
            std::cout << "Invalid choice. Please select 1-7." << std::endl;
            break;
    }
}

void MessageUClient::registerUser() {
    if (is_registered_) {
        std::cout << "Already registered. Cannot register again." << std::endl;
        return;
    }
    
    std::cout << "=== Registration ===" << std::endl;
    
    // Get username from user
    std::string username;
    std::cout << "Enter username: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer
    std::getline(std::cin, username);
    
    if (username.empty()) {
        std::cout << "Username cannot be empty." << std::endl;
        return;
    }
    
    // Generate RSA key pair for the user
    if (!crypto_.generateKeyPair()) {
        std::cout << "Failed to generate RSA key pair." << std::endl;
        return;
    }
    
    std::string public_key = crypto_.getPublicKeyPEM();
    if (public_key.empty()) {
        std::cout << "Failed to get public key." << std::endl;
        return;
    }
    
    std::cout << "Generated RSA key pair for: " << username << std::endl;
    
    // Create registration request with PEM-formatted public key
    std::vector<uint8_t> request = protocol_.createRegistrationRequest(username, public_key);
    
    // Connect to server
    if (!network_.connect(server_ip_, server_port_)) {
        std::cout << "Failed to connect to server." << std::endl;
        return;
    }
    
    std::cout << "Connected to server. Sending registration request..." << std::endl;
    
    // Send registration request
    if (!network_.sendData(request)) {
        std::cout << "Failed to send registration request." << std::endl;
        network_.disconnect();
        return;
    }
    
    // Receive response
    std::vector<uint8_t> response;
    if (!network_.receiveData(response)) {
        std::cout << "Failed to receive registration response." << std::endl;
        network_.disconnect();
        return;
    }
    
    // Parse response
    if (!protocol_.parseResponse(response)) {
        std::cout << "Invalid response format." << std::endl;
        network_.disconnect();
        return;
    }
    
    if (protocol_.isRegistrationSuccess()) {
        // Extract client ID from response payload
        if (response.size() >= 9 + 16) { // header(9) + client_id(16)
            std::string client_id;
            for (int i = 9; i < 25; i++) {
                if (response[i] != 0) {
                    client_id += static_cast<char>(response[i]);
                }
            }
            
            // Save to me.info file
            std::ofstream file("me.info");
            if (file.is_open()) {
                file << username << std::endl;
                file << client_id << std::endl;
                file << public_key << std::endl;
                file << crypto_.getPrivateKeyPEM() << std::endl;  // Save private key too
                file.close();
                
                // Update internal state
                client_name_ = username;
                client_id_ = client_id;
                client_public_key_ = public_key;
                is_registered_ = true;
                
                std::cout << "Registration successful!" << std::endl;
                std::cout << "Client ID: " << client_id << std::endl;
                std::cout << "RSA keys saved to me.info" << std::endl;
            } else {
                std::cout << "Failed to save user info to me.info" << std::endl;
            }
        } else {
            std::cout << "Invalid registration response format." << std::endl;
        }
    } else {
        std::string error_msg = protocol_.getErrorMessage();
        if (!error_msg.empty()) {
            std::cout << "Registration failed: " << error_msg << std::endl;
        } else {
            std::cout << "Registration failed: Unknown error" << std::endl;
        }
    }
    
    network_.disconnect();
}

void MessageUClient::requestClientList() {
    if (!is_registered_) {
        std::cout << "Must register first before requesting client list." << std::endl;
        return;
    }
    
    std::cout << "=== Requesting Client List ===" << std::endl;
    
    // Create request for client list
    std::vector<uint8_t> request = protocol_.createRequestUsersRequest();
    
    // Connect to server
    if (!network_.connect(server_ip_, server_port_)) {
        std::cout << "Failed to connect to server." << std::endl;
        return;
    }
    
    std::cout << "Connected to server. Sending client list request..." << std::endl;
    
    // Send request
    if (!network_.sendData(request)) {
        std::cout << "Failed to send client list request." << std::endl;
        network_.disconnect();
        return;
    }
    
    // Receive response
    std::vector<uint8_t> response;
    if (!network_.receiveData(response)) {
        std::cout << "Failed to receive client list response." << std::endl;
        network_.disconnect();
        return;
    }
    
    // Parse response
    if (!protocol_.parseResponse(response)) {
        std::cout << "Invalid response format." << std::endl;
        network_.disconnect();
        return;
    }
    
    if (protocol_.isUsersListReceived()) {
        // Display client list
        std::vector<std::string> users = protocol_.getUsersList();
        if (!users.empty()) {
            std::cout << "\nRegistered Clients:" << std::endl;
            std::cout << "==================" << std::endl;
            for (const auto& user : users) {
                std::cout << "- " << user << std::endl;
            }
            std::cout << "==================" << std::endl;
        } else {
            std::cout << "No clients found." << std::endl;
        }
    } else {
        std::string error_msg = protocol_.getErrorMessage();
        if (!error_msg.empty()) {
            std::cout << "Failed to get client list: " << error_msg << std::endl;
        } else {
            std::cout << "Failed to get client list: Unknown error" << std::endl;
        }
    }
    
    network_.disconnect();
}

void MessageUClient::getPublicKey() {
    if (!is_registered_) {
        std::cout << "Must register first before getting public keys." << std::endl;
        return;
    }
    
    std::cout << "=== Request Public Key ===" << std::endl;
    
    // Get client identifier from user
    std::string client_identifier;
    std::cout << "Enter client ID or nickname: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer
    std::getline(std::cin, client_identifier);
    
    if (client_identifier.empty()) {
        std::cout << "Client identifier cannot be empty." << std::endl;
        return;
    }
    
    // Create public key request
    std::vector<uint8_t> request = protocol_.createRequestPublicKeyRequest(client_identifier);
    
    // Connect to server
    if (!network_.connect(server_ip_, server_port_)) {
        std::cout << "Failed to connect to server." << std::endl;
        return;
    }
    
    std::cout << "Connected to server. Sending public key request..." << std::endl;
    
    // Send request
    if (!network_.sendData(request)) {
        std::cout << "Failed to send public key request." << std::endl;
        network_.disconnect();
        return;
    }
    
    // Receive response
    std::vector<uint8_t> response;
    if (!network_.receiveData(response)) {
        std::cout << "Failed to receive public key response." << std::endl;
        network_.disconnect();
        return;
    }
    
    // Parse response
    if (!protocol_.parseResponse(response)) {
        std::cout << "Invalid response format." << std::endl;
        network_.disconnect();
        return;
    }
    
    if (protocol_.isPublicKeyReceived()) {
        // Extract public key data
        auto public_key_data = protocol_.getPublicKeyData();
        std::string client_id = public_key_data.first;
        std::string public_key = public_key_data.second;
        
        if (!client_id.empty() && !public_key.empty()) {
            std::cout << "\nPublic Key Retrieved Successfully!" << std::endl;
            std::cout << "================================" << std::endl;
            std::cout << "Client ID: " << client_id << std::endl;
            std::cout << "Public Key:" << std::endl;
            std::cout << public_key << std::endl;
            std::cout << "================================" << std::endl;
        } else {
            std::cout << "Invalid public key response format." << std::endl;
        }
    } else {
        std::string error_msg = protocol_.getErrorMessage();
        if (!error_msg.empty()) {
            std::cout << "Failed to get public key: " << error_msg << std::endl;
        } else {
            std::cout << "Failed to get public key: Unknown error" << std::endl;
        }
    }
    
    network_.disconnect();
}

void MessageUClient::getWaitingMessages() {
    if (!is_registered_) {
        std::cout << "Must register first before getting waiting messages." << std::endl;
        return;
    }
    
    std::cout << "=== Get Waiting Messages ===" << std::endl;
    
    // Create request for waiting messages
    std::vector<uint8_t> request = protocol_.createRequestMessagesRequest(client_id_);
    
    // Connect to server
    if (!network_.connect(server_ip_, server_port_)) {
        std::cout << "Failed to connect to server." << std::endl;
        return;
    }
    
    std::cout << "Connected to server. Sending waiting messages request..." << std::endl;
    
    // Send request
    if (!network_.sendData(request)) {
        std::cout << "Failed to send waiting messages request." << std::endl;
        network_.disconnect();
        return;
    }
    
    // Receive response
    std::vector<uint8_t> response;
    if (!network_.receiveData(response)) {
        std::cout << "Failed to receive waiting messages response." << std::endl;
        network_.disconnect();
        return;
    }
    
    // Parse response
    if (!protocol_.parseResponse(response)) {
        std::cout << "Invalid response format." << std::endl;
        network_.disconnect();
        return;
    }
    
    if (protocol_.isMessagesReceived()) {
        // Extract messages data
        auto messages = protocol_.getMessagesData();
        
        if (!messages.empty()) {
            std::cout << "\nWaiting Messages:" << std::endl;
            std::cout << "=================" << std::endl;
            
            // First, process any symmetric key messages (Type 2)
            for (size_t i = 0; i < messages.size(); i++) {
                auto message_data = messages[i];
                std::string from_client_id = std::get<0>(message_data);
                uint32_t message_id = std::get<1>(message_data);
                uint8_t message_type = std::get<2>(message_data);
                std::string content = std::get<3>(message_data);
                
                if (message_type == 2) { // Symmetric key message
                    std::cout << "Processing symmetric key from: " << from_client_id << std::endl;
                    
                    // Convert content string to bytes for processing
                    std::vector<uint8_t> encrypted_key;
                    
                    // Decode base64 content
                    try {
                        std::string base64_content = content;
                        encrypted_key = crypto_.base64Decode(base64_content);
                    } catch (...) {
                        // Fallback: treat as raw bytes
                        encrypted_key = std::vector<uint8_t>(content.begin(), content.end());
                    }
                    
                    // Process the symmetric key exchange message
                    if (processSymmetricKeyMessage(from_client_id, encrypted_key)) {
                        std::cout << "✓ Symmetric key processed successfully from " << from_client_id << std::endl;
                    } else {
                        std::cout << "✗ Failed to process symmetric key from " << from_client_id << std::endl;
                    }
                }
            }
            
            // Now process regular messages (Type 1)
            for (size_t i = 0; i < messages.size(); i++) {
                auto message_data = messages[i];
                std::string from_client_id = std::get<0>(message_data);
                uint32_t message_id = std::get<1>(message_data);
                uint8_t message_type = std::get<2>(message_data);
                std::string content = std::get<3>(message_data);
                
                if (message_type == 1) { // Regular message
                    // Convert content string to bytes for decryption
                    std::vector<uint8_t> encrypted_content;
                    
                    // Decode base64 content
                    try {
                        // Simple base64 decoding (for testing)
                        std::string base64_content = content;
                        encrypted_content = crypto_.base64Decode(base64_content);
                    } catch (...) {
                        // Fallback: treat as raw bytes
                        encrypted_content = std::vector<uint8_t>(content.begin(), content.end());
                    }
                    
                    // Try to decrypt the message
                    std::string decrypted_content;
                    if (crypto_.hasSymmetricKey(from_client_id)) {
                        std::vector<uint8_t> symmetric_key = crypto_.getSymmetricKey(from_client_id);
                        std::vector<uint8_t> decrypted_bytes = crypto_.decryptAES(encrypted_content, symmetric_key);
                        
                        if (!decrypted_bytes.empty()) {
                            decrypted_content = std::string(decrypted_bytes.begin(), decrypted_bytes.end());
                        } else {
                            decrypted_content = "[Failed to decrypt message]";
                        }
                    } else {
                        decrypted_content = "[No symmetric key available for decryption]";
                    }
                    
                    std::cout << "Message " << (i + 1) << ":" << std::endl;
                    std::cout << "  From: " << from_client_id << std::endl;
                    std::cout << "  ID: " << message_id << std::endl;
                    std::cout << "  Type: " << static_cast<int>(message_type) << std::endl;
                    std::cout << "  Content: " << decrypted_content << std::endl;
                    std::cout << "  ---" << std::endl;
                }
            }
            std::cout << "=================" << std::endl;
        } else {
            std::cout << "No waiting messages." << std::endl;
        }
    } else {
        std::string error_msg = protocol_.getErrorMessage();
        if (!error_msg.empty()) {
            std::cout << "Failed to get waiting messages: " << error_msg << std::endl;
        } else {
            std::cout << "Failed to get waiting messages: Unknown error" << std::endl;
        }
    }
    
    network_.disconnect();
}

void MessageUClient::sendMessage() {
    if (!is_registered_) {
        std::cout << "Must register first before sending messages." << std::endl;
        return;
    }
    
    std::cout << "=== Send Message ===" << std::endl;
    
    // Get recipient from user
    std::string recipient;
    std::cout << "Enter recipient ID or nickname: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear buffer
    std::getline(std::cin, recipient);
    
    if (recipient.empty()) {
        std::cout << "Recipient cannot be empty." << std::endl;
        return;
    }
    
    // Check if we have a symmetric key for this recipient
    if (!crypto_.hasSymmetricKey(recipient)) {
        std::cout << "No symmetric key found for recipient: " << recipient << std::endl;
        std::cout << "Initiating key exchange..." << std::endl;
        
        // Send symmetric key (this will also get the public key)
        if (!sendSymmetricKey(recipient)) {
            std::cout << "Failed to send symmetric key. Cannot send message." << std::endl;
            return;
        }
        
        std::cout << "Key exchange completed successfully!" << std::endl;
    }
    
    // Get message content from user
    std::string message_content;
    std::cout << "Enter message content: ";
    std::getline(std::cin, message_content);
    
    if (message_content.empty()) {
        std::cout << "Message content cannot be empty." << std::endl;
        return;
    }
    
    // Convert message content to bytes
    std::vector<uint8_t> message_bytes(message_content.begin(), message_content.end());
    
    // Encrypt the message with the symmetric key
    std::vector<uint8_t> symmetric_key = crypto_.getSymmetricKey(recipient);
    if (symmetric_key.empty()) {
        std::cout << "Failed to get symmetric key for encryption." << std::endl;
        return;
    }
    
    std::vector<uint8_t> encrypted_message = crypto_.encryptAES(message_bytes, symmetric_key);
    if (encrypted_message.empty()) {
        std::cout << "Failed to encrypt message." << std::endl;
        return;
    }
    
    std::cout << "Message encrypted successfully." << std::endl;
    
    // Create send message request with encrypted content
    std::vector<uint8_t> request = protocol_.createSendMessageRequest(client_id_, recipient, encrypted_message);
    
    // Connect to server
    if (!network_.connect(server_ip_, server_port_)) {
        std::cout << "Failed to connect to server." << std::endl;
        return;
    }
    
    std::cout << "Connected to server. Sending encrypted message..." << std::endl;
    
    // Send request
    if (!network_.sendData(request)) {
        std::cout << "Failed to send message request." << std::endl;
        network_.disconnect();
        return;
    }
    
    // Receive response
    std::vector<uint8_t> response;
    if (!network_.receiveData(response)) {
        std::cout << "Failed to receive send message response." << std::endl;
        network_.disconnect();
        return;
    }
    
    // Parse response
    if (!protocol_.parseResponse(response)) {
        std::cout << "Invalid response format." << std::endl;
        network_.disconnect();
        return;
    }
    
    if (protocol_.isSendMessageSuccess()) {
        std::string success_msg = protocol_.getErrorMessage(); // This actually gets the success message
        if (!success_msg.empty()) {
            std::cout << "✓ " << success_msg << std::endl;
        } else {
            std::cout << "✓ Encrypted message sent successfully!" << std::endl;
        }
    } else {
        std::string error_msg = protocol_.getErrorMessage();
        if (!error_msg.empty()) {
            std::cout << "✗ Failed to send message: " << error_msg << std::endl;
        } else {
            std::cout << "✗ Failed to send message: Unknown error" << std::endl;
        }
    }
    
    network_.disconnect();
}

void MessageUClient::sendFile() {
    if (!is_registered_) {
        std::cout << "Must register first before sending files." << std::endl;
        return;
    }
    std::cout << "Selected: Send file" << std::endl;
    // Send file logic will be implemented here
}

void MessageUClient::exitClient() {
    std::cout << "Selected: Exit" << std::endl;
    std::cout << "Goodbye!" << std::endl;
}

bool MessageUClient::getPublicKeyForRecipient(const std::string& recipient) {
    // Create public key request
    std::vector<uint8_t> request = protocol_.createRequestPublicKeyRequest(recipient);
    
    // Connect to server
    if (!network_.connect(server_ip_, server_port_)) {
        std::cout << "Failed to connect to server for public key request." << std::endl;
        return false;
    }
    
    // Send request
    if (!network_.sendData(request)) {
        std::cout << "Failed to send public key request." << std::endl;
        network_.disconnect();
        return false;
    }
    
    // Receive response
    std::vector<uint8_t> response;
    if (!network_.receiveData(response)) {
        std::cout << "Failed to receive public key response." << std::endl;
        network_.disconnect();
        return false;
    }
    
    // Parse response
    if (!protocol_.parseResponse(response)) {
        std::cout << "Invalid public key response format." << std::endl;
        network_.disconnect();
        return false;
    }
    
    if (protocol_.isPublicKeyReceived()) {
        auto public_key_data = protocol_.getPublicKeyData();
        std::string client_id = public_key_data.first;
        std::string public_key = public_key_data.second;
        
        if (!client_id.empty() && !public_key.empty()) {
            std::cout << "Received public key for: " << client_id << std::endl;
            network_.disconnect();
            return true;
        } else {
            std::cout << "Invalid public key response format." << std::endl;
            network_.disconnect();
            return false;
        }
    } else {
        std::string error_msg = protocol_.getErrorMessage();
        std::cout << "Failed to get public key: " << error_msg << std::endl;
        network_.disconnect();
        return false;
    }
}

bool MessageUClient::sendSymmetricKey(const std::string& recipient) {
    // Get recipient's public key first
    if (!getPublicKeyForRecipient(recipient)) {
        std::cout << "Failed to get recipient's public key for key exchange." << std::endl;
        return false;
    }
    
    // Get the public key from the last response
    auto public_key_data = protocol_.getPublicKeyData();
    std::string recipient_public_key = public_key_data.second;
    
    if (recipient_public_key.empty()) {
        std::cout << "Failed to get recipient's public key." << std::endl;
        return false;
    }
    
    // Create symmetric key exchange message
    std::vector<uint8_t> encrypted_key = crypto_.createKeyExchangeMessage(recipient, recipient_public_key);
    if (encrypted_key.empty()) {
        std::cout << "Failed to create key exchange message." << std::endl;
        return false;
    }
    
    // Create symmetric key request
    std::vector<uint8_t> request = protocol_.createSendSymmetricKeyRequest(client_id_, recipient, encrypted_key);
    
    // Connect to server
    if (!network_.connect(server_ip_, server_port_)) {
        std::cout << "Failed to connect to server for key exchange." << std::endl;
        return false;
    }
    
    // Send request
    if (!network_.sendData(request)) {
        std::cout << "Failed to send symmetric key." << std::endl;
        network_.disconnect();
        return false;
    }
    
    // Receive response
    std::vector<uint8_t> response;
    if (!network_.receiveData(response)) {
        std::cout << "Failed to receive key exchange response." << std::endl;
        network_.disconnect();
        return false;
    }
    
    // Parse response
    if (!protocol_.parseResponse(response)) {
        std::cout << "Invalid key exchange response format." << std::endl;
        network_.disconnect();
        return false;
    }
    
    if (protocol_.isSymmetricKeyReceived()) {
        std::cout << "Symmetric key sent successfully!" << std::endl;
        network_.disconnect();
        return true;
    } else {
        std::string error_msg = protocol_.getErrorMessage();
        std::cout << "Failed to send symmetric key: " << error_msg << std::endl;
        network_.disconnect();
        return false;
    }
}

bool MessageUClient::processSymmetricKeyMessage(const std::string& sender_id, const std::vector<uint8_t>& encrypted_key) {
    return crypto_.processKeyExchangeMessage(sender_id, encrypted_key);
} 