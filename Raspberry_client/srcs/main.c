#include "../incs/define.h"

int is_connected = 0;
volatile int recording_flag = 1;


int main() {

    int socket_fd;

    curl_global_init(CURL_GLOBAL_ALL);
    socket_fd = connect_to_server();
    if (socket_fd < 0) {
        fprintf(stderr, "Server conneection failed.\n");
        curl_global_cleanup();
        return 1;
    }

    set_socket_fd(socket_fd);
    pthread_t response_thread;
    if (pthread_create(&response_thread, NULL, server_response_thread, &socket_fd) != 0) {
        fprintf(stderr, "Creating server response thread failed.\n");
        disconnect_from_server(socket_fd);
        curl_global_cleanup();
        return 1;
    }

    pthread_t audio_thread = start_realtime_stt(handle_stt_result, socket_fd);

    if (audio_thread) {
        printf("Voice recognition and server connection started...\n");
        pthread_join(audio_thread, NULL);
    }

    disconnect_from_server(socket_fd);
    curl_global_cleanup();
    return 0;
}

