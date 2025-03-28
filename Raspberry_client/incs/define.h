#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <glib.h>
#include <glib/gbase64.h>
#include <pthread.h>
#include <alsa/asoundlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>

#define GOOGLE_STT_API "https://speech.googleapis.com/v1/speech:recognize?key=AIzaSyDAPXSoj4TgR6HhaOHK9Kvbl0ahNTmCh2Q"
#define AUDIO_PATH "/home/pi/Final_Project/carelift/Raspberry_client/voices"

#define SAMPLE_RATE 16000
#define CHANNELS 1
#define FORMAT SND_PCM_FORMAT_S16_LE
#define CHUNK_DURATION_MS 3000  // 3초 단위로 STT 처리
#define FRAMES_PER_CHUNK (SAMPLE_RATE * CHUNK_DURATION_MS / 1000)
#define BYTES_PER_SAMPLE 2  // 16비트 오디오
#define BUFFER_SIZE (FRAMES_PER_CHUNK * CHANNELS * BYTES_PER_SAMPLE)

#define SERVER_IP "10.10.141.72"  
#define SERVER_PORT 5000

struct ResponseBuffer {
    char *data;
    size_t size;
};

struct STTThreadArg {
    void *audio_data;
    size_t audio_size;
    void (*callback)(const char *text);  
};


extern int is_connected;
extern volatile int recording_flag;

int connect_to_server();
void disconnect_from_server(int server_socket);
int send_text_to_server(const char *text);

void *audio_capture_thread(void *arg);
void handle_signal(int sig);
pthread_t start_realtime_stt(void (*callback)(const char *), int socket_fd);

size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *userp);
char *encode_binary_base64(const void *data, size_t size);
char* extract_transcript_from_json(const char *json_str, void (*callback)(const char *));
void *STT_thread(void *arg);

void handle_stt_result(const char *text);

void *server_response_thread(void *arg);
void play_tts(const char *text);

void set_socket_fd(int fd);
