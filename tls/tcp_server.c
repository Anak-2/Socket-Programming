#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9999
#define BUFFER_SIZE 1024
#define SERVER_IP "192.168.1.10" // 가상 IP 주소

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int bytes_read;

    // Create a socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Bind the socket to the specified IP and port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running and waiting for connections on %s...\n", SERVER_IP);

    // Accept an incoming connection
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // Communicate with the client
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE); // Clear the buffer
        bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0)
        {
            break; // Client disconnected or error occurred
        }

        printf("Message from client: %s\n", buffer);

        // Echo the message back to the client
        write(client_fd, buffer, strlen(buffer));
    }

    // Close the client and server sockets
    close(client_fd);
    close(server_fd);

    printf("Connection closed.\n");
    return 0;
}
