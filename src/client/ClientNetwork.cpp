#include "ClientNetwork.h"
#include <iostream>
#include <boost/asio.hpp>

ClientNetwork::ClientNetwork() : io_context_(), socket_(io_context_) {
    // Constructor implementation
}

ClientNetwork::~ClientNetwork() {
    disconnect();
}

bool ClientNetwork::connect(const std::string& host, unsigned short port) {
    try {
        boost::asio::ip::tcp::resolver resolver(io_context_);
        boost::asio::ip::tcp::resolver::results_type endpoints = 
            resolver.resolve(host, std::to_string(port));
        
        boost::asio::connect(socket_, endpoints);
        std::cout << "Connected to " << host << ":" << port << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return false;
    }
}

void ClientNetwork::disconnect() {
    try {
        if (socket_.is_open()) {
            socket_.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during disconnect: " << e.what() << std::endl;
    }
}

bool ClientNetwork::isConnected() const {
    return socket_.is_open();
}

bool ClientNetwork::sendData(const std::vector<uint8_t>& data) {
    try {
        boost::asio::write(socket_, boost::asio::buffer(data));
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Send failed: " << e.what() << std::endl;
        return false;
    }
}

bool ClientNetwork::receiveData(std::vector<uint8_t>& data) {
    try {
        // First read the header (9 bytes)
        std::vector<uint8_t> header(9);
        boost::asio::read(socket_, boost::asio::buffer(header));
        
        // Parse header to get payload size
        uint16_t payload_size = static_cast<uint16_t>(header[3]) | (static_cast<uint16_t>(header[4]) << 8);
        
        // Read the payload
        data.resize(9 + payload_size);
        std::copy(header.begin(), header.end(), data.begin());
        
        if (payload_size > 0) {
            boost::asio::read(socket_, boost::asio::buffer(&data[9], payload_size));
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Receive failed: " << e.what() << std::endl;
        return false;
    }
}

void ClientNetwork::setServerInfo(const std::string& host, unsigned short port) {
    server_host_ = host;
    server_port_ = port;
} 