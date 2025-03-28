#include "../incs/define.h"

void handle_stt_result(const char *text) {
    printf("STT result: %s\n", text);

    if(strcmp(text, "엘리베이터 시작") == 0 || strcmp(text, "엘레베이터 시작") == 0|| strcmp(text, "elevator start") == 0)
    {       
        send_text_to_server(text);
    }   
}