#!/usr/bin/env python3
"""
MessageU Server - Main entry point
Handles server initialization, client connections, and protocol processing.
Version: 2.0 (with bonus database implementation)
***worked on MAC environment***

Features:
- Multi-client TCP server with threading
- Binary protocol handling
- SQLite database for persistent storage (bonus)
- End-to-end encryption support (server stores encrypted data only)
"""

import socket
import threading
import sys
import os
from client import ClientConnection, ClientManager
from message import Message, MessageManager, MessageFormatter
from db_handler import DatabaseHandler
from protocol_handler import ProtocolHandler, ProtocolCodes
import struct

# Add the server directory to the path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

class MessageUServer:
    def __init__(self):
        self.host = '0.0.0.0'
        self.port = 1357  # Default port
        self.server_socket = None
        self.running = False
        self.client_manager = ClientManager()
        self.message_manager = MessageManager()
        self.message_formatter = MessageFormatter()
        self.database = DatabaseHandler()
        self.protocol_handler = ProtocolHandler()
        
    def load_port_from_file(self):
        """Load port number from myport.info file."""
        try:
            with open('myport.info', 'r') as f:
                port_str = f.read().strip()
                self.port = int(port_str)
                print(f"Loaded port from myport.info: {self.port}")
        except FileNotFoundError:
            print(f"myport.info not found, using default port: {self.port}")
        except ValueError:
            print(f"Invalid port in myport.info, using default port: {self.port}")
        except Exception as e:
            print(f"Error reading myport.info: {e}, using default port: {self.port}")
    
    def initialize(self):
        """Initialize the server."""
        print("Initializing MessageU Server...")
        self.load_port_from_file()
        
        # Create TCP socket
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(5)
            print(f"Server listening on {self.host}:{self.port}")
            return True
        except socket.error as e:
            print(f"Failed to bind to {self.host}:{self.port}: {e}")
            return False
    
    def start(self):
        """Start the server and accept connections."""
        if not self.initialize():
            return
        
        self.running = True
        print("Server started. Waiting for connections...")
        
        try:
            while self.running:
                try:
                    client_socket, client_address = self.server_socket.accept()
                    print(f"New connection from {client_address}")
                    
                    # Spawn a new thread for each client
                    client_thread = threading.Thread(
                        target=self.handle_client_wrapper,
                        args=(client_socket, client_address)
                    )
                    client_thread.daemon = True
                    client_thread.start()
                    
                except socket.error as e:
                    if self.running:
                        print(f"Socket error: {e}")
                    break
                    
        except KeyboardInterrupt:
            print("\nShutdown signal received...")
        finally:
            self.stop()
    
    def stop(self):
        """Stop the server."""
        print("Stopping server...")
        self.running = False
        
        if self.server_socket:
            self.server_socket.close()
        
        self.database.close()
        print("Server stopped.")
    
    def handle_client_wrapper(self, client_socket, client_address):
        """Wrapper for client handling with error protection."""
        try:
            self.handle_client(client_socket, client_address)
        except Exception as e:
            print(f"Error handling client {client_address}: {e}")
        finally:
            try:
                client_socket.close()
            except:
                pass
    
    def handle_client(self, client_socket, client_address):
        """Handle communication with a client."""
        print(f"Handling client: {client_address}")
        
        try:
            while self.running:
                # Read header first (9 bytes)
                header = b''
                while len(header) < 9:
                    chunk = client_socket.recv(9 - len(header))
                    if not chunk:
                        print(f"Client {client_address} disconnected")
                        return
                    header += chunk

                version, code, payload_size, checksum = struct.unpack('<BHHI', header)

                # Now read the payload
                payload = b''
                while len(payload) < payload_size:
                    chunk = client_socket.recv(payload_size - len(payload))
                    if not chunk:
                        print(f"Client {client_address} disconnected")
                        return
                    payload += chunk

                # Now process the request
                full_data = header + payload
                code, payload = self.protocol_handler.parse_request(full_data)
                if not code:
                    print(f"Invalid request from {client_address}")
                    continue
                
                # Handle the request based on protocol code
                response = self.handle_protocol_request(code, payload, client_address)
                
                # Send response back to client
                if response:
                    client_socket.send(response)
                    
        except socket.error as e:
            print(f"Socket error with client {client_address}: {e}")
        except Exception as e:
            print(f"Error handling client {client_address}: {e}")
    
    def handle_protocol_request(self, header, payload, client_address):
        """Handle different protocol requests."""
        try:
            if header == 1000:  # Registration request
                return self.handle_registration_request(payload)
            elif header == 2000:  # Login request
                return self.handle_login_request(payload)
            elif header == 3000:  # Send message request
                return self.handle_send_message_request(payload)
            elif header == 4000:  # Request messages
                return self.handle_request_messages(payload)
            elif header == 5000:  # Request users
                return self.handle_request_users(payload)
            elif header == 5002:  # Request public key
                return self.handle_get_public_key_request(payload)
            elif header == 5004:  # Send symmetric key
                return self.handle_send_symmetric_key_request(payload)
            elif header == 6000:  # Logout request
                return self.handle_logout_request(payload)
            else:
                print(f"Unknown protocol code: {header}")
                return self.protocol_handler.create_error_response("Unknown protocol code")
                
        except Exception as e:
            print(f"Error handling protocol request: {e}")
            return self.protocol_handler.create_error_response("Internal server error")
    
    def handle_registration_request(self, payload):
        """Handle client registration request."""
        try:
            # Parse registration data from payload
            if len(payload) < 1279:  # username(255) + public_key(1024)
                return self.protocol_handler.create_error_response("Invalid registration payload")
            
            username_bytes = payload[:255]
            public_key_bytes = payload[255:1279]
            
            # Username is utf-8, public key is binary (store as latin1)
            username = username_bytes.rstrip(b'\0').decode('utf-8', errors='replace')
            # Handle public key as PEM text (utf-8)
            public_key = public_key_bytes.rstrip(b'\0').decode('utf-8', errors='replace')
            
            # Generate a simple client ID (in real implementation, this would be a proper UUID)
            import hashlib
            client_id = hashlib.md5(f"{username}{public_key}".encode()).hexdigest()[:16]
            
            # Try to register in database
            if self.database.register_client(client_id, username, public_key):
                print(f"Registration successful for {username} (ID: {client_id})")
                return self.protocol_handler.create_registration_response(True, client_id, "Registration successful")
            else:
                print(f"Registration failed for {username} - already exists")
                return self.protocol_handler.create_registration_response(False, "", "Client already exists")
                
        except Exception as e:
            print(f"Error in registration: {e}")
            return self.protocol_handler.create_error_response("Registration failed")
    
    def handle_login_request(self, payload):
        """Handle client login request."""
        # Placeholder for login handling
        return self.protocol_handler.create_error_response("Login not implemented yet")
    
    def handle_send_message_request(self, payload):
        """Handle send message request."""
        try:
            # Parse send message data from payload
            # Format: sender_id(16) + recipient(255) + message_length(4) + message_content
            if len(payload) < 275:  # sender_id(16) + recipient(255) + message_length(4)
                return self.protocol_handler.create_error_response("Invalid send message payload")
            
            # Parse sender ID (16 bytes)
            sender_id_bytes = payload[:16]
            sender_id = sender_id_bytes.rstrip(b'\0').decode('utf-8')
            
            # Parse recipient (255 bytes)
            recipient_bytes = payload[16:271]
            recipient = recipient_bytes.rstrip(b'\0').decode('utf-8')
            
            if not recipient:
                return self.protocol_handler.create_error_response("Empty recipient")
            
            # Parse message length (4 bytes, little-endian)
            message_length = int.from_bytes(payload[271:275], byteorder='little')
            
            if len(payload) < 275 + message_length:
                return self.protocol_handler.create_error_response("Invalid message content length")
            
            # Parse message content (binary data, not UTF-8 text)
            message_content_bytes = payload[275:275 + message_length]
            # Convert binary data to base64 string for storage to preserve binary data
            import base64
            message_content = base64.b64encode(message_content_bytes).decode('ascii')
            
            print(f"Send message request: from {sender_id} to {recipient}")
            print(f"Message content: {message_content}")
            
            # Validate that recipient exists
            recipient_client = self.database.get_client_by_identifier(recipient)
            if not recipient_client:
                print(f"Recipient not found: {recipient}")
                return self.protocol_handler.create_send_message_response(
                    False, 
                    f"Recipient '{recipient}' not found"
                )
            
            print(f"Recipient found: {recipient_client['name']} (ID: {recipient_client['client_id']})")
            
            # Store the message in the database with actual sender ID
            if self.database.store_message(sender_id, recipient_client['client_id'], 1, message_content):
                print(f"Message stored successfully for {recipient_client['name']}")
                return self.protocol_handler.create_send_message_response(
                    True, 
                    f"Message sent successfully to {recipient_client['name']}"
                )
            else:
                print("Failed to store message in database")
                return self.protocol_handler.create_send_message_response(
                    False, 
                    "Failed to store message"
                )
                
        except Exception as e:
            print(f"Error in send message request: {e}")
            return self.protocol_handler.create_error_response("Failed to send message")
    
    def handle_request_messages(self, payload):
        """Handle request for waiting messages."""
        try:
            # Parse waiting messages request
            # Format: client_id(16) - the requesting client's ID
            if len(payload) < 16:
                return self.protocol_handler.create_error_response("Invalid waiting messages request")
            
            # Parse client ID (16 bytes)
            client_id_bytes = payload[:16]
            client_id = client_id_bytes.rstrip(b'\0').decode('utf-8')
            
            print(f"Waiting messages request received from client: {client_id}")
            
            # Get waiting messages for this specific client
            messages = self.database.get_waiting_messages(client_id)
            
            if messages:
                print(f"Found {len(messages)} waiting messages for {client_id}")
                # Mark messages as delivered by deleting them
                message_ids = [msg['id'] for msg in messages]
                self.database.delete_messages(message_ids)
                return self.protocol_handler.create_messages_response(messages)
            else:
                print(f"No waiting messages found for {client_id}")
                return self.protocol_handler.create_messages_response([])
                
        except Exception as e:
            print(f"Error in waiting messages request: {e}")
            return self.protocol_handler.create_error_response("Failed to get waiting messages")
    
    def handle_request_users(self, payload):
        """Handle request for user list."""
        try:
            # Get all clients from database
            clients = self.database.get_all_clients()
            
            if clients:
                print(f"Returning {len(clients)} clients to requesting user")
                return self.protocol_handler.create_users_response(clients)
            else:
                print("No clients found in database")
                return self.protocol_handler.create_users_response([])
                
        except Exception as e:
            print(f"Error getting users list: {e}")
            return self.protocol_handler.create_error_response("Failed to get users list")
    
    def handle_get_public_key_request(self, payload):
        """Handle request for public key."""
        try:
            # Parse the requested client identifier from payload
            if len(payload) < 255:  # client_identifier(255)
                return self.protocol_handler.create_error_response("Invalid public key request payload")
            
            identifier_bytes = payload[:255]
            identifier = identifier_bytes.rstrip(b'\0').decode('utf-8')
            
            if not identifier:
                return self.protocol_handler.create_error_response("Empty client identifier")
            
            print(f"Public key request for client: {identifier}")
            
            # Look up the client in database
            client = self.database.get_client_by_identifier(identifier)
            
            if client:
                print(f"Found client: {client['name']} (ID: {client['client_id']})")
                return self.protocol_handler.create_public_key_response(
                    True, 
                    client['client_id'], 
                    client['public_key'], 
                    f"Public key for {client['name']}"
                )
            else:
                print(f"Client not found: {identifier}")
                return self.protocol_handler.create_public_key_response(
                    False, 
                    "", 
                    "", 
                    f"Client '{identifier}' not found"
                )
                
        except Exception as e:
            print(f"Error in public key request: {e}")
            return self.protocol_handler.create_error_response("Failed to get public key")
    
    def handle_send_symmetric_key_request(self, payload):
        """Handle symmetric key exchange request."""
        try:
            # Parse symmetric key data from payload
            # Format: sender_id(16) + recipient(255) + key_length(4) + encrypted_key
            if len(payload) < 275:  # sender_id(16) + recipient(255) + key_length(4)
                return self.protocol_handler.create_error_response("Invalid symmetric key request payload")
            
            # Parse sender ID (16 bytes)
            sender_id_bytes = payload[:16]
            sender_id = sender_id_bytes.rstrip(b'\0').decode('utf-8')
            
            # Parse recipient (255 bytes)
            recipient_bytes = payload[16:271]
            recipient = recipient_bytes.rstrip(b'\0').decode('utf-8')
            
            if not recipient:
                return self.protocol_handler.create_error_response("Empty recipient")
            
            # Parse key length (4 bytes, little-endian)
            key_length = int.from_bytes(payload[271:275], byteorder='little')
            
            if len(payload) < 275 + key_length:
                return self.protocol_handler.create_error_response("Invalid encrypted key length")
            
            # Extract encrypted key
            encrypted_key = payload[275:275 + key_length]
            
            print(f"Symmetric key request: from {sender_id} to {recipient}")
            print(f"Encrypted key length: {key_length} bytes")
            
            # Validate that recipient exists
            recipient_client = self.database.get_client_by_identifier(recipient)
            if not recipient_client:
                print(f"Recipient not found: {recipient}")
                return self.protocol_handler.create_error_response(f"Recipient '{recipient}' not found")
            
            print(f"Recipient found: {recipient_client['name']} (ID: {recipient_client['client_id']})")
            
            # Convert encrypted key to base64 string for storage
            import base64
            encrypted_key_str = base64.b64encode(encrypted_key).decode('ascii')
            
            if self.database.store_message(sender_id, recipient_client['client_id'], 2, encrypted_key_str):
                print(f"Symmetric key stored successfully for {recipient_client['name']}")
                return self.protocol_handler.create_response(ProtocolCodes.SYMMETRIC_KEY_RESPONSE, b"Symmetric key received")
            else:
                print("Failed to store symmetric key in database")
                return self.protocol_handler.create_error_response("Failed to store symmetric key")
                
        except Exception as e:
            print(f"Error in symmetric key request: {e}")
            return self.protocol_handler.create_error_response("Failed to process symmetric key")
    
    def handle_logout_request(self, payload):
        """Handle logout request."""
        # Placeholder for logout
        return self.protocol_handler.create_error_response("Logout not implemented yet")

def main():
    server = MessageUServer()
    server.start()

if __name__ == "__main__":
    main()
