# C Web Server

This is a simple, multi-threaded web server written in C. It's capable of serving static files and handling multiple client connections concurrently.

## Core Components

- `server.c`: The main entry point of the application. It sets up the socket, listens for incoming connections, and spawns a new thread to handle each client.
- `server_utils.c`: Contains the core logic for handling HTTP requests, parsing headers, serving files, and managing responses.
- `server_utils.h`: The header file for `server_utils.c`, defining the data structures and function prototypes used throughout the application.

## Features

- **Multi-threaded:** Each client connection is handled in a separate thread, allowing the server to manage multiple requests simultaneously.
- **Static File Serving:** The server can serve various file types, including HTML, CSS, JavaScript, and images (JPEG, PNG, GIF, WebP).
- **HTTP/1.1 Compliant:** The server follows the basic structure of HTTP/1.1 for requests and responses.
- **Error Handling:** The server provides appropriate HTTP error codes for common issues like "404 Not Found" and "500 Internal Server Error."

## How to Compile and Run

1.  **Compile the code:**

    ```bash
    gcc -o server.out server.c server_utils.c -lpthread
    ```

2.  **Run the server:**

    ```bash
    ./server.out
    ```

By default, the server listens on port `6969`. You can access it by navigating to `http://localhost:6969` in your web browser.

## Key Functions

### `server.c`

- **`main()`**: Initializes the server, binds it to a port, and enters an infinite loop to accept and handle incoming connections.

### `server_utils.c`

- **`handle_client(void *arg)`**: The main function for handling a client connection. It reads the request, parses it, and sends the appropriate response.
- **`setup_socket()`**: Configures and binds the server's socket to listen for connections.
- **`parse_request(char *request)`**: Parses the client's HTTP request to extract the method, path, and protocol.
- **`parse_file(char *file_name)`**: Opens the requested file and retrieves its metadata, such as content type and length.
- **`send_response(...)`**: Constructs and sends the HTTP response header to the client.
- **`render_html(...)`**: Streams the content of the requested file to the client.
- **`send_error(...)`**: Sends an HTTP error response to the client.

## Data Structures

- **`FileData`**: Holds information about a file, including its descriptor, content length, and MIME type.
- **`HeaderData`**: Stores the parsed components of an HTTP request header.
- **`ThreadArgs`**: Contains the arguments passed to each client-handling thread, including the socket descriptor and client's IP address.
