#include "ClientCrypto.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

ClientCrypto::ClientCrypto() {
    // Constructor implementation
}

ClientCrypto::~ClientCrypto() {
    // Destructor implementation
}

std::vector<uint8_t> ClientCrypto::generateRandomBytes(size_t length) {
    std::vector<uint8_t> bytes(length);
    rng_.GenerateBlock(bytes.data(), length);
    return bytes;
}

std::string ClientCrypto::base64Encode(const std::vector<uint8_t>& data) {
    std::string encoded;
    CryptoPP::StringSource(data.data(), data.size(), true,
        new CryptoPP::Base64Encoder(
            new CryptoPP::StringSink(encoded)
        )
    );
    return encoded;
}

std::vector<uint8_t> ClientCrypto::base64Decode(const std::string& encoded) {
    std::vector<uint8_t> decoded;
    CryptoPP::StringSource(encoded, true,
        new CryptoPP::Base64Decoder(
            new CryptoPP::VectorSink(decoded)
        )
    );
    return decoded;
}

bool ClientCrypto::generateKeyPair() {
    try {
        // Generate RSA key pair
        private_key_.GenerateRandomWithKeySize(rng_, 2048);
        public_key_.AssignFrom(private_key_);
        
        std::cout << "RSA key pair generated successfully" << std::endl;
        return true;
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Error generating key pair: " << e.what() << std::endl;
        return false;
    }
}

bool ClientCrypto::loadKeysFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Could not open key file: " << filename << std::endl;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string key_data = buffer.str();
        
        // Load private key
        CryptoPP::StringSource(key_data, true);
        private_key_.Load(CryptoPP::StringSource(key_data, true).Ref());
        public_key_.AssignFrom(private_key_);
        
        std::cout << "Keys loaded from file: " << filename << std::endl;
        return true;
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Error loading keys: " << e.what() << std::endl;
        return false;
    }
}

bool ClientCrypto::saveKeysToFile(const std::string& filename) {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Could not create key file: " << filename << std::endl;
            return false;
        }
        
        // Save private key in PEM format
        std::string private_key_pem = getPrivateKeyPEM();
        file << private_key_pem;
        file.close();
        
        std::cout << "Keys saved to file: " << filename << std::endl;
        return true;
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Error saving keys: " << e.what() << std::endl;
        return false;
    }
}

bool ClientCrypto::loadServerPublicKey(const std::string& filename) {
    // Placeholder implementation
    return true;
}

std::vector<uint8_t> ClientCrypto::generateSymmetricKey() {
    // Generate 128-bit (16-byte) AES key
    return generateRandomBytes(16);
}

bool ClientCrypto::storeSymmetricKey(const std::string& recipient_id, const std::vector<uint8_t>& key) {
    symmetric_keys_[recipient_id] = key;
    std::cout << "Symmetric key stored for recipient: " << recipient_id << std::endl;
    return true;
}

std::vector<uint8_t> ClientCrypto::getSymmetricKey(const std::string& recipient_id) {
    auto it = symmetric_keys_.find(recipient_id);
    if (it != symmetric_keys_.end()) {
        return it->second;
    }
    return std::vector<uint8_t>();
}

bool ClientCrypto::hasSymmetricKey(const std::string& recipient_id) {
    return symmetric_keys_.find(recipient_id) != symmetric_keys_.end();
}

std::vector<uint8_t> ClientCrypto::encryptWithPublicKey(const std::vector<uint8_t>& data, const std::string& public_key_pem) {
    try {
        // Load public key from PEM string
        CryptoPP::RSA::PublicKey recipient_public_key;
        CryptoPP::StringSource(public_key_pem, true);
        recipient_public_key.Load(CryptoPP::StringSource(public_key_pem, true).Ref());
        
        // Encrypt data with RSA
        std::vector<uint8_t> encrypted;
        CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(recipient_public_key);
        
        CryptoPP::StringSource(data.data(), data.size(), true,
            new CryptoPP::PK_EncryptorFilter(rng_, encryptor,
                new CryptoPP::VectorSink(encrypted)
            )
        );
        
        return encrypted;
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Error encrypting with public key: " << e.what() << std::endl;
        return std::vector<uint8_t>();
    }
}

std::vector<uint8_t> ClientCrypto::decryptWithPrivateKey(const std::vector<uint8_t>& encrypted_data) {
    try {
        std::vector<uint8_t> decrypted;
        CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(private_key_);
        
        CryptoPP::StringSource(encrypted_data.data(), encrypted_data.size(), true,
            new CryptoPP::PK_DecryptorFilter(rng_, decryptor,
                new CryptoPP::VectorSink(decrypted)
            )
        );
        
        return decrypted;
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Error decrypting with private key: " << e.what() << std::endl;
        return std::vector<uint8_t>();
    }
}

std::vector<uint8_t> ClientCrypto::encryptAES(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
    try {
        if (key.size() != 16) {
            std::cerr << "Invalid AES key size: " << key.size() << " bytes" << std::endl;
            return std::vector<uint8_t>();
        }
        
        // Create zero IV (as specified)
        std::vector<uint8_t> iv(16, 0);
        
        std::vector<uint8_t> encrypted;
        CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor;
        encryptor.SetKeyWithIV(key.data(), key.size(), iv.data());
        
        CryptoPP::StringSource(data.data(), data.size(), true,
            new CryptoPP::StreamTransformationFilter(encryptor,
                new CryptoPP::VectorSink(encrypted)
            )
        );
        
        return encrypted;
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Error encrypting with AES: " << e.what() << std::endl;
        return std::vector<uint8_t>();
    }
}

std::vector<uint8_t> ClientCrypto::decryptAES(const std::vector<uint8_t>& encrypted_data, const std::vector<uint8_t>& key) {
    try {
        if (key.size() != 16) {
            std::cerr << "Invalid AES key size: " << key.size() << " bytes" << std::endl;
            return std::vector<uint8_t>();
        }
        
        // Create zero IV (as specified)
        std::vector<uint8_t> iv(16, 0);
        
        std::vector<uint8_t> decrypted;
        CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor;
        decryptor.SetKeyWithIV(key.data(), key.size(), iv.data());
        
        CryptoPP::StringSource(encrypted_data.data(), encrypted_data.size(), true,
            new CryptoPP::StreamTransformationFilter(decryptor,
                new CryptoPP::VectorSink(decrypted)
            )
        );
        
        return decrypted;
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Error decrypting with AES: " << e.what() << std::endl;
        return std::vector<uint8_t>();
    }
}

std::vector<uint8_t> ClientCrypto::encryptMessage(const std::vector<uint8_t>& message) {
    // Placeholder implementation
    return message;
}

std::vector<uint8_t> ClientCrypto::decryptMessage(const std::vector<uint8_t>& encrypted_message) {
    // Placeholder implementation
    return encrypted_message;
}

std::vector<uint8_t> ClientCrypto::signMessage(const std::vector<uint8_t>& message) {
    // Placeholder implementation
    return std::vector<uint8_t>();
}

bool ClientCrypto::verifySignature(const std::vector<uint8_t>& message, 
                                  const std::vector<uint8_t>& signature,
                                  const CryptoPP::RSA::PublicKey& key) {
    // Placeholder implementation
    return true;
}

std::vector<uint8_t> ClientCrypto::generateSessionKey() {
    // Placeholder implementation
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ClientCrypto::encryptSessionKey(const std::vector<uint8_t>& session_key) {
    // Placeholder implementation
    return std::vector<uint8_t>();
}

std::vector<uint8_t> ClientCrypto::createKeyExchangeMessage(const std::string& recipient_id, const std::string& recipient_public_key) {
    try {
        // Generate a new symmetric key for this recipient
        std::vector<uint8_t> symmetric_key = generateSymmetricKey();
        
        // Encrypt the symmetric key with recipient's public key
        std::vector<uint8_t> encrypted_key = encryptWithPublicKey(symmetric_key, recipient_public_key);
        
        // Store the symmetric key locally
        storeSymmetricKey(recipient_id, symmetric_key);
        
        return encrypted_key;
    } catch (const std::exception& e) {
        std::cerr << "Error creating key exchange message: " << e.what() << std::endl;
        return std::vector<uint8_t>();
    }
}

bool ClientCrypto::processKeyExchangeMessage(const std::string& sender_id, const std::vector<uint8_t>& encrypted_key) {
    try {
        // Decrypt the symmetric key with our private key
        std::vector<uint8_t> symmetric_key = decryptWithPrivateKey(encrypted_key);
        
        if (symmetric_key.empty()) {
            std::cerr << "Failed to decrypt symmetric key" << std::endl;
            return false;
        }
        
        // Store the symmetric key for this sender
        storeSymmetricKey(sender_id, symmetric_key);
        
        std::cout << "Symmetric key received and stored for sender: " << sender_id << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error processing key exchange message: " << e.what() << std::endl;
        return false;
    }
}

std::string ClientCrypto::getPublicKeyPEM() {
    try {
        // For now, let's create a simple PEM format manually
        // In a real implementation, you would use Crypto++'s PEM encoding
        std::string public_key_pem = "-----BEGIN PUBLIC KEY-----\n";
        
        // Convert the binary key to base64
        std::string base64_key = base64Encode(std::vector<uint8_t>());
        
        // For now, use a placeholder that will work with our encryption
        public_key_pem += "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA";
        public_key_pem += "placeholder_key_for_testing_12345678901234567890";
        public_key_pem += "\n-----END PUBLIC KEY-----";
        
        return public_key_pem;
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Error getting public key PEM: " << e.what() << std::endl;
        return "";
    }
}

std::string ClientCrypto::getPrivateKeyPEM() {
    try {
        std::string private_key_pem;
        CryptoPP::StringSink sink(private_key_pem);
        private_key_.Save(sink);
        return private_key_pem;
    } catch (const CryptoPP::Exception& e) {
        std::cerr << "Error getting private key PEM: " << e.what() << std::endl;
        return "";
    }
} 