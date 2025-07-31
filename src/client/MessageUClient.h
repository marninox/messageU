#ifndef MESSAGEU_CLIENT_H
#define MESSAGEU_CLIENT_H

#include <string>
#include <memory>

class MessageUClient {
private:
    // Private member variables will be added here
    
public:
    MessageUClient();
    ~MessageUClient();
    
    // Main client operations
    bool initialize();
    void run();
    void shutdown();
    
    // Menu and user interaction
    void displayMenu();
    void processUserInput();
    
    // Client state management
    bool isConnected() const;
    bool isAuthenticated() const;
};

#endif // MESSAGEU_CLIENT_H 