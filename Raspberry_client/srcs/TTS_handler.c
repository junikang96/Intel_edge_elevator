#include "../incs/define.h"

void *server_response_thread(void *arg) {
    char buffer[1024];
    int bytes_received;
    
    while (is_connected) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            printf("서버 연결이 끊겼습니다.\n");
            is_connected = 0;
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("서버로부터 응답 수신: %s\n", buffer);
        
        if (strncmp(buffer, "TTS:", 4) == 0) {
            char *tts_text = buffer + 4;  
            play_tts(tts_text);
        }
    }
    
    return NULL;
}

void play_tts(const char *text) {
    char command[1024];
    char filename[256];
    
    if (strcmp(text, "floor1") == 0) {
        strcpy(filename, "floor1_FEMALE.mp3");
    } 
    else if (strcmp(text, "floor2") == 0) {
        strcpy(filename, "floor2_FEMALE.mp3");
    }
    else if (strcmp(text, "door_open") == 0) {
        strcpy(filename, "door_open_FEMALE.mp3");
    }
    else if (strcmp(text, "door_close") == 0) {
        strcpy(filename, "door_close_FEMALE.mp3");
    }
    else {
        printf("해당 음성 파일이 없습니다: %s\n", text);
        return;
    }
    
    snprintf(command, sizeof(command), "mpg123 /home/pi/Final_Project/Raspberry_client_test/voices/%s", filename);
    system(command);
    printf("오디오 재생: %s\n", filename);
}