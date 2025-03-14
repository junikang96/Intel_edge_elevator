#include "../incs/STT_client.h"

int server_socket = -1;
int is_connected = 0;
volatile int recording_flag = 1;

// 서버로부터 데이터를 수신하는 스레드 함수
void *server_response_thread(void *arg) {
    char buffer[1024];
    int bytes_received;
    
    while (is_connected) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            // 연결 끊김 처리
            printf("서버 연결이 끊겼습니다.\n");
            is_connected = 0;
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("서버로부터 응답 수신: %s\n", buffer);
        
        // TTS 명령 처리
        if (strncmp(buffer, "TTS:", 4) == 0) {
            char *tts_text = buffer + 4;  // "TTS:" 다음의 텍스트
            play_tts(tts_text);
        }
    }
    
    return NULL;
}

// TTS 실행 함수
void play_tts(const char *text) {
    char command[1024];
    char filename[256];
    
    // 텍스트에 따라 미리 녹음된 파일 결정
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
        // 기본 오디오 파일 또는 에러 처리
        printf("해당 음성 파일이 없습니다: %s\n", text);
        return;
    }
    
    // 오디오 파일 재생
    snprintf(command, sizeof(command), "mpg123 /home/pi/Final_Project/Raspberry_client_test/voices/%s", filename);
    system(command);
    printf("오디오 재생: %s\n", filename);
}


int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    if (connect_to_server() < 0) {
        fprintf(stderr, "Server conneection failed.\n");
        curl_global_cleanup();
        return 1;
    }

    // 서버 응답 수신 스레드 시작
    pthread_t response_thread;
    if (pthread_create(&response_thread, NULL, server_response_thread, NULL) != 0) {
        fprintf(stderr, "서버 응답 스레드 생성 실패\n");
        disconnect_from_server();
        curl_global_cleanup();
        return 1;
    }

    pthread_t audio_thread = start_realtime_stt(handle_stt_result);

    if (audio_thread) {
        printf("Voice recognition and server connection started...\n");
        pthread_join(audio_thread, NULL);
    }

    disconnect_from_server();
    curl_global_cleanup();
    return 0;
}

// STT 결과를 처리하는 콜백 함수 
void handle_stt_result(const char *text) {
    printf("STT result: %s\n", text);
    send_text_to_server(text);
}