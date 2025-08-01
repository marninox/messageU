#!/usr/bin/env python3
"""
MessageU Server - Main entry point
Handles server initialization, client connections, and protocol processing.
"""

import socket
import threading
import sys
import os
from client import ClientConnection, ClientManager
from message import Message, MessageManager, MessageFormatter
from db_handler import DatabaseHandler
from protocol_handler import ProtocolHandler

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
                # Receive data from client
                data = client_socket.recv(1024)
                if not data:
                    print(f"Client {client_address} disconnected")
                    break
                
                # Parse the request
                code, payload = self.protocol_handler.parse_request(data)
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
            if len(payload) < 415:  # username(255) + public_key(160)
                return self.protocol_handler.create_error_response("Invalid registration payload")
            
            username_bytes = payload[:255]
            public_key_bytes = payload[255:415]
            
            username = username_bytes.rstrip(b'\0').decode('utf-8')
            public_key = public_key_bytes.rstrip(b'\0').decode('utf-8')
            
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
        # Placeholder for message sending
        return self.protocol_handler.create_error_response("Send message not implemented yet")
    
    def handle_request_messages(self, payload):
        """Handle request for waiting messages."""
        try:
            # For now, we'll use a simple approach: the requesting client is identified
            # by their connection. In a real implementation, this would be based on
            # authentication or session management.
            
            # Since we don't have authentication yet, we'll need to identify the client
            # from the payload or connection context. For simplicity, let's assume
            # the payload contains the client ID or we can derive it from the connection.
            
            # For this implementation, we'll use a placeholder approach
            # In a real system, this would be based on the authenticated user
            print("Waiting messages request received")
            
            # For testing purposes, let's get messages for the first registered client
            # In a real implementation, this would be the authenticated user's ID
            clients = self.database.get_all_clients()
            if not clients:
                print("No clients found for message retrieval")
                return self.protocol_handler.create_messages_response([])
            
            # Use the first client as the recipient (for testing)
            recipient_id = clients[0]['client_id']
            print(f"Getting waiting messages for client: {recipient_id}")
            
            # Get waiting messages for this client
            messages = self.database.get_waiting_messages(recipient_id)
            
            if messages:
                print(f"Found {len(messages)} waiting messages")
                # Mark messages as delivered by deleting them
                message_ids = [msg['id'] for msg in messages]
                self.database.delete_messages(message_ids)
                return self.protocol_handler.create_messages_response(messages)
            else:
                print("No waiting messages found")
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
    
    def handle_logout_request(self, payload):
        """Handle logout request."""
        # Placeholder for logout
        return self.protocol_handler.create_error_response("Logout not implemented yet")

def main():
    server = MessageUServer()
    server.start()

if __name__ == "__main__":
    main()
