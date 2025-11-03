
CC = gcc
CFLAGS = -Wall -g

SERVER_SRC = Socket_Server.c
CLIENT_SRC = Socket_Client.c

SERVER_BIN = server
CLIENT_BIN = client

.PHONY: all server client run stop clean

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $^

run_server: $(SERVER_BIN)
	@echo "Starting server on port 5000..."
	@{ ./$(SERVER_BIN) 5000 > server.log 2>&1 & echo $$! > server.pid; }
	@sleep 1
	@echo "Server started with PID: `cat server.pid`"
	@echo "Server is listening on port 5000"

run_client: $(CLIENT_BIN)
	@echo "Connecting client to server on port 5000..."
	@./$(CLIENT_BIN) 127.0.0.1 5000

run: run_server
	@echo ""
	@echo "To connect client, run in another terminal:"
	@echo "  make run_client"
	@echo "or:"
	@echo "  ./client 127.0.0.1 5000"

stop:
	@if [ -f server.pid ]; then kill `cat server.pid` 2>/dev/null || true; rm -f server.pid; fi
	@echo "Server stopped."

clean:
	@rm -f $(SERVER_BIN) $(CLIENT_BIN) *.o *.log *.pid

