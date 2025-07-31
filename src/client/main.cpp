#include <iostream>
#include "MessageUClient.h"

int main() {
    MessageUClient client;
    
    if (!client.initialize()) {
        std::cerr << "Failed to initialize client" << std::endl;
        return 1;
    }
    
    try {
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }
    
    client.shutdown();
    return 0;
}
