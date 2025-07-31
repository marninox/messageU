#!/usr/bin/env python3
"""
MessageU Server - Main entry point
Handles server initialization, client connections, and protocol processing.
"""

import socket
import threading
import sys
import os
from typing import Optional

# Import our modules
from client import ClientManager
from message import MessageManager
from protocol_handler import ProtocolHandler
from db_handler import DatabaseHandler, InMemoryStorage


class MessageUServer:
    """Main server class for MessageU."""
    
    def __init__(self, host: str = '0.0.0.0', port: int = 1357):
        self.host = host
        self.port = port
        self.server_socket: Optional[socket.socket] = None
        self.running = False
        self.client_threads: list = []  # Track client threads for cleanup
        
        # Initialize components
        self.client_manager = ClientManager()
        self.message_manager = MessageManager()
        self.protocol_handler = ProtocolHandler()
        self.db_handler: Optional[DatabaseHandler] = None
        self.storage = InMemoryStorage()
        
    def initialize(self) -> bool:
        """Initialize the server and create listening socket."""
        try:
            # Create TCP socket (IPv4)
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            
            # Set socket options for reuse
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            
            # Bind to the specified host and port
            self.server_socket.bind((self.host, self.port))
            
            # Start listening for connections
            self.server_socket.listen(5)  # Allow up to 5 pending connections
            
            print(f"MessageU Server initialized successfully")
            print(f"Listening on {self.host}:{self.port}")
            
            return True
            
        except socket.error as e:
            print(f"Failed to initialize server socket: {e}")
            return False
        except Exception as e:
            print(f"Unexpected error during server initialization: {e}")
            return False
        
    def start(self):
        """Start the server and accept client connections."""
        if not self.server_socket:
            print("Server not initialized. Call initialize() first.")
            return
            
        self.running = True
        print("Server started. Accepting connections...")
        
        try:
            while self.running:
                # Accept incoming connections
                client_socket, client_address = self.server_socket.accept()
                print(f"New connection from {client_address}")
                
                # Create a new thread to handle this client
                client_thread = threading.Thread(
                    target=self.handle_client_wrapper,
                    args=(client_socket, client_address),
                    daemon=True  # Thread will be terminated when main thread exits
                )
                
                # Store thread reference for cleanup
                self.client_threads.append(client_thread)
                
                # Start the client handling thread
                client_thread.start()
                
                # Clean up completed threads
                self.cleanup_finished_threads()
                
        except socket.error as e:
            if self.running:  # Only print error if we're supposed to be running
                print(f"Socket error: {e}")
        except KeyboardInterrupt:
            print("\nReceived shutdown signal...")
        except Exception as e:
            print(f"Unexpected error in server loop: {e}")
        finally:
            self.stop()
        
    def stop(self):
        """Stop the server and cleanup resources."""
        print("Stopping server...")
        self.running = False
        
        # Close server socket
        if self.server_socket:
            try:
                self.server_socket.close()
                print("Server socket closed")
            except Exception as e:
                print(f"Error closing server socket: {e}")
        
        # Wait for client threads to finish (with timeout)
        print("Waiting for client threads to finish...")
        for thread in self.client_threads:
            if thread.is_alive():
                thread.join(timeout=2.0)  # Wait up to 2 seconds per thread
        
        print("Server stopped")
        
    def handle_client_wrapper(self, client_socket: socket.socket, address: tuple):
        """Wrapper function to handle client connections with error handling."""
        try:
            print(f"Starting client handler for {address}")
            self.handle_client(client_socket, address)
        except Exception as e:
            print(f"Error handling client {address}: {e}")
        finally:
            # Ensure client socket is closed
            try:
                client_socket.close()
                print(f"Client connection closed: {address}")
            except Exception as e:
                print(f"Error closing client socket {address}: {e}")
        
    def handle_client(self, client_socket: socket.socket, address: tuple):
        """Handle individual client connection."""
        # TODO: Implement protocol handling logic
        # This will be implemented in future steps
        print(f"Client handler placeholder for {address}")
        
        # For now, just keep the connection alive
        while self.running:
            try:
                # Set a timeout so we can check if server is still running
                client_socket.settimeout(1.0)
                
                # Try to receive data (this will be implemented later)
                data = client_socket.recv(1024)
                if not data:
                    print(f"Client {address} disconnected")
                    break
                    
                # TODO: Process received data according to protocol
                print(f"Received {len(data)} bytes from {address}")
                
            except socket.timeout:
                # Timeout is expected, continue loop
                continue
            except socket.error as e:
                print(f"Socket error with client {address}: {e}")
                break
            except Exception as e:
                print(f"Unexpected error with client {address}: {e}")
                break
                
    def cleanup_finished_threads(self):
        """Remove completed threads from the tracking list."""
        self.client_threads = [thread for thread in self.client_threads if thread.is_alive()]


def load_port_from_file() -> int:
    """Load port number from myport.info file."""
    port_file = "myport.info"
    default_port = 1357
    
    try:
        if os.path.exists(port_file):
            with open(port_file, 'r') as f:
                port_str = f.read().strip()
                port = int(port_str)
                
                # Validate port range
                if 1 <= port <= 65535:
                    print(f"Loaded port {port} from {port_file}")
                    return port
                else:
                    print(f"Invalid port {port} in {port_file}, using default {default_port}")
                    return default_port
        else:
            print(f"Port file {port_file} not found, using default port {default_port}")
            return default_port
            
    except (ValueError, IOError) as e:
        print(f"Error reading port from {port_file}: {e}")
        print(f"Using default port {default_port}")
        return default_port


if __name__ == "__main__":
    # Load port from myport.info or fallback to 1357
    port = load_port_from_file()
    
    # Initialize server
    server = MessageUServer(port=port)
    
    if not server.initialize():
        print("Failed to initialize server")
        sys.exit(1)
    
    try:
        print(f"Starting MessageU server on port {port}")
        server.start()
    except KeyboardInterrupt:
        print("\nShutting down server...")
        server.stop()
    except Exception as e:
        print(f"Server error: {e}")
        sys.exit(1)
