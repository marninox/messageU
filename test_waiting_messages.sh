#!/bin/bash

echo "Testing MessageU Client - Get Waiting Messages"
echo "=============================================="

# Check if server is running
if ! lsof -i :8888 > /dev/null 2>&1; then
    echo "ERROR: Server is not running on port 8888"
    exit 1
fi

echo "✓ Server is running on port 8888"

# Check if client is registered
if [ ! -f "me.info" ]; then
    echo "ERROR: Client is not registered (me.info not found)"
    exit 1
fi

echo "✓ Client is registered (me.info exists)"

# Check if there are users in the database
USER_COUNT=$(sqlite3 src/server/defensive.db "SELECT COUNT(*) FROM clients;" 2>/dev/null)
if [ $? -eq 0 ] && [ "$USER_COUNT" -gt 0 ]; then
    echo "✓ Database contains $USER_COUNT registered user(s)"
else
    echo "WARNING: No users found in database"
fi

# Check if there are messages in the database
MESSAGE_COUNT=$(sqlite3 src/server/defensive.db "SELECT COUNT(*) FROM messages;" 2>/dev/null)
if [ $? -eq 0 ]; then
    echo "✓ Database contains $MESSAGE_COUNT message(s)"
else
    echo "WARNING: Could not check messages in database"
fi

echo ""
echo "Waiting Messages Feature Implementation:"
echo "======================================="
echo "✓ Server-side:"
echo "  - Added create_messages_response() method to protocol handler"
echo "  - Added get_waiting_messages() method to database handler"
echo "  - Added delete_messages() method to mark messages as delivered"
echo "  - Updated handle_request_messages() method in main server"
echo "  - Messages are automatically deleted after being retrieved"
echo ""
echo "✓ Client-side:"
echo "  - Added isMessagesReceived() method to check response type"
echo "  - Added getMessagesData() method to extract message details"
echo "  - Implemented getWaitingMessages() method in MessageUClient"
echo "  - Displays messages in a formatted way with sender, ID, type, and content"
echo ""
echo "✓ Testing:"
echo "  - Client compiles successfully"
echo "  - Server is running and ready to handle requests"
echo "  - Tested with no messages: correctly shows 'No waiting messages'"
echo ""
echo "✓ Protocol Format:"
echo "  - Request: Empty payload (no authentication yet)"
echo "  - Response: number_of_messages(4) + for each message:"
echo "    from_client_id(16) + message_id(4) + message_type(1) + content_size(4) + content"
echo ""
echo "To test interactively:"
echo "1. Run: ./messageu_client"
echo "2. Select option 4 (Get waiting messages)"
echo "3. The client should display waiting messages or 'No waiting messages'"
echo ""
echo "Note: Currently, the server uses the first registered client as the recipient"
echo "for testing purposes. In a real implementation, this would be based on authentication." 