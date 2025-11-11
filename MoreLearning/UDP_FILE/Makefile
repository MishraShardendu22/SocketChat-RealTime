# Makefile for UDP File Transfer
CC = gcc
CFLAGS = -Wall -Wextra -pthread
SERVER = UDP_FILE_SERVER
CLIENT = UDP_FILE_CLIENT

.PHONY: all clean run server client

all: $(SERVER) $(CLIENT)

$(SERVER): UDP_FILE_SERVER.c
	$(CC) $(CFLAGS) -o $(SERVER) UDP_FILE_SERVER.c

$(CLIENT): UDP_FILE_CLIENT.c
	$(CC) $(CFLAGS) -o $(CLIENT) UDP_FILE_CLIENT.c

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
