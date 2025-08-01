#ifndef MESSAGEU_CLIENT_H
#define MESSAGEU_CLIENT_H

#include <string>
#include <vector>
#include "ClientNetwork.h"
#include "ClientCrypto.h"
#include "ProtocolHandler.h"

/**
 * MessageU Client - Main client application
 * 
 * Features:
 * - User registration with RSA key generation
 * - End-to-end encrypted messaging
 * - Automatic key exchange and management
 * - Client discovery and public key retrieval
 * - Secure message storage and retrieval
 */
class MessageUClient {
private:
    ClientNetwork network_;
    ClientCrypto crypto_;
    ProtocolHandler protocol_;
    
    // Configuration data
    std::string server_ip_;
    unsigned short server_port_;
    std::string client_name_;
    std::string client_id_;
    std::string client_private_key_;
    std::string client_public_key_;
    
    // State
    bool is_registered_;
    bool is_connected_;
    
    // Private methods
    bool loadServerConfig();
    bool loadClientConfig();
    void showMenu();
    void handleMenuChoice(int choice);
    // Menu options
    void registerUser();
    void requestClientList();
    void getPublicKey();
    void getWaitingMessages();
    void sendMessage();
    void exitClient();
    
    // Key exchange helper methods
    bool getPublicKeyForRecipient(const std::string& recipient);
    bool sendSymmetricKey(const std::string& recipient);
    bool processSymmetricKeyMessage(const std::string& sender_id, const std::vector<uint8_t>& encrypted_key);
    
public:
    MessageUClient();
    ~MessageUClient();
    
    bool initialize();
    void run();
    void shutdown();
};

#endif // MESSAGEU_CLIENT_H 