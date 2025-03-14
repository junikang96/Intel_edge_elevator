#include "../incs/STT_client.h"

int connect_to_server() {
    struct sockaddr_in server_addr;
    
    if (is_connected && server_socket >= 0) {
        return server_socket;
    }
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("서버 소켓 생성 오류");
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("잘못된 서버 주소");
        close(server_socket);
        server_socket = -1;
        return -1;
    }
    
    if (connect(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("서버 연결 실패");
        close(server_socket);
        server_socket = -1;
        return -1;
    }
    
    printf("서버에 연결되었습니다 (IP: %s, 포트: %d)\n", SERVER_IP, SERVER_PORT);
    
    if (send(server_socket, "raspberry", 9, 0) < 0) {
        perror("클라이언트 ID 전송 실패");
        close(server_socket);
        server_socket = -1;
        return -1;
    }
    
    printf("클라이언트 ID 전송 완료\n");
    is_connected = 1;
    return server_socket;
}

void disconnect_from_server() {
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
        is_connected = 0;
        printf("서버 연결이 종료되었습니다\n");
    }
}

int send_text_to_server(const char *text) {
    char buffer[1024];
    int socket_fd;
    
    socket_fd = connect_to_server();
    if (socket_fd < 0) {
        fprintf(stderr, "서버 연결 실패, 텍스트 전송 불가\n");
        return -1;
    }
    
    snprintf(buffer, sizeof(buffer), "%s", text);
    
    if (send(socket_fd, buffer, strlen(buffer), 0) < 0) {
        perror("텍스트 전송 실패");
        return -1;
    }
    
    printf("텍스트를 서버로 전송했습니다: %s\n", text);
    return 0;
}