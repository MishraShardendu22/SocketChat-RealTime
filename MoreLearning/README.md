# Socket Programming Examples

All programs are minimal implementations without extensive error checking.
**Status: ✅ All 9 programs compile and work correctly**

## TCP Chat Programs

### TCP_CHAT/Arbitary
- **Simultaneous chat** using pthreads
- Both server and client can send/receive at any time
- No turn-taking required
- Uses 2 threads per program (send + receive)
- Port: 5000

### TCP_CHAT/TurnTaking
- **Strict alternating chat**
- Server talks first, client responds
- Simple single-threaded design
- Port: 5000

### TCP_CHAT/Brodcast
- **Multi-client broadcast server**
- Uses select() for multiplexing
- Messages from one client broadcast to all others
- Supports up to 10 simultaneous clients
- Port: 5000

## TCP File Transfer & One-Way

### TCP_FILE
- **File transfer over TCP**
- Server sends file in chunks to client
- Client receives and saves to file
- Port: 5000
- Files: `file_to_send.txt` → `received_file.txt`

### TCP_ONE_WAY
- **One-way messaging (client to server)**
- Client sends messages, server receives and displays
- No response from server to client
- Type "exit" or "quit" to disconnect
- Port: 6000

## UDP Chat Programs

### UDP_CHAT/Brodcast
- **Multi-client UDP broadcast**
- Server maintains client list (up to 10 clients)
- Messages broadcast to all except sender
- Uses pthreads for simultaneous I/O on client
- Port: 8080

### UDP_CHAT/TurnTaking
- **Simple 1-on-1 UDP chat**
- Client talks first, server responds
- No connection state (connectionless UDP)
- Simple turn-based messaging
- Port: 8080

## UDP File Transfer & One-Way

### UDP_FILE
- **File transfer over UDP with acknowledgments**
- Packet-based transfer with ACK after each packet
- Includes timeout handling and EOF marker
- Server sends `file.txt` → Client saves as `received.txt`
- Port: 5000

### UDP_ONE_WAY
- **One-way messaging (client to server)**
- Client sends single message, server receives
- No acknowledgment or response
- Minimal connectionless communication
- Port: 5000

## Usage

Each folder has a Makefile with these commands:
- `make all` - Compile both server and client
- `make run` - Open server and client in separate terminals (uses gnome-terminal)
- `make server` - Run server only
- `make client` - Run client only
- `make clean` - Remove compiled binaries

### Examples:

**Run simultaneous chat:**
```bash
cd TCP_CHAT/Arbitary
make run
```

**Transfer a file:**
```bash
cd TCP_FILE
make run
```

**Test UDP broadcast:**
```bash
cd UDP_CHAT/Brodcast
make run
# You can open multiple clients to test broadcasting
```

## Testing

All programs have been tested and compile without errors:
```bash
✅ TCP_CHAT/Arbitary
✅ TCP_CHAT/Brodcast
✅ TCP_CHAT/TurnTaking
✅ TCP_FILE
✅ TCP_ONE_WAY
✅ UDP_CHAT/Brodcast
✅ UDP_CHAT/TurnTaking
✅ UDP_FILE
✅ UDP_ONE_WAY
```

## Notes

- All programs use `127.0.0.1` (localhost) for testing
- Minimal error checking for educational clarity
- TCP programs use `SOCK_STREAM`, UDP programs use `SOCK_DGRAM`
- File transfer programs create files in their respective directories
