#include "../incs/define.h"

// STT 결과를 처리하는 콜백 함수 
void handle_stt_result(const char *text) {
    printf("STT result: %s\n", text);
    send_text_to_server(text);
}