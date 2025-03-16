#include "../incs/define.h"

size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct ResponseBuffer *buf = (struct ResponseBuffer *)userp;
    size_t new_size = buf->size + (size * nmemb);
    buf->data = realloc(buf->data, new_size + 1);
    if (buf->data == NULL) {
        return 0;
    }
    memcpy(&(buf->data[buf->size]), ptr, size * nmemb);
    buf->size = new_size;
    buf->data[new_size] = '\0';  
    return size * nmemb;
}

char *encode_binary_base64(const void *data, size_t size) {
    // Base64 인코딩 (GLib 사용)
    char *encoded = g_base64_encode((const guchar *)data, size);
    return encoded;
}

// JSON 응답에서 텍스트 추출
char* extract_transcript_from_json(const char *json_str, void (*callback)(const char *)) {
    static char empty_result[] = "";
    cJSON *json = NULL;
    
    // JSON 파싱
    json = cJSON_Parse(json_str);
    if (!json) {
        printf("JSON phasing error: %s\n", json_str);
        return empty_result;
    }
    
    // results 배열 가져오기
    cJSON *results = cJSON_GetObjectItem(json, "results");
    if (!results || !cJSON_IsArray(results) || cJSON_GetArraySize(results) == 0) {
        printf("No recognized test.\n");
        cJSON_Delete(json);
        return empty_result;
    }
    
    // 첫 번째 결과 가져오기
    cJSON *first_result = cJSON_GetArrayItem(results, 0);
    if (!first_result) {
        cJSON_Delete(json);
        return empty_result;
    }
    
    // alternatives 배열 가져오기
    cJSON *alternatives = cJSON_GetObjectItem(first_result, "alternatives");
    if (!alternatives || !cJSON_IsArray(alternatives) || cJSON_GetArraySize(alternatives) == 0) {
        cJSON_Delete(json);
        return empty_result;
    }
    
    // 첫 번째 대안 가져오기
    cJSON *first_alt = cJSON_GetArrayItem(alternatives, 0);
    if (!first_alt) {
        cJSON_Delete(json);
        return empty_result;
    }
    
    // transcript 가져오기
    cJSON *transcript = cJSON_GetObjectItem(first_alt, "transcript");
    if (!transcript || !cJSON_IsString(transcript) || !transcript->valuestring) {
        cJSON_Delete(json);
        return empty_result;
    }
    
    // 결과 출력 및 콜백 호출
    printf("Recognized text: %s\n", transcript->valuestring);
    fflush(stdout);
    
    if (callback) {
        callback(transcript->valuestring);
    }
    
    // 임시 버퍼에 결과 복사
    static char result_buffer[4096];
    strncpy(result_buffer, transcript->valuestring, sizeof(result_buffer) - 1);
    result_buffer[sizeof(result_buffer) - 1] = '\0';
    
    cJSON_Delete(json);
    return result_buffer;
}

void *STT_thread(void *arg) {
    struct STTThreadArg *thread_arg = (struct STTThreadArg *)arg;
    CURL *curl;
    CURLcode res;
    struct ResponseBuffer response = {NULL, 0};

    // 오디오 데이터를 Base64로 변환
    char *audio_base64 = encode_binary_base64(thread_arg->audio_data, thread_arg->audio_size);
    if (!audio_base64) {
        printf("Base64 encoding failed.\n");
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

    // JSON 응답 파싱 및 텍스트 추출
    if (response.data) {
        extract_transcript_from_json(response.data, thread_arg->callback);
        free(response.data);
    }

    free(thread_arg->audio_data);
    free(thread_arg);
    pthread_exit(NULL);
}