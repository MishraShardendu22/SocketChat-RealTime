# Makefile for TCP Chat TurnTaking
CC = gcc
CFLAGS = -Wall -Wextra -pthread
SERVER = Server
CLIENT = Client

.PHONY: all clean run server client

all: $(SERVER) $(CLIENT)

$(SERVER): Server.c
	$(CC) $(CFLAGS) -o $(SERVER) Server.c

$(CLIENT): Client.c
	$(CC) $(CFLAGS) -o $(CLIENT) Client.c

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
