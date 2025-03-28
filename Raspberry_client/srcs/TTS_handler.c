#include "../incs/define.h"

void *server_response_thread(void *arg) 
{
    char buffer[1024];
    int bytes_received;
    int *serverfd = (int *)arg;

    while (is_connected) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(*serverfd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            printf("Server disconnected.\n");
            is_connected = 0;
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("Response from server: %s\n", buffer);
        
        if (strncmp(buffer, "TTS:", 4) == 0) {
            char *tts_text = buffer + 4;  
            play_tts(tts_text);
        }
    }
    
    return NULL;
}

void play_tts(const char *text) {
    char command[1024];
    char filename[256];
    
    if (strcmp(text, "elevator_start") == 0) {
        strcpy(filename, "elevator_start.mp3");
    }
    else if (strcmp(text, "door_open") == 0) {
        strcpy(filename, "door_open_FEMALE.mp3");
    }
    else if (strcmp(text, "door_close") == 0) {
        strcpy(filename, "door_close_FEMALE.mp3");
    }   
    else if (strcmp(text, "elevator_end") == 0) {
        strcpy(filename, "elevator_end.mp3");
    }
    else if (strcmp(text, "wheelchair") == 0) {
        strcpy(filename, "wheelchair_FEMALE.mp3");
    }
    else {
        printf("No audio files: %s\n", text);
        return;
    }
    
    snprintf(command, sizeof(command), "mpg123 %s/%s", AUDIO_PATH, filename);
    system(command);
    printf("Audio Playing: %s\n", filename);
}