#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PORT 8080

void serveFile(int socket, const char *path);
const char* getContentType(const char *path);
char* read_random_line(const char* filename);

int server_fd;

void handle_sigint(int sig) {
    printf("\nClosing socket and exiting...\n");
    close(server_fd); // Close the server socket
    exit(0); // Exit the program gracefully
}

int main() {
	// Signal handling setup
    signal(SIGINT, handle_sigint);
    
    int new_socket, opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    
    // Create a socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }
    
    // Set the server options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("In setsockopt");
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
        char *method = strtok(buffer, " ");
        char *uri = strtok(NULL, " ");
        
        if (strcmp(method, "POST") == 0 && strcmp(uri, "/joke") ==0) {
        	char *joke = read_random_line("jokes.txt");
         	if (joke) {
          		char response[2048];
                sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<span>%s</span>", joke);
                write(new_socket, response, strlen(response));
                free(joke);
          	} else {
		  		char *response = "HTTP/1.1 500 Internal Server Error\n\n";
				write(new_socket, response, strlen(response));
		  	}
        } else if (strcmp(method, "GET") == 0) {
            char path[1024] = "./public";
            if (strcmp(uri, "/") == 0) {
                strcat(path, "/index.html");
            } else {
                strcat(path, uri);
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

char* read_random_line(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    srand((unsigned)time(NULL));
    long random_pos = rand() % size;

    fseek(file, random_pos, SEEK_SET);

    // Adjust to the start of the next line
    char buffer[256];
    fgets(buffer, sizeof(buffer), file); // Potentially partial line

    char *line = malloc(256);
    if (!line) {
		perror("Failed to allocate memory");
		fclose(file);	
		return NULL;
	}
    
    if (!fgets(line, 256, file)) { // Read the next full line
        // If EOF is reached, start from the beginning
        fseek(file, 0, SEEK_SET);
        if (!fgets(line, 256, file)) {
			perror("Failed to read line");
			fclose(file);
			free(line);
			return NULL;
		}
    }
    fclose(file);
    return line;
}
