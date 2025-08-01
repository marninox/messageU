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