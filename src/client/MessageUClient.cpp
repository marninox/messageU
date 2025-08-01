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
    
    while (std::getline(file, line) && line_count < 3) {
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
        }
        line_count++;
    }
    
    if (line_count >= 2) {
        is_registered_ = true;
        std::cout << "Client config loaded: " << client_name_ << " (ID: " << client_id_ << ")" << std::endl;
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
    
    // Generate a simple public key (in real implementation, this would use Crypto++)
    std::string public_key = "-----BEGIN PUBLIC KEY-----\n";
    public_key += "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA";
    public_key += username + "_generated_key_12345\n";
    public_key += "-----END PUBLIC KEY-----";
    
    std::cout << "Generated public key for: " << username << std::endl;
    
    // Create registration request
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
                file.close();
                
                // Update internal state
                client_name_ = username;
                client_id_ = client_id;
                client_public_key_ = public_key;
                is_registered_ = true;
                
                std::cout << "Registration successful!" << std::endl;
                std::cout << "Client ID: " << client_id << std::endl;
                std::cout << "User info saved to me.info" << std::endl;
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
    std::cout << "Selected: Get public key" << std::endl;
    // Public key request logic will be implemented here
}

void MessageUClient::getWaitingMessages() {
    if (!is_registered_) {
        std::cout << "Must register first before getting waiting messages." << std::endl;
        return;
    }
    std::cout << "Selected: Get waiting messages" << std::endl;
    // Waiting messages logic will be implemented here
}

void MessageUClient::sendMessage() {
    if (!is_registered_) {
        std::cout << "Must register first before sending messages." << std::endl;
        return;
    }
    std::cout << "Selected: Send message" << std::endl;
    // Send message logic will be implemented here
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