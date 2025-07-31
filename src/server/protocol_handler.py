"""
Protocol handler for MessageU server.
Handles binary protocol parsing, message creation, and response formatting.
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
    LOGOUT_REQUEST = 6000
    LOGOUT_SUCCESS = 6001


class ProtocolHeader:
    """Protocol message header structure."""
    
    def __init__(self, version: int = 1, code: int = 0, payload_size: int = 0, 
                 payload_checksum: int = 0):
        self.version = version
        self.code = code
        self.payload_size = payload_size
        self.payload_checksum = payload_checksum
        
    def to_bytes(self) -> bytes:
        """Serialize header to bytes (little-endian)."""
        return struct.pack('<BHHI', self.version, self.code, self.payload_size, self.payload_checksum)
        
    @classmethod
    def from_bytes(cls, data: bytes) -> Optional['ProtocolHeader']:
        """Deserialize header from bytes (little-endian)."""
        if len(data) < 9:
            return None
        try:
            version, code, payload_size, payload_checksum = struct.unpack('<BHHI', data[:9])
            return cls(version, code, payload_size, payload_checksum)
        except struct.error:
            return None


class ProtocolHandler:
    """Handles protocol message parsing and creation."""
    
    def __init__(self):
        self.version = 1
        
    def parse_request(self, data: bytes) -> Tuple[Optional[ProtocolHeader], Optional[bytes]]:
        """Parse incoming request data."""
        if len(data) < 9:
            return None, None
            
        header = ProtocolHeader.from_bytes(data)
        if not header:
            return None, None
            
        if len(data) < 9 + header.payload_size:
            return None, None
            
        payload = data[9:9 + header.payload_size]
        return header, payload
        
    def create_response(self, code: int, payload: bytes = b"") -> bytes:
        """Create a protocol response."""
        checksum = self.calculate_checksum(payload)
        header = ProtocolHeader(self.version, code, len(payload), checksum)
        return header.to_bytes() + payload
        
    def create_registration_response(self, success: bool, error_message: str = "") -> bytes:
        """Create registration response."""
        if success:
            return self.create_response(ProtocolCodes.REGISTRATION_SUCCESS)
        else:
            payload = self._pack_error_response(error_message)
            return self.create_response(ProtocolCodes.REGISTRATION_FAILURE, payload)
        
    def create_login_response(self, success: bool, error_message: str = "") -> bytes:
        """Create login response."""
        if success:
            return self.create_response(ProtocolCodes.LOGIN_SUCCESS)
        else:
            payload = self._pack_error_response(error_message)
            return self.create_response(ProtocolCodes.LOGIN_FAILURE, payload)
        
    def create_send_message_response(self, success: bool, message_id: int = 0, 
                                   error_message: str = "") -> bytes:
        """Create send message response."""
        if success:
            payload = struct.pack('<I', message_id)
            return self.create_response(ProtocolCodes.SEND_MESSAGE_SUCCESS, payload)
        else:
            payload = self._pack_error_response(error_message)
            return self.create_response(ProtocolCodes.SEND_MESSAGE_FAILURE, payload)
        
    def create_messages_response(self, messages: List[Dict[str, Any]]) -> bytes:
        """Create messages response."""
        payload = self._pack_messages_response(messages)
        return self.create_response(ProtocolCodes.MESSAGES_RESPONSE, payload)
        
    def create_users_response(self, users: List[str]) -> bytes:
        """Create users response."""
        payload = self._pack_users_response(users)
        return self.create_response(ProtocolCodes.USERS_RESPONSE, payload)
        
    def create_logout_response(self, success: bool) -> bytes:
        """Create logout response."""
        return self.create_response(ProtocolCodes.LOGOUT_SUCCESS)
        
    def calculate_checksum(self, data: bytes) -> int:
        """Calculate checksum for data (simple sum of bytes)."""
        return sum(data) & 0xFFFFFFFF
        
    def validate_checksum(self, data: bytes, checksum: int) -> bool:
        """Validate data checksum."""
        return self.calculate_checksum(data) == checksum
        
    def extract_payload(self, data: bytes) -> Optional[bytes]:
        """Extract payload from protocol message."""
        if len(data) < 9:
            return None
        header = ProtocolHeader.from_bytes(data)
        if not header:
            return None
        if len(data) < 9 + header.payload_size:
            return None
        return data[9:9 + header.payload_size]
        
    def create_payload(self, code: int, data: Dict[str, Any]) -> bytes:
        """Create payload for specific message type."""
        if code == ProtocolCodes.REGISTRATION_REQUEST:
            return self._pack_registration_request(data)
        elif code == ProtocolCodes.LOGIN_REQUEST:
            return self._pack_login_request(data)
        elif code == ProtocolCodes.SEND_MESSAGE_REQUEST:
            return self._pack_send_message_request(data)
        elif code == ProtocolCodes.REQUEST_MESSAGES:
            return b""  # No payload
        elif code == ProtocolCodes.REQUEST_USERS:
            return b""  # No payload
        elif code == ProtocolCodes.LOGOUT_REQUEST:
            return b""  # No payload
        else:
            return b""
    
    def _pack_registration_request(self, data: Dict[str, Any]) -> bytes:
        """Pack registration request payload."""
        username = data.get("username", "")
        public_key = data.get("public_key", "")
        
        # Username: 255 bytes (padded with nulls)
        username_bytes = username.encode('utf-8')[:255].ljust(255, b'\0')
        
        # Public key: 160 bytes (padded with nulls)
        public_key_bytes = public_key.encode('utf-8')[:160].ljust(160, b'\0')
        
        return username_bytes + public_key_bytes
    
    def _pack_login_request(self, data: Dict[str, Any]) -> bytes:
        """Pack login request payload."""
        username = data.get("username", "")
        
        # Username: 255 bytes (padded with nulls)
        username_bytes = username.encode('utf-8')[:255].ljust(255, b'\0')
        
        return username_bytes
    
    def _pack_send_message_request(self, data: Dict[str, Any]) -> bytes:
        """Pack send message request payload."""
        recipient = data.get("recipient", "")
        message = data.get("message", b"")
        message_type = data.get("message_type", 0)
        
        # Recipient: 255 bytes (padded with nulls)
        recipient_bytes = recipient.encode('utf-8')[:255].ljust(255, b'\0')
        
        # Message type: 1 byte
        message_type_bytes = struct.pack('<B', message_type)
        
        # Message content: 4 bytes length + content
        message_length = len(message)
        message_length_bytes = struct.pack('<I', message_length)
        
        return recipient_bytes + message_type_bytes + message_length_bytes + message
    
    def _pack_error_response(self, error_message: str) -> bytes:
        """Pack error response payload."""
        # Error message: 255 bytes (padded with nulls)
        error_bytes = error_message.encode('utf-8')[:255].ljust(255, b'\0')
        return error_bytes
    
    def _pack_messages_response(self, messages: List[Dict[str, Any]]) -> bytes:
        """Pack messages response payload."""
        # Number of messages: 4 bytes
        num_messages = len(messages)
        result = struct.pack('<I', num_messages)
        
        for message in messages:
            # Each message: sender(255) + content_length(4) + content
            sender = message.get("sender", "")
            content = message.get("content", b"")
            
            sender_bytes = sender.encode('utf-8')[:255].ljust(255, b'\0')
            content_length = len(content)
            content_length_bytes = struct.pack('<I', content_length)
            
            result += sender_bytes + content_length_bytes + content
        
        return result
    
    def _pack_users_response(self, users: List[str]) -> bytes:
        """Pack users response payload."""
        # Number of users: 4 bytes
        num_users = len(users)
        result = struct.pack('<I', num_users)
        
        for user in users:
            # Each user: username(255)
            username_bytes = user.encode('utf-8')[:255].ljust(255, b'\0')
            result += username_bytes
        
        return result 