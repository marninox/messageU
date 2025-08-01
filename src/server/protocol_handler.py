"""
Simple protocol handler for MessageU server.
"""

import struct
from typing import Optional, Tuple, Dict, Any, List
from enum import IntEnum


class ProtocolCodes(IntEnum):
    """Protocol message codes."""
    REGISTRATION_REQUEST = 1000
    REGISTRATION_SUCCESS = 1001
    REGISTRATION_FAILURE = 1002
    LOGIN_REQUEST = 2000
    LOGIN_SUCCESS = 2001
    LOGIN_FAILURE = 2002
    SEND_MESSAGE_REQUEST = 3000
    SEND_MESSAGE_SUCCESS = 3001
    SEND_MESSAGE_FAILURE = 3002
    REQUEST_MESSAGES = 4000
    MESSAGES_RESPONSE = 4001
    REQUEST_USERS = 5000
    USERS_RESPONSE = 5001
    REQUEST_PUBLIC_KEY = 5002
    PUBLIC_KEY_RESPONSE = 5003
    SEND_SYMMETRIC_KEY = 5004
    SYMMETRIC_KEY_RESPONSE = 5005
    LOGOUT_REQUEST = 6000
    LOGOUT_SUCCESS = 6001


class ProtocolHandler:
    """Simple protocol handler."""
    
    def __init__(self):
        self.version = 1
        
    def parse_request(self, data: bytes) -> Tuple[Optional[int], Optional[bytes]]:
        """Parse incoming request data.
        Returns: (protocol_code, payload) or (None, None) if invalid
        """
        if len(data) < 9:  # Minimum header size
            return None, None
            
        try:
            # Parse header: version(1) + code(2) + payload_size(2) + checksum(4) = 9 bytes
            version, code, payload_size, checksum = struct.unpack('<BHHI', data[:9])
            
            if version != 1:
                return None, None
                
            if len(data) < 9 + payload_size:
                return None, None
                
            payload = data[9:9 + payload_size]
            return code, payload
            
        except struct.error:
            return None, None
        
    def create_response(self, code: int, payload: bytes = b"") -> bytes:
        """Create a protocol response."""
        # Header: version(1) + code(2) + payload_size(2) + checksum(4) = 9 bytes
        checksum = sum(payload) & 0xFFFFFFFF  # Simple checksum
        header = struct.pack('<BHHI', self.version, code, len(payload), checksum)
        return header + payload
        
    def create_registration_response(self, success: bool, client_id: str = "", message: str = "") -> bytes:
        """Create registration response."""
        if success:
            # Success: client_id(16) + message
            client_id_bytes = client_id.encode('utf-8')[:16].ljust(16, b'\0')
            message_bytes = message.encode('utf-8')
            payload = client_id_bytes + message_bytes
            return self.create_response(ProtocolCodes.REGISTRATION_SUCCESS, payload)
        else:
            # Failure: error message
            payload = message.encode('utf-8')
            return self.create_response(ProtocolCodes.REGISTRATION_FAILURE, payload)
    
    def create_error_response(self, error_message: str) -> bytes:
        """Create error response."""
        payload = error_message.encode('utf-8')
        return self.create_response(ProtocolCodes.REGISTRATION_FAILURE, payload)
    
    def create_users_response(self, users: List[Dict[str, Any]]) -> bytes:
        """Create users response for client list."""
        # Format: number_of_users(4) + for each user: client_id(16) + name(255)
        result = struct.pack('<I', len(users))  # Number of users (4 bytes)
        
        for user in users:
            # Client ID (16 bytes, padded with nulls)
            client_id = user.get('client_id', '')[:16].ljust(16, '\0')
            result += client_id.encode('utf-8')
            
            # Name (255 bytes, padded with nulls)
            name = user.get('name', '')[:255].ljust(255, '\0')
            result += name.encode('utf-8')
        
        return self.create_response(ProtocolCodes.USERS_RESPONSE, result)
    
    def create_public_key_response(self, success: bool, client_id: str = "", public_key: str = "", message: str = "") -> bytes:
        """Create public key response."""
        if success:
            # Success: client_id(16) + public_key(1024) + message
            client_id_bytes = client_id.encode('utf-8')[:16].ljust(16, b'\0')
            # Handle public key as PEM text (utf-8)
            public_key_bytes = public_key.encode('utf-8')[:1024].ljust(1024, b'\0')
            message_bytes = message.encode('utf-8')
            payload = client_id_bytes + public_key_bytes + message_bytes
            return self.create_response(ProtocolCodes.PUBLIC_KEY_RESPONSE, payload)
        else:
            # Failure: error message
            payload = message.encode('utf-8')
            return self.create_response(ProtocolCodes.REGISTRATION_FAILURE, payload)
    
    def create_messages_response(self, messages: List[Dict[str, Any]]) -> bytes:
        """Create messages response for waiting messages."""
        # Format: number_of_messages(4) + for each message: from_client_id(16) + message_id(4) + message_type(1) + content_size(4) + content
        result = struct.pack('<I', len(messages))  # Number of messages (4 bytes)
        
        for message in messages:
            # From client ID (16 bytes, padded with nulls)
            from_client_id = message.get('from_client_id', '')[:16].ljust(16, '\0')
            result += from_client_id.encode('utf-8')
            
            # Message ID (4 bytes, little-endian)
            message_id = message.get('id', 0)
            result += struct.pack('<I', message_id)
            
            # Message type (1 byte)
            message_type = message.get('message_type', 0)
            result += struct.pack('<B', message_type)
            
            # Content (variable length)
            content = message.get('content', '')
            content_bytes = content.encode('utf-8')
            content_size = len(content_bytes)
            result += struct.pack('<I', content_size)  # Content size (4 bytes, little-endian)
            result += content_bytes
        
        return self.create_response(ProtocolCodes.MESSAGES_RESPONSE, result)
    
    def create_send_message_response(self, success: bool, message: str = "") -> bytes:
        """Create send message response."""
        if success:
            # Success: confirmation message
            payload = message.encode('utf-8')
            return self.create_response(ProtocolCodes.SEND_MESSAGE_SUCCESS, payload)
        else:
            # Failure: error message
            payload = message.encode('utf-8')
            return self.create_response(ProtocolCodes.SEND_MESSAGE_FAILURE, payload) 