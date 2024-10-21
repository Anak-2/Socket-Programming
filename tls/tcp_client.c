#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9999
#define SERVER_IP "192.168.1.10" // 서버의 IP 주소
#define CLIENT_IP "192.168.1.20" // 가상 클라이언트 IP 주소
#define BUFFER_SIZE 1024

int main()
{
    int client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    // Create a socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        perror("Unable to create socket");
        return 1;
    }

    // Set up the client address structure (bind client to CLIENT_IP)
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    inet_pton(AF_INET, CLIENT_IP, &client_addr.sin_addr);
    client_addr.sin_port = 0; // Let the system pick an available port

    if (bind(client_fd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("Client bind failed");
        close(client_fd);
        return 1;
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect failed");
        close(client_fd);
        return 1;
    }

    printf("Connected to the server.\n");

    // Communicate with the server
    while (1)
    {
        printf("Enter message to send to the server: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline character

        if (strcmp(buffer, "exit") == 0)
        {
            break; // Exit the loop if the user types "exit"
        }

        // Send the message to the server
        write(client_fd, buffer, strlen(buffer));

        // Receive the echoed message from the server
        memset(buffer, 0, BUFFER_SIZE); // Clear the buffer
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0)
        {
            break; // Server disconnected or error occurred
        }

        printf("Message from server: %s\n", buffer);
    }

    // Close the client socket
    close(client_fd);
    printf("Connection closed.\n");
    return 0;
}
