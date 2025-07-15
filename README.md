# C Web Server

This is a simple, multi-threaded web server written in C. It's capable of serving static files and handling multiple simultaneous connections.

## Features

- **Multi-threaded:** Handles multiple clients concurrently using POSIX threads.
- **Static File Serving:** Serves static files (HTML, CSS, JavaScript, images, etc.).
- **Configuration:** Uses a `config.json` file for easy configuration of port, backlog, and root directory.
- **MIME Type Support:**  Correctly identifies and serves common file types with the appropriate MIME types.
- **Error Handling:**  Sends appropriate HTTP error codes (400, 404, 500) to the client.

## Dependencies

- **cJSON:** Used for parsing the `config.json` file. The source code for cJSON is included in the `cjson` directory.

## How to Compile and Run

1.  **Compile:**
    ```bash
    gcc -o server.out server.c server_utils.c cjson/cJSON.c -lpthread
    ```

2.  **Configure:**
    Create a `config.json` file in the same directory with the following format:
    ```json
    {
      "port": "8080",
      "backlog": 10,
      "root": "./public"
    }
    ```
    - `port`: The port number for the server to listen on.
    - `backlog`: The maximum number of pending connections.
    - `root`: The root directory from which to serve files.

3.  **Run:**
    ```bash
    ./server.out
    ```

The server will then be running and listening for connections on the specified port.

## Code Structure

- **`server.c`:** The main entry point of the application. It sets up the socket, listens for incoming connections, and creates a new thread to handle each client.
- **`server_utils.h`:** Header file for the server utility functions.
- **`server_utils.c`:** Contains helper functions for the server, including:
    - Socket setup and configuration.
    - Request parsing.
    - Response generation.
    - File handling and MIME type detection.
- **`cjson/`:** Contains the cJSON library for parsing JSON.
- **`public/`:**  The default root directory for serving static files.
- **`config.json`:** Configuration file for the server.