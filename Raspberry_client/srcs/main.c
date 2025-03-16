#include "../incs/define.h"

int server_socket = -1;
int is_connected = 0;
volatile int recording_flag = 1;

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    if (connect_to_server() < 0) {
        fprintf(stderr, "Server conneection failed.\n");
        curl_global_cleanup();
        return 1;
    }

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

