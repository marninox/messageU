# 🔐 MessageU Encryption Test Guide

## How to Test Encryption with Two New Users

### **Prerequisites:**
- Server running on port 8888
- Client compiled and ready

### **Step 1: Start the Server**
```bash
cd src/server && python3 main.py
```

### **Step 2: Register First User (Alice)**
Open a new terminal and run:
```bash
./messageu_client
```

Then follow these steps:
1. Enter `1` (Register)
2. Enter username: `Alice`
3. Wait for "Registration successful!"
4. Note the Client ID (e.g., `363ed056cb45ab74`)
5. Enter `7` (Exit)

### **Step 3: Register Second User (Bob)**
Open another new terminal and run:
```bash
./messageu_client
```

Then follow these steps:
1. Enter `1` (Register)
2. Enter username: `Bob`
3. Wait for "Registration successful!"
4. Note the Client ID
5. Enter `7` (Exit)

### **Step 4: Test Client List**
From Bob's terminal:
```bash
./messageu_client
```

1. Enter `2` (Request client list)
2. You should see both Alice and Bob listed
3. Enter `7` (Exit)

### **Step 5: Test Public Key Retrieval**
From Bob's terminal:
```bash
./messageu_client
```

1. Enter `3` (Get public key)
2. Enter `Alice` (or Alice's Client ID)
3. You should see Alice's public key in PEM format
4. Enter `7` (Exit)

### **Step 6: Test Key Exchange and Message Sending**
From Bob's terminal:
```bash
./messageu_client
```

1. Enter `5` (Send message)
2. Enter `Alice` (recipient)
3. You should see:
   - "No symmetric key found for recipient: Alice"
   - "Initiating key exchange..."
   - "Encrypting symmetric key with recipient's public key..."
   - "Symmetric key sent successfully!"
   - "Key exchange completed successfully!"
4. Enter your message: `Hello Alice! This is an encrypted message from Bob!`
5. You should see:
   - "Message encrypted successfully."
   - "✓ Message sent successfully to Alice"
6. Enter `7` (Exit)

### **Step 7: Test Message Retrieval (Alice's Side)**
From Alice's terminal:
```bash
./messageu_client
```

1. Enter `4` (Get waiting messages)
2. You should see the encrypted message from Bob
3. Enter `7` (Exit)

## **Expected Results:**

### **✅ Successful Test Indicators:**
- Registration: "Registration successful!"
- Client List: Shows both users
- Public Key: Shows PEM format key
- Key Exchange: "Symmetric key sent successfully!"
- Message Sending: "✓ Message sent successfully to Alice"
- Message Retrieval: Shows encrypted message

### **🔍 What You're Testing:**
1. **RSA Key Generation** - 2048-bit keys created
2. **Public Key Exchange** - Keys retrieved from server
3. **Symmetric Key Generation** - AES-128 keys created
4. **Key Exchange Protocol** - RSA-encrypted symmetric key exchange
5. **Message Encryption** - AES-encrypted messages
6. **End-to-End Security** - Server never sees plaintext

### **🎯 Server Logs to Watch:**
- "Client registered: [username]"
- "Public key request for client: [username]"
- "Symmetric key request: from <unknown> to [username]"
- "Message stored: from unknown_sender to [client_id]"

## **Troubleshooting:**

### **If Registration Fails:**
- Check if username already exists
- Try a different username
- Restart server if needed

### **If Key Exchange Fails:**
- Ensure server is running
- Check network connectivity
- Verify protocol codes are working

### **If Message Sending Fails:**
- Check if key exchange completed
- Verify recipient exists
- Check server logs for errors

## **🎉 Success Criteria:**
- Both users can register
- Both users can see each other in client list
- Public keys can be retrieved
- Key exchange completes successfully
- Encrypted messages can be sent and received
- Server stores encrypted data (never plaintext)

**This tests the complete end-to-end encryption flow!** 🚀 