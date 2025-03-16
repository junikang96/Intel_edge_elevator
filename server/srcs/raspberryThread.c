#include "../incs/main.h"

void voice_command_handling(t_data *raspberryData, const char *command) {
    printf("voice command handling: %s\n", command);
    char response[BUFSIZE];

    if (strstr(command, "1층") != NULL || strstr(command, "일층") != NULL) {
        printf("1층 이동 음성 감지\n");
        *(raspberryData->state) = 0;
        // 엘리베이터 제어 명령 전달


        // 음성 출력 명령 전달
        strcpy(response, "TTS:floor1");
        printf("sending to client: %s\n", response);
        write(raspberryData->clientfd, response, strlen(response));
        
    }
    else if (strstr(command, "2층") != NULL || strstr(command, "이층") != NULL) {
        printf("2층 이동 음성 감지\n");
        // 엘리베이터 제어 명령 전달


        // 음성 출력 명령 전달
        strcpy(response, "TTS:floor2");
        printf("sending to client: %s\n", response);
        write(raspberryData->clientfd, response, strlen(response));
    }

    else if (strstr(command, "문 열어") != NULL) {
 a       printf("문 열림 음성 감지\n");
        // 엘리베이터 제어 명령 전달


        // 음성 출력 명령 전달
        strcpy(response, "TTS:door_open");
        printf("sending to client: %s\n", response);
        write(raspberryData->clientfd, response, strlen(response));
    }
    else if (strstr(command, "문 닫아") != NULL ) {
        printf("문 닫힘 음성 감지\n");
        // 엘리베이터 제어 명령 전달


        // 음성 출력 명령 전달
        strcpy(response, "TTS:door_close");
        printf("sending to client: %s\n", response);
        write(raspberryData->clientfd, response, strlen(response));
    }
}

void *raspberryThread(void *arg)
{
    t_data *raspberryData = (t_data *)arg;
    char buf[BUFSIZE];
    int ret;
    printf("RaspberryPi Thread started, clientfd: %d\n", raspberryData->clientfd);
  
    while (1)
    {
        memset(buf, 0, BUFSIZE); 
        ret = read(raspberryData->clientfd, buf, BUFSIZE - 1);
        if (ret <= 0)
        {
            printf("RaspberryPi Connection Error\n");
            break;
        }
        buf[ret] = '\0'; 
        printf("RaspberryPi Data: %s\n", buf);

        voice_command_handling(raspberryData, buf);
    }
    
    printf("RaspberryPi Thread terminated.\n");
    return (NULL);
}