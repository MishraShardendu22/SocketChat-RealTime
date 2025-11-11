# Makefile for TCP One Way Communication
CC = gcc
CFLAGS = -Wall -Wextra -pthread
SERVER = TCP_SERVER_ONE_WAY
CLIENT = TCP_CLIENT_ONE_WAY

.PHONY: all clean run server client

all: $(SERVER) $(CLIENT)

$(SERVER): TCP_SERVER_ONE_WAY.c
	$(CC) $(CFLAGS) -o $(SERVER) TCP_SERVER_ONE_WAY.c

$(CLIENT): TCP_CLIENT_ONE_WAY.c
	$(CC) $(CFLAGS) -o $(CLIENT) TCP_CLIENT_ONE_WAY.c

run: all
	@echo "Starting server and client in separate terminals..."
	gnome-terminal -- bash -c "./$(SERVER); exec bash" &
	sleep 1
	gnome-terminal -- bash -c "./$(CLIENT); exec bash" &

server: $(SERVER)
	./$(SERVER)

client: $(CLIENT)
	./$(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)
