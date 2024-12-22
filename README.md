Socket-Based Chat Application with Logging and Parity Checking

Description

This project implements a multi-client chat application using socket programming in C. The system consists of two main components:

Client: A user-friendly application for sending and receiving messages.

Server: A robust server that handles multiple clients, supports commands, and broadcasts messages.

The application is suitable for real-time communication with features like user listing, private messaging, logging, and parity checking for message integrity.

Features

Server

Multi-Client Support: Handles up to 10 simultaneous connections.

Broadcast Messaging: Sends messages from one client to all others.

Private Messaging: Allows clients to send messages directly to specific users.

Command Handling:

/list: Displays a list of connected users.

/help: Provides a list of available commands.

/logout: Disconnects a user.

/msg <username> <message>: Sends a private message to a specific user.

Client

Socket Communication: Connects to the server for message exchange.

Real-Time Logging: Logs all incoming messages with timestamps and user-specific filenames.

Parity Check: Ensures message integrity using bitwise parity verification.

Command Integration: Supports server-specific commands (/list, /help, /logout).

Prerequisites

A C compiler (e.g., GCC)

Libraries: <stdio.h>, <stdlib.h>, <unistd.h>, <string.h>, <sys/types.h>, <sys/socket.h>, <netinet/in.h>, <pthread.h>, <sys/stat.h>, <time.h>, <netdb.h>

Setup and Usage

Compilation

Compile both the server and client programs:

gcc -o server server.c -pthread
gcc -o client client.c -pthread

Running the Server

Start the server with a specified port:

./server <port>

Replace <port> with the port number the server should listen on.

Example:

./server 8080

Running the Client

Connect a client to the server:

./client <hostname> <port> <username>

<hostname>: Server's IP address or hostname (e.g., 127.0.0.1).

<port>: The port number the server is listening on.

<username>: A unique identifier for the client.

Example:

./client 127.0.0.1 8080 alice

Command Reference

Server-Side Commands

/list: Show a list of connected users.

/help: Display the list of available commands.

/logout: Disconnect from the server.

/msg <username> <message>: Send a private message to a specific user.

Client-Side Features

Parity Check: Verifies the even/odd parity of each message before sending.

Logging: Logs all incoming messages in logs/<timestamp>_<username>.log.

Code Overview

Server

handle_client(): Handles client communication, processes commands, and broadcasts messages.

broadcast_message(): Sends a message to all connected clients.

send_private_message(): Sends a private message to a specific client.

Client

socket_reader(): Reads and logs messages from the server.

check_parity(): Checks the parity of outgoing messages.

Logging: Creates a timestamped log file for each client.

Example Workflow

Start the Server:

./server 8080

Connect Clients:

./client 127.0.0.1 8080 user1
./client 127.0.0.1 8080 user2

Broadcast Message:
User1 sends a message, and User2 receives it.

Private Message:
User1 sends /msg user2 Hello!.

List Users:
Type /list to see a list of connected users.

Logout:
Type /logout to disconnect.

Notes

Ensure the server is started before clients connect.

Logs are stored in the logs/ directory created dynamically by the client.

