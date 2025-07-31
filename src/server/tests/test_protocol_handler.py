"""
Simple tests for MessageU server protocol handler.
"""

import sys
import os

# Add the server directory to the path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from protocol_handler import ProtocolHandler, ProtocolHeader, ProtocolCodes


def test_registration_request():
    """Test registration request packing and unpacking."""
    print("Testing registration request...")
    
    handler = ProtocolHandler()
    
    # Test data
    test_data = {
        "username": "testuser",
        "public_key": "test_public_key_12345"
    }
    
    # Pack the data
    payload = handler._pack_registration_request(test_data)
    
    # Verify size (255 + 160 = 415 bytes)
    assert len(payload) == 415, f"Expected 415 bytes, got {len(payload)}"
    
    # Extract username and public key
    username_bytes = payload[:255]
    public_key_bytes = payload[255:415]
    
    username = username_bytes.rstrip(b'\0').decode('utf-8')
    public_key = public_key_bytes.rstrip(b'\0').decode('utf-8')
    
    assert username == test_data["username"], f"Username mismatch: {username} != {test_data['username']}"
    assert public_key == test_data["public_key"], f"Public key mismatch: {public_key} != {test_data['public_key']}"
    
    print("✓ Registration request test passed!")


def test_header_serialization():
    """Test header serialization and deserialization."""
    print("Testing header serialization...")
    
    # Create header
    header = ProtocolHeader(version=1, code=1000, payload_size=100, payload_checksum=12345)
    
    # Serialize
    header_bytes = header.to_bytes()
    
    # Verify size (1 + 2 + 2 + 4 = 9 bytes)
    assert len(header_bytes) == 9, f"Expected 9 bytes, got {len(header_bytes)}"
    
    # Deserialize
    deserialized_header = ProtocolHeader.from_bytes(header_bytes)
    
    assert deserialized_header is not None, "Failed to deserialize header"
    assert deserialized_header.version == 1, "Version mismatch"
    assert deserialized_header.code == 1000, "Code mismatch"
    assert deserialized_header.payload_size == 100, "Payload size mismatch"
    assert deserialized_header.payload_checksum == 12345, "Checksum mismatch"
    
    print("✓ Header serialization test passed!")


if __name__ == "__main__":
    print("Running simple protocol handler tests...")
    test_header_serialization()
    test_registration_request()
    print("All tests passed!") 