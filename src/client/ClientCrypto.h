#ifndef CLIENT_CRYPTO_H
#define CLIENT_CRYPTO_H

#include <string>
#include <vector>
#include <map>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>

class ClientCrypto {
private:
    CryptoPP::AutoSeededRandomPool rng_;
    CryptoPP::RSA::PrivateKey private_key_;
    CryptoPP::RSA::PublicKey public_key_;
    std::map<std::string, std::vector<uint8_t>> symmetric_keys_; // recipient_id -> symmetric_key
    
    // Helper methods
    std::vector<uint8_t> generateRandomBytes(size_t length);
    std::string base64Encode(const std::vector<uint8_t>& data);
    
public:
    ClientCrypto();
    ~ClientCrypto();
    
    // Helper methods
    std::vector<uint8_t> base64Decode(const std::string& encoded);
    
    // RSA Key Management
    bool generateKeyPair();
    bool loadKeysFromFile(const std::string& filename);
    bool saveKeysToFile(const std::string& filename);
    bool loadServerPublicKey(const std::string& filename);
    bool loadPrivateKeyFromPEM(const std::string& pem_key);
    
    // Symmetric Key Management
    std::vector<uint8_t> generateSymmetricKey();
    bool storeSymmetricKey(const std::string& recipient_id, const std::vector<uint8_t>& key);
    std::vector<uint8_t> getSymmetricKey(const std::string& recipient_id);
    bool hasSymmetricKey(const std::string& recipient_id);
    
    // RSA Encryption/Decryption
    std::vector<uint8_t> encryptWithPublicKey(const std::vector<uint8_t>& data, const std::string& public_key_pem);
    std::vector<uint8_t> decryptWithPrivateKey(const std::vector<uint8_t>& encrypted_data);
    
    // AES Encryption/Decryption
    std::vector<uint8_t> encryptAES(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key);
    std::vector<uint8_t> decryptAES(const std::vector<uint8_t>& encrypted_data, const std::vector<uint8_t>& key);
    
    // Message Encryption/Decryption
    std::vector<uint8_t> encryptMessage(const std::vector<uint8_t>& message);
    std::vector<uint8_t> decryptMessage(const std::vector<uint8_t>& encrypted_message);
    
    // Digital Signatures
    std::vector<uint8_t> signMessage(const std::vector<uint8_t>& message);
    bool verifySignature(const std::vector<uint8_t>& message, 
                        const std::vector<uint8_t>& signature,
                        const CryptoPP::RSA::PublicKey& key);
    
    // Session Key Management
    std::vector<uint8_t> generateSessionKey();
    std::vector<uint8_t> encryptSessionKey(const std::vector<uint8_t>& session_key);
    
    // Key Exchange
    std::vector<uint8_t> createKeyExchangeMessage(const std::string& recipient_id, const std::string& recipient_public_key);
    bool processKeyExchangeMessage(const std::string& sender_id, const std::vector<uint8_t>& encrypted_key);
    
    // Utility methods
    std::string getPublicKeyPEM();
    std::string getPrivateKeyPEM();
};

#endif // CLIENT_CRYPTO_H 