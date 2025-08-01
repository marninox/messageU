# MessageU - End-to-End Encrypted Messaging System

A secure client-server messaging application with end-to-end encryption.

## Features

- **End-to-End Encryption**: RSA-2048 for key exchange, AES-128-CBC for messages
- **User Registration**: Secure key generation and storage
- **Client Discovery**: List registered users and retrieve public keys
- **Secure Messaging**: Encrypted message exchange with automatic key management
- **Database Storage**: SQLite persistence for users and messages (bonus implementation)

## Architecture

- **Client**: C++11+ with Boost.Asio networking and Crypto++ cryptography
- **Server**: Python 3 with SQLite database backend
- **Protocol**: Binary protocol with checksums and proper error handling

## Building

```bash
make client
```

## Running

1. Start the server:
```bash
cd src/server && python3 main.py
```

2. Run the client:
```bash
./messageu_client
```

## Configuration

- `server.info`: Server IP and port
- `myport.info`: Server listening port (default: 8888)
- `me.info`: Client identity and keys (auto-generated on registration)
