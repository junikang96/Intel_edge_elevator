#include "../incs/define.h"

int socket_fd = 0;

void set_socket_fd(int fd)
{
    socket_fd = fd;
    printf("fd is %d\n", fd);
}

int connect_to_server() {
    struct sockaddr_in server_addr;
    int server_socket = 0;
    
    if (is_connected && server_socket >= 0) {
        return server_socket;
    }
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed.");
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Wrong server IP");
        close(server_socket);
        server_socket = -1;
        return -1;
    }
    
    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Server connection failed.");
        close(server_socket);
        server_socket = -1;
        return -1;
    }
    
    printf("Server connected (IP: %s, Port: %d)\n", SERVER_IP, SERVER_PORT);
    
    if (send(server_socket, "raspberry", 9, 0) < 0) {
        perror("Sending Client ID failed.");
        close(server_socket);
        server_socket = -1;
        return -1;
    }
    
    printf("Sending Client Id completed.\n");
    is_connected = 1;
    return server_socket;
}

void disconnect_from_server(int server_socket) {
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
        is_connected = 0;
        printf("Server connection terminated.\n");
    }
}

int send_text_to_server(const char *text) {
    char buffer[1024];
    // int socket_fd;
    
    // socket_fd = connect_to_server();
    // if (socket_fd < 0) {
    //     fprintf(stderr, "Server connection failed, sending text failed.\n");
    //     return -1;
    // }
    
    printf("this is serverFD:%d\n", socket_fd);
    snprintf(buffer, sizeof(buffer), "%s", "BUTTON_PRESSED");
    
    if (send(socket_fd, buffer, strlen(buffer), 0) < 0) {
        perror("Sending Text failed");
        return -1;
    }
    
    printf("Send: %s\n", text);
    return 0;
}