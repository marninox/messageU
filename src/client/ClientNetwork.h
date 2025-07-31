#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

#include <string>
#include <vector>
#include <boost/asio.hpp>

class ClientNetwork {
private:
    boost::asio::io_context io_context_;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
    std::string server_host_;
    unsigned short server_port_;
    
public:
    ClientNetwork();
    ~ClientNetwork();
    
    // Connection management
    bool connect(const std::string& host, unsigned short port);
    void disconnect();
    bool isConnected() const;
    
    // Data transmission
    bool sendData(const std::vector<uint8_t>& data);
    bool receiveData(std::vector<uint8_t>& data);
    
    // Network utilities
    void setServerInfo(const std::string& host, unsigned short port);
    boost::asio::io_context& getIoContext();
};

#endif // CLIENT_NETWORK_H 