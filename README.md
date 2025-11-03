# SocketChat-RealTime

## Creating a Connection Using Sockets

---

## **Server Side**

### **SBLA → Socket (1) → Bind (2) → Listen (3) → Accept (4)**

---

### **(1) Socket**

```c
int sockfd = socket(int domain, int type, int protocol);
```

* **Socket File Descriptor:** Integer uniquely identifying a socket in the process.
* **domain:** `AF_INET` → IPv4
* **type:**

  * `SOCK_STREAM` → TCP
  * `SOCK_DGRAM` → UDP
* **protocol:** `0` → Default protocol for given domain and type.

---

#### **(2) Bind**

```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

Associates the socket with a specific IP address and port.

**`struct sockaddr` (generic structure):**

```c
struct sockaddr {
    sa_family_t sa_family;
    char sa_data[14];
};
```

**Example:**

```c
int result = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
if (result == -1) {
    perror("bind failed");
}
```

---

#### **(3) Listen**

```c
int listen(int sockfd, int backlog);
```

Marks the socket as a passive socket that listens for incoming connections.

* **sockfd:** The socket file descriptor.
* **backlog:** The maximum number of pending connections in the queue (e.g., `4`).

---

#### **(4) Accept**

```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

Accepts an incoming connection request.

* **sockfd:** Listening socket descriptor.
* **addr:** Pointer to a `sockaddr` structure where the client’s address will be stored.
* **addrlen:** Pointer to a variable that initially holds the size of the address structure and is modified to indicate the actual size.

---

## **Client Side**

## **SC → Socket (1) → Connect (2)**

---

### **(1) Socket**

```c
int sockfd = socket(int domain, int type, int protocol);
```

Creates an endpoint for communication, same as on the server side.

---

### **(2) Connect**

```c
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

Initiates a connection to a server.

* **sockfd:** Client socket descriptor.
* **addr:** Pointer to the server’s address structure.
* **addrlen:** Size of the address structure.

---

### **Both Sides (Communication)**

#### **Read**

```c
int read(int sockfd, void *buf, size_t count);
```

Reads data from the socket into the buffer.

#### **Write**

```c
int write(int sockfd, const void *buf, size_t count);
```

Sends data from the buffer to the socket.

## What is 127.0.0.1?

127.0.0.1 is the loopback IP address. It refers to your own machine (localhost).
It’s used for testing or communication within the same device, not across a network.# SocketChat-RealTime
