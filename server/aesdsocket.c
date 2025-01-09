#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>


#define PORT "9000"
#define BUFF_SIZE 20480
#define DATA_FILE "/var/tmp/aesdsocketdata"

int serverfd, clientfd;

void handle_client(int clientfd) {

    // Open DATA_FILE
    int filefd = open(DATA_FILE, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (filefd == -1) {
        syslog(LOG_ERR, "Opening file %s failed", DATA_FILE);
        perror("Opening file failed");
        close(clientfd);  // Close the client socket before exiting
        exit(1);
    }

    // Receive data from client until data packet is complete (EOF)
    char buffer[BUFF_SIZE];
    int data_bytes;
    while ((data_bytes = recv(clientfd, buffer, BUFF_SIZE - 1, 0)) > 0) {

        // Search newline on buffer
        for (int i = 0; i < data_bytes; i++) {
            if (buffer[i] == '\n') {
                // Append the data packet to file including newline
                if (write(filefd, buffer, i + 1) == -1) {
                    syslog(LOG_ERR, "Writing data to %s failed", DATA_FILE);
                    perror("Writing data to file failed");
                    close(filefd);
                    close(clientfd);
                    exit(1);
                } else {
                    syslog(LOG_INFO, "written to file");
                    printf("written to file: %s\n", buffer);
                }
            }
        }

        // Reposition file offset to start of file
        if (lseek(filefd, 0, SEEK_SET) == -1) {
            syslog(LOG_ERR, "Rewinding file failed");
            perror("Rewinding file failed");
            close(filefd);
            close(clientfd);
            exit(1);
        }

        // Read file content
        int bytes_read = read(filefd, buffer, BUFF_SIZE);
        if (bytes_read == -1) {
            syslog(LOG_ERR, "Reading from file failed");
            perror("Reading from file failed");
            close(filefd);
            close(clientfd);
            exit(1);
        }
        syslog(LOG_INFO, "Read from file");
        printf("Read from file: %s\n", buffer);

        // Send the file content to the client
        if (send(clientfd, buffer, bytes_read, 0) == -1) {
            syslog(LOG_ERR, "Sending file content to client failed");
            perror("Sending file content to client failed");
            close(filefd);
            close(clientfd);
            exit(1);
        }
        syslog(LOG_INFO, "Sent to client");
        printf("num of bytes: %d, Sent to client: %s\n", bytes_read, buffer);

    }
    if (data_bytes == -1) {
        syslog(LOG_ERR, "Receiving message failed");
        perror("Receiving message failed");
        close(filefd);
        close(clientfd);
        exit(1);
    }

    syslog(LOG_INFO, "Stop recieving from client");
    printf("Stop recieving from client\n");

    close(filefd);
    close(clientfd);    
}

void handle_signal (int sig) {

    printf(" Caught signal %d, exiting...\n", sig);

    close(clientfd);
    close(serverfd);

    // Delete DATA_FILE
    if (unlink(DATA_FILE) == 0) {
        printf("Delete file: %s\n", DATA_FILE);
    } else {
        perror("Failed to delete file\n");
    }

    exit(0);
}

void support_deamon () {

    pid_t PID;
    PID = fork();

    if (PID < 0){
        syslog(LOG_ERR, "fork faild");
        perror("fork failed");
        exit(1);
    }

    if (PID > 0) {
        // Parent exit
        exit(0);
    }

    // Child becomes session leader 
    if (setsid() < 0) {
        syslog(LOG_ERR, "Failed to create new session");
        perror("Failed to create new session\n");
        exit(1);
    }

    // Second fork
    PID = fork();

    if (PID < 0) {
        syslog(LOG_ERR, "Second fork faild");
        perror("Second fork failed\n");
        exit(1);
    }

    if (PID > 0) {
        // First child exits
        exit(0);
    }

    // Daemon
    // Change working directory to root
    if (chdir("/") < 0) {
        perror("Failed to change directory to /\n");
        exit(1);
    }
    
    syslog(LOG_INFO, "Daemon process created successfully");

}

int main(int argc, char *argv[]) {

    printf("********************************aesdsocket*******************************\n");

    bool daemon = false;

    // Check for -d flag
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        daemon = true;
    }

    // Open connection to syslog
    openlog(NULL, 0, LOG_USER);

    // Handle SIGINT and SIGTERM
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) != 0) {
        perror("Failed to set SIGINT handler\n");
        return 1;
    }
    if (sigaction(SIGTERM, &action, NULL) != 0) {
        perror("Failed to set SIGTERM handler\n");
        return 1;
    }

    // Open socket
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverfd == -1) {
        syslog(LOG_ERR, "Socket creation failed");
        perror("Socket creation failed\n");
        return 1;
    }
    syslog(LOG_INFO, "Socket created successfully");
    printf("Socket created successfully\n");

    // Setting socket to reuse address
    int opt = 1;
    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        syslog(LOG_ERR, "setsockopt failed");
        perror("setsockopt failed\n");
        close(serverfd);
        return 1;
    }

    // Server address information
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *servinfo;

    if (getaddrinfo(NULL, PORT, &hints, &servinfo) != 0) {
        syslog(LOG_ERR, "Function getaddrinfo failed");
        perror("Function getaddrinfo failed\n");
        close(serverfd);
        return 1;
    }

    // Bind server socket to address
    if (bind(serverfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        syslog(LOG_ERR, "Binding server socket failed");
        perror("Binding server socket failed\n");
        close(serverfd);
        return 1;
    }

    // Server IP to string
    char server_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(servinfo->ai_addr), server_ip, INET_ADDRSTRLEN);

    syslog(LOG_INFO, "Socket bound to port %s", PORT);
    printf("Socket bound to port %s and server %s\n", PORT, server_ip);

    freeaddrinfo(servinfo);

    if (daemon) {
        support_deamon();
    }

    // Listening for connections
    if (listen(serverfd, 1) == -1) {
        syslog(LOG_ERR, "Listen failed");
        perror("Listen failed\n");
        close(serverfd);
        return 1;
    }

    syslog(LOG_INFO, "Listening on port %s...", PORT);
    printf("Listening on port %s...\n", PORT);

    // Client address information
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    while (1) {
        // Accepting connection
        clientfd = accept(serverfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (clientfd == -1) {
            syslog(LOG_ERR, "Accepting connection failed");
            perror("Accepting connection failed\n");
            close(serverfd);
            return 1;
        }

         // Client IP to string
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, (struct sockaddr *)&client_addr, client_ip, INET_ADDRSTRLEN);
        
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);
        printf("Accepted connection from %s\n", client_ip);

        handle_client(clientfd);
    
    }

    closelog();
    return 0;
}