"""
Database handler for MessageU server.
Provides SQLite persistence for users and messages (bonus implementation).

Features:
- Thread-safe database connections
- Client registration and management
- Message storage and retrieval
- Automatic table creation
- Retry logic for database locks
"""

import sqlite3
import os
import threading
from typing import Optional, List, Dict, Any

class DatabaseHandler:
    def __init__(self, db_path: str = "defensive.db"):
        self.db_path = db_path
        self._local = threading.local()  # Thread-local storage
        self.initialize_database()
    
    def _get_connection(self):
        """Get a thread-local database connection."""
        if not hasattr(self._local, 'connection'):
            self._local.connection = sqlite3.connect(self.db_path)
        return self._local.connection
    
    def initialize_database(self):
        """Initialize the database and create tables if they don't exist."""
        try:
            conn = sqlite3.connect(self.db_path)
            self.create_tables(conn)
            conn.close()
            print(f"Database initialized: {self.db_path}")
        except sqlite3.Error as e:
            print(f"Database initialization error: {e}")
            raise
    
    def create_tables(self, conn=None):
        """Create the required tables if they don't exist."""
        if conn is None:
            conn = self._get_connection()
            
        cursor = conn.cursor()
        
        # Create clients table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS clients (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                client_id TEXT UNIQUE NOT NULL,
                name TEXT NOT NULL,
                public_key TEXT NOT NULL,
                last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Create messages table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                from_client_id TEXT NOT NULL,
                to_client_id TEXT NOT NULL,
                message_type INTEGER NOT NULL,
                content TEXT NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (from_client_id) REFERENCES clients (client_id),
                FOREIGN KEY (to_client_id) REFERENCES clients (client_id)
            )
        ''')
        
        conn.commit()
        print("Database tables created/verified")
    
    def register_client(self, client_id: str, name: str, public_key: str) -> bool:
        """Register a new client in the database."""
        try:
            conn = self._get_connection()
            cursor = conn.cursor()
            cursor.execute('''
                INSERT INTO clients (client_id, name, public_key)
                VALUES (?, ?, ?)
            ''', (client_id, name, public_key))
            conn.commit()
            print(f"Client registered: {name} (ID: {client_id})")
            return True
        except sqlite3.IntegrityError:
            print(f"Client already exists: {client_id}")
            return False
        except sqlite3.Error as e:
            print(f"Database error during registration: {e}")
            return False
    
    def get_client(self, client_id: str) -> Optional[Dict[str, Any]]:
        """Get client information by ID."""
        try:
            conn = self._get_connection()
            cursor = conn.cursor()
            cursor.execute('''
                SELECT client_id, name, public_key, last_seen
                FROM clients WHERE client_id = ?
            ''', (client_id,))
            row = cursor.fetchone()
            if row:
                return {
                    'client_id': row[0],
                    'name': row[1],
                    'public_key': row[2],
                    'last_seen': row[3]
                }
            return None
        except sqlite3.Error as e:
            print(f"Database error getting client: {e}")
            return None
    
    def get_all_clients(self) -> List[Dict[str, Any]]:
        """Get all registered clients."""
        try:
            conn = self._get_connection()
            cursor = conn.cursor()
            cursor.execute('''
                SELECT client_id, name, public_key, last_seen
                FROM clients ORDER BY name
            ''')
            rows = cursor.fetchall()
            return [
                {
                    'client_id': row[0],
                    'name': row[1],
                    'public_key': row[2],
                    'last_seen': row[3]
                }
                for row in rows
            ]
        except sqlite3.Error as e:
            print(f"Database error getting all clients: {e}")
            return []
    
    def get_client_by_identifier(self, identifier: str) -> Optional[Dict[str, Any]]:
        """Get a client by ID or name."""
        try:
            conn = self._get_connection()
            cursor = conn.cursor()
            cursor.execute('''
                SELECT client_id, name, public_key, last_seen
                FROM clients WHERE client_id = ? OR name = ?
            ''', (identifier, identifier))
            row = cursor.fetchone()
            if row:
                return {
                    'client_id': row[0],
                    'name': row[1],
                    'public_key': row[2],
                    'last_seen': row[3]
                }
            return None
        except sqlite3.Error as e:
            print(f"Database error getting client by identifier: {e}")
            return None
    
    def get_waiting_messages(self, to_client_id: str) -> List[Dict[str, Any]]:
        """Get all waiting messages for a client."""
        try:
            conn = self._get_connection()
            cursor = conn.cursor()
            cursor.execute('''
                SELECT m.id, m.from_client_id, m.to_client_id, m.message_type, m.content, m.created_at,
                       c.name as sender_name
                FROM messages m
                LEFT JOIN clients c ON m.from_client_id = c.client_id
                WHERE m.to_client_id = ?
                ORDER BY m.created_at ASC
            ''', (to_client_id,))
            rows = cursor.fetchall()
            return [
                {
                    'id': row[0],
                    'from_client_id': row[1],
                    'to_client_id': row[2],
                    'message_type': row[3],
                    'content': row[4],
                    'created_at': row[5],
                    'sender_name': row[6] if row[6] else 'Unknown'
                }
                for row in rows
            ]
        except sqlite3.Error as e:
            print(f"Database error getting waiting messages: {e}")
            return []
    
    def delete_messages(self, message_ids: List[int]) -> bool:
        """Delete messages by their IDs (mark as delivered)."""
        if not message_ids:
            return True
            
        max_retries = 3
        for attempt in range(max_retries):
            try:
                conn = self._get_connection()
                cursor = conn.cursor()
                placeholders = ','.join(['?' for _ in message_ids])
                cursor.execute(f'''
                    DELETE FROM messages WHERE id IN ({placeholders})
                ''', message_ids)
                conn.commit()
                print(f"Deleted {len(message_ids)} messages")
                return True
            except sqlite3.OperationalError as e:
                if "database is locked" in str(e) and attempt < max_retries - 1:
                    print(f"Database locked, retrying... (attempt {attempt + 1}/{max_retries})")
                    import time
                    time.sleep(0.1)  # Wait 100ms before retry
                    continue
                else:
                    print(f"Database error deleting messages: {e}")
                    return False
            except sqlite3.Error as e:
                print(f"Database error deleting messages: {e}")
                return False
        
        return False
    
    def store_message(self, from_client_id: str, to_client_id: str, message_type: int, content: str) -> bool:
        """Store a new message in the database."""
        try:
            conn = self._get_connection()
            cursor = conn.cursor()
            cursor.execute('''
                INSERT INTO messages (from_client_id, to_client_id, message_type, content)
                VALUES (?, ?, ?, ?)
            ''', (from_client_id, to_client_id, message_type, content))
            conn.commit()
            print(f"Message stored: from {from_client_id} to {to_client_id}")
            return True
        except sqlite3.Error as e:
            print(f"Database error storing message: {e}")
            return False
    
    def update_last_seen(self, client_id: str):
        """Update the last_seen timestamp for a client."""
        try:
            conn = self._get_connection()
            cursor = conn.cursor()
            cursor.execute('''
                UPDATE clients SET last_seen = CURRENT_TIMESTAMP
                WHERE client_id = ?
            ''', (client_id,))
            conn.commit()
        except sqlite3.Error as e:
            print(f"Database error updating last_seen: {e}")
    
    def close(self):
        """Close the database connection."""
        if hasattr(self._local, 'connection'):
            self._local.connection.close()
            print("Database connection closed") 