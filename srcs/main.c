#include "../incs/STT_client.h"

int server_socket = -1;
int is_connected = 0;
volatile int recording_flag = 1;

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    if (connect_to_server() < 0) {
        fprintf(stderr, "서버 연결에 실패했습니다. 프로그램을 종료합니다.\n");
        curl_global_cleanup();
        return 1;
    }

    pthread_t audio_thread = start_realtime_stt(handle_stt_result);

    if (audio_thread) {
        printf("음성 인식 및 서버 통신이 시작되었습니다...\n");
        pthread_join(audio_thread, NULL);
    }

    disconnect_from_server();
    curl_global_cleanup();
    return 0;
}

// STT 결과를 처리하는 콜백 함수 (서버로 전송 추가)
void handle_stt_result(const char *text) {
    printf("STT 결과 처리: %s\n", text);
    send_text_to_server(text);
}