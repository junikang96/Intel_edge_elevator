#include "../incs/define.h"

void handle_stt_result(const char *text) {
    printf("STT result: %s\n", text);
    send_text_to_server(text);
}