#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 5000

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char message[1024];
    
    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    // 서버 주소 설정
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // IP 주소 변환
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    // 서버에 연결
    printf("Trying to connect to server at 127.0.0.1:%d\n", PORT);
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed: %s \n", strerror(errno));
        return -1;
    }
    
    printf("Connected to server\n");
    
    // 메시지 송수신
    while(1) {
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);
        
        // 'exit' 입력 시 종료
        if (strncmp(message, "exit", 4) == 0) {
            break;
        }
        
        send(sock, message, strlen(message), 0);
        printf("Message sent\n");
        
        memset(buffer, 0, sizeof(buffer));
        int valread = read(sock, buffer, 1024);
        printf("Server response: %s\n", buffer);
    }
    
    close(sock);
    return 0;
}