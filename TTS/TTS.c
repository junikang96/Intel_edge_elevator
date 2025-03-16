#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <glib.h>

#define GOOGLE_TTS_API "https://texttospeech.googleapis.com/v1/text:synthesize?key=AIzaSyDAPXSoj4TgR6HhaOHK9Kvbl0ahNTmCh2Q"
#define MAX_TEXT_LENGTH 1024

// HTTP 응답을 저장할 구조체
struct ResponseBuffer {
    char *data;
    size_t size;
};

// HTTP 응답 저장 콜백 함수
static size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct ResponseBuffer *buf = (struct ResponseBuffer *)userp;
    size_t new_size = buf->size + (size * nmemb);
    buf->data = realloc(buf->data, new_size + 1);
    memcpy(&(buf->data[buf->size]), ptr, size * nmemb);
    buf->size = new_size;
    buf->data[new_size] = '\0';  // Null-terminate
    return size * nmemb;
}

int main(int argc, char *argv[]) {
    char input_text[MAX_TEXT_LENGTH] = {0};
    char gender[10] = "FEMALE"; // 기본값
    char output_filename[256] = "output.mp3"; // 기본값
    
    // 명령줄 인수로 입력받기
    if (argc > 1) {
        strncpy(input_text, argv[1], MAX_TEXT_LENGTH - 1);
    } else {
        printf("TTS로 변환할 텍스트를 입력하세요: ");
        fgets(input_text, MAX_TEXT_LENGTH, stdin);
        
        // 줄바꿈 문자 제거
        size_t len = strlen(input_text);
        if (len > 0 && input_text[len-1] == '\n') {
            input_text[len-1] = '\0';
        }
    }
    
    // 성별 입력 받기
    if (argc > 2) {
        strncpy(gender, argv[2], sizeof(gender) - 1);
    } else {
        printf("음성 성별을 입력하세요 (MALE/FEMALE/NEUTRAL, 기본값: FEMALE): ");
        char temp[32];
        fgets(temp, sizeof(temp), stdin);
        size_t gender_len = strlen(temp);
        if (gender_len > 0 && temp[gender_len-1] == '\n') {
            temp[gender_len-1] = '\0';
        }
        
        if (strlen(temp) > 0) {
            strncpy(gender, temp, sizeof(gender) - 1);
        }
    }
    
    // 대문자로 변환
    for (int i = 0; gender[i]; i++) {
        if (gender[i] >= 'a' && gender[i] <= 'z') {
            gender[i] = gender[i] - 'a' + 'A';
        }
    }
    
    // 성별 입력 검증
    if (strcmp(gender, "MALE") != 0 && strcmp(gender, "FEMALE") != 0 && strcmp(gender, "NEUTRAL") != 0) {
        printf("잘못된 성별 입력입니다. FEMALE로 설정합니다.\n");
        strcpy(gender, "FEMALE");
    }
    
    // 출력 파일명 입력 받기
    if (argc > 3) {
        strncpy(output_filename, argv[3], sizeof(output_filename) - 1);
    } else {
        printf("출력 파일명을 입력하세요 (기본값: output.mp3): ");
        char temp[256];
        fgets(temp, sizeof(temp), stdin);
        size_t filename_len = strlen(temp);
        if (filename_len > 0 && temp[filename_len-1] == '\n') {
            temp[filename_len-1] = '\0';
        }
        
        if (strlen(temp) > 0) {
            strncpy(output_filename, temp, sizeof(output_filename) - 1);
        }
    }
    
    // .mp3 확장자 확인 및 추가
    if (strlen(output_filename) < 4 || 
        strcmp(output_filename + strlen(output_filename) - 4, ".mp3") != 0) {
        strcat(output_filename, ".mp3");
    }
    
    // JSON 요청 데이터 생성
    char *json_payload = NULL;
    cJSON *root = cJSON_CreateObject();
    cJSON *input = cJSON_CreateObject();
    cJSON *voice = cJSON_CreateObject();
    cJSON *audioConfig = cJSON_CreateObject();
    
    cJSON_AddItemToObject(root, "input", input);
    cJSON_AddStringToObject(input, "text", input_text);
    
    cJSON_AddItemToObject(root, "voice", voice);
    cJSON_AddStringToObject(voice, "languageCode", "ko-KR");
    cJSON_AddStringToObject(voice, "ssmlGender", gender);
    
    cJSON_AddItemToObject(root, "audioConfig", audioConfig);
    cJSON_AddStringToObject(audioConfig, "audioEncoding", "MP3");
    
    json_payload = cJSON_Print(root);
    cJSON_Delete(root);
    
    // API 호출
    CURL *curl;
    CURLcode res;
    struct ResponseBuffer response = {NULL, 0};

    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, GOOGLE_TTS_API);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        printf("Google TTS API 호출 중...\n");
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() 실패: %s\n", curl_easy_strerror(res));
            free(json_payload);
            return 1;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    free(json_payload);

    if (response.data) {
        // JSON 응답 파싱
        cJSON *json = cJSON_Parse(response.data);
        if (json) {
            cJSON *audioContent = cJSON_GetObjectItem(json, "audioContent");
            if (cJSON_IsString(audioContent)) {
                // Base64 디코딩 (GLib 사용)
                gsize decoded_length;
                guchar *mp3_data = g_base64_decode(audioContent->valuestring, &decoded_length);

                // MP3 파일 저장 (바이너리 모드)
                FILE *fp = fopen(output_filename, "wb");
                if (fp) {
                    fwrite(mp3_data, 1, decoded_length, fp);
                    fclose(fp);
                    printf("MP3 파일 저장 완료: %s\n", output_filename);
                } else {
                    printf("MP3 파일 저장 실패\n");
                }

                g_free(mp3_data);
            } else {
                printf("오류: API 응답에서 audioContent를 찾을 수 없습니다.\n");
            }
            cJSON_Delete(json);
        } else {
            printf("오류: JSON 응답 파싱 실패\n");
            printf("응답 데이터: %s\n", response.data);
        }
        free(response.data);
    }

    return 0;
}