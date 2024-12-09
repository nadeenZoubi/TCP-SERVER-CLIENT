# Client-Server Proxy Application

This repository contains the implementation of a **Client-Server Proxy Application** developed in the C programming language. The application demonstrates a basic proxy mechanism where a client communicates with a server via a proxy server. The project is designed to showcase fundamental concepts of socket programming and network communication.

---

## Features

- **Client**: 
  - Sends requests to the proxy server.
  - Receives responses from the server via the proxy.

- **Proxy Server**: 
  - Intercepts requests from the client.
  - Forwards requests to the target server and relays the server's response back to the client.

- **Network Communication**: 
  - Implements socket programming for client-proxy-server communication.

- **Error Handling**: 
  - Includes basic error handling for common issues such as connection failures and invalid responses.

---

## Installation and Setup

### Prerequisites
- **GCC** or any compatible C compiler.
- A **Linux/Unix-based system**.
  - For Windows users, consider using **WSL (Windows Subsystem for Linux)** or a compatible environment.
- Basic knowledge of **socket programming**.

---

### Compilation

To compile the application components, use the following commands:

```bash
gcc -o client CLIENT.c
gcc -o proxy PROXY.c

