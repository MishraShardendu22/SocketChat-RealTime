# Quick Start Guide

## Quick Test - Run Any Program

```bash
cd MoreLearning/<folder_name>
make run
```

This opens server and client in separate terminals automatically!

## All Programs at a Glance

| Program | Type | Port | Description |
|---------|------|------|-------------|
| TCP_CHAT/Arbitary | TCP | 5000 | Both can chat simultaneously (threads) |
| TCP_CHAT/Brodcast | TCP | 5000 | Multi-client broadcast server |
| TCP_CHAT/TurnTaking | TCP | 5000 | Server first, then client |
| TCP_FILE | TCP | 5000 | Send file from server to client |
| TCP_ONE_WAY | TCP | 6000 | Client sends, server receives only |
| UDP_CHAT/Brodcast | UDP | 8080 | Multi-client UDP broadcast |
| UDP_CHAT/TurnTaking | UDP | 8080 | Client first, then server |
| UDP_FILE | UDP | 5000 | File transfer with ACK packets |
| UDP_ONE_WAY | UDP | 5000 | Client sends once, server receives |

## Manual Testing

### Option 1: Use Makefile (Recommended)
```bash
make run      # Opens both in separate terminals
```

### Option 2: Manual Terminal Method
Terminal 1:
```bash
make server   # or ./Server or ./TCP_FILE_SERVER
```

Terminal 2:
```bash
make client   # or ./Client or ./TCP_FILE_CLIENT
```

## Examples

### Test Chat
```bash
cd TCP_CHAT/Arbitary
make run
# Type messages in both windows simultaneously!
```

### Test File Transfer
```bash
cd TCP_FILE
make run
# Watch file_to_send.txt transfer to received_file.txt
```

### Test Multi-Client Broadcast
```bash
cd TCP_CHAT/Brodcast
make server   # Terminal 1
make client   # Terminal 2
make client   # Terminal 3 (yes, run again!)
# Messages from one client appear on all others
```

## Tips

- All TCP programs use port 5000 or 6000
- All UDP programs use port 8080 or 5000
- Press Ctrl+C to exit most programs
- Type "exit" or "quit" in one-way programs
- File transfer programs create files automatically

## Troubleshooting

**Port already in use?**
```bash
# Kill previous instance
pkill -f Server
pkill -f CLIENT
```

**Can't find executable?**
```bash
make clean
make all
```

**Make not working?**
Check you're in the right directory with `ls` - should see Makefile

## Compilation Only

Just want to compile without running?
```bash
make all      # Compile both
make clean    # Remove binaries
```
