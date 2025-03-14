#include "../incs/STT_client.h"

// HTTP 응답 저장 콜백
size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct ResponseBuffer *buf = (struct ResponseBuffer *)userp;
    size_t new_size = buf->size + (size * nmemb);
    buf->data = realloc(buf->data, new_size + 1);
    if (buf->data == NULL) {
        return 0;
    }
    memcpy(&(buf->data[buf->size]), ptr, size * nmemb);
    buf->size = new_size;
    buf->data[new_size] = '\0';  // Null-terminate
    return size * nmemb;
}

// 바이너리 데이터를 Base64로 인코딩
char *encode_binary_base64(const void *data, size_t size) {
    // Base64 인코딩 (GLib 사용)
    char *encoded = g_base64_encode((const guchar *)data, size);
    return encoded;
}

// STT 처리를 위한 스레드
void *thread_STT(void *arg) {
    struct STTThreadArg *thread_arg = (struct STTThreadArg *)arg;
    CURL *curl;
    CURLcode res;
    struct ResponseBuffer response = {NULL, 0};

    // 오디오 데이터를 Base64로 변환
    char *audio_base64 = encode_binary_base64(thread_arg->audio_data, thread_arg->audio_size);
    if (!audio_base64) {
        printf("오디오 데이터를 Base64로 인코딩할 수 없습니다.\n");
        free(thread_arg->audio_data);
        free(thread_arg);
        pthread_exit(NULL);
    }

    // JSON 요청 데이터 생성
    char *json_payload = (char *)malloc(strlen(audio_base64) + 200);
    if (!json_payload) {
        free(audio_base64);
        free(thread_arg->audio_data);
        free(thread_arg);
        pthread_exit(NULL);
    }
    sprintf(json_payload,
             "{"
             "\"config\": {"
             "\"encoding\": \"LINEAR16\","
             "\"sampleRateHertz\": %d,"
             "\"languageCode\": \"ko-KR\""
             "},"
             "\"audio\": {"
             "\"content\": \"%s\""
             "}"
             "}", SAMPLE_RATE, audio_base64);

    free(audio_base64);  // Base64 메모리 해제

    // Google STT API 요청
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, GOOGLE_STT_API);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(json_payload);
        
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            if (response.data) free(response.data);
            free(thread_arg->audio_data);
            free(thread_arg);
            pthread_exit(NULL);
        }
    } else {
        free(json_payload);
        free(thread_arg->audio_data);
        free(thread_arg);
        pthread_exit(NULL);
    }

    // JSON 응답 파싱
    if (response.data) {
        cJSON *json = cJSON_Parse(response.data);
        if (json) {
            cJSON *results = cJSON_GetObjectItem(json, "results");
            if (cJSON_IsArray(results) && cJSON_GetArraySize(results) > 0) {
                cJSON *first_result = cJSON_GetArrayItem(results, 0);
                if (first_result) {
                    cJSON *alternatives = cJSON_GetObjectItem(first_result, "alternatives");
                    if (cJSON_IsArray(alternatives) && cJSON_GetArraySize(alternatives) > 0) {
                        cJSON *first_alt = cJSON_GetArrayItem(alternatives, 0);
                        if (first_alt) {
                            cJSON *transcript = cJSON_GetObjectItem(first_alt, "transcript");
                            if (cJSON_IsString(transcript) && transcript->valuestring != NULL) {
                                printf("인식된 텍스트: %s\n", transcript->valuestring);
                                fflush(stdout);
                                
                                // 콜백 함수가 있으면 결과 전달
                                if (thread_arg->callback) {
                                    thread_arg->callback(transcript->valuestring);
                                }
                            }
                        }
                    }
                }
            } else {
                printf("인식된 텍스트 없음\n");
            }
            cJSON_Delete(json);
        } else {
            printf("JSON 파싱 오류: %s\n", response.data);
        }
        free(response.data);
    }

    free(thread_arg->audio_data);
    free(thread_arg);
    pthread_exit(NULL);
}