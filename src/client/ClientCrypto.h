#ifndef CLIENT_CRYPTO_H
#define CLIENT_CRYPTO_H

#include <string>
#include <vector>
#include <cryptopp/rsa.h>
#include <cryptopp/aes.h>

class ClientCrypto {
private:
    // Crypto++ objects for key management
    CryptoPP::RSA::PrivateKey private_key_;
    CryptoPP::RSA::PublicKey public_key_;
    CryptoPP::RSA::PublicKey server_public_key_;
    
public:
    ClientCrypto();
    ~ClientCrypto();
    
    // Key management
    bool generateKeyPair();
    bool loadKeysFromFile(const std::string& filename);
    bool saveKeysToFile(const std::string& filename);
    bool loadServerPublicKey(const std::string& filename);
    
    // Encryption/Decryption
    std::vector<uint8_t> encryptMessage(const std::vector<uint8_t>& message);
    std::vector<uint8_t> decryptMessage(const std::vector<uint8_t>& encrypted_message);
    
    // Digital signatures
    std::vector<uint8_t> signMessage(const std::vector<uint8_t>& message);
    bool verifySignature(const std::vector<uint8_t>& message, 
                        const std::vector<uint8_t>& signature,
                        const CryptoPP::RSA::PublicKey& key);
    
    // Key exchange and session management
    std::vector<uint8_t> generateSessionKey();
    std::vector<uint8_t> encryptSessionKey(const std::vector<uint8_t>& session_key);
    
    // Utility functions
    std::string getPublicKeyString() const;
    bool hasValidKeys() const;
};

#endif // CLIENT_CRYPTO_H 