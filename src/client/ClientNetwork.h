#ifndef CLIENT_NETWORK_H
#define CLIENT_NETWORK_H

#include <string>
#include <vector>
#include <boost/asio.hpp>

class ClientNetwork {
private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::socket socket_;
    std::string server_host_;
    unsigned short server_port_;
    
public:
    ClientNetwork();
    ~ClientNetwork();
    
    bool connect(const std::string& host, unsigned short port);
    void disconnect();
    bool isConnected() const;
    
    bool sendData(const std::vector<uint8_t>& data);
    bool receiveData(std::vector<uint8_t>& data);
    
    void setServerInfo(const std::string& host, unsigned short port);
};

#endif // CLIENT_NETWORK_H 