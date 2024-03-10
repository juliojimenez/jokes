#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

void serveFile(int socket, const char *path);
const char* getContentType(const char *path);

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    
    // Create a socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    
    // Set the server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    
    // Listen for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    
    // Accept a new connection
    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }
        
        // Read the request from the client
        read(new_socket, buffer, 2048);
        
        // Simple request parsing for GET request
        char *token = strtok(buffer, " ");
        if (strcmp(token, "GET") == 0) {
            token = strtok(NULL, " ");
            char path[1024] = "public";
            if (strlen(token) > 1) {    // request is not root
                strcat(path, token);
            } else {
                strcat(path, "/index.html");
            }
            serveFile(new_socket, path);
        }
        close(new_socket);
    }
    return 0;
}
void serveFile(int socket, const char *path) {
    char buffer[1024];
    int filefd = open(path, O_RDONLY);
    
    if (filefd == -1) {
        // File not found, send 404
        char *header = "HTTP/1.1 404 Not Found...ya bum\n\n";
        write(socket, header, strlen(header));
    } else {
        // File found, send 200
        int bytes;
        const char* contentType = getContentType(path);
        char header[1024];
        sprintf(header, "HTTP/1.1 200 OK\nContent-Type: %s\n\n", contentType);
        write(socket, header, strlen(header));
        
        while ((bytes = read(filefd, buffer, sizeof(buffer))) > 0) {
            write(socket, buffer, bytes);
        }
        close(filefd);
    }
}

const char* getContentType(const char *path) {
    // Determine content type based on file extension
    const char *dot = strrchr(path, '.');
    if (dot) {
        if (strcmp(dot, ".html") == 0) return "text/html";
        if (strcmp(dot, ".css") == 0) return "text/css";
        if (strcmp(dot, ".js") == 0) return "text/javascript";
        if (strcmp(dot, ".jpg") == 0) return "image/jpeg";
        if (strcmp(dot, ".png") == 0) return "image/png";
        if (strcmp(dot, ".gif") == 0) return "image/gif";
    }
    return "text/plain";
}
