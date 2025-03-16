#include "../incs/main.h"

char threadName[5][20] = {"jetsonOutside", "jetsonInside", "raspberry", "arduino", "stm"};
void *(*threadFunc[5])(void *) = {jetsonOneThread, jetsonTwoThread, raspberryThread, arduinoThread, stmThread};

int main(void)
{
    int serverfd, max;
    fd_set fds, wfds, rfds;
    socklen_t addr_len;
    struct sockaddr_in addr;
    char bufRead[BUFSIZE];
    t_client clients[1024];
    int state;
    bool wheelChair, people;
    t_data **datas;

    serverfd = startSocket(&addr, &addr_len);
    datas = init();

    max = serverfd;
    printf("before while, max is:%d\n", max);
    FD_SET(serverfd, &fds);
    while (1) 
    {
        wfds = rfds = fds;
        // if (max == 5개 전부 연결한 값)
        // {
        //     스레드 끝내기
        //     할당해제
        //      메인스레드 시작
        // }


        if (select(max + 1, &wfds, &rfds, NULL, NULL) < 0)
            continue;

        for (int i = 0; i <= max; i++)
        {
            if (FD_ISSET(i, &wfds) && i == serverfd)
            {
                int clientSock = accept(serverfd, (struct sockaddr *)&addr, &addr_len);
                if (clientSock < 0)
                    continue;
                printf("accepted, clientfd is %d\n", clientSock);
                max = (clientSock > max) ? clientSock : max;
                printf("max is:%d\n", max);
                clients[clientSock].clientfd = clientSock;
                FD_SET(clientSock, &fds);
                break;
            }
            if (FD_ISSET(i, &wfds) && i != serverfd)
            {
                int res = read(i, bufRead, BUFSIZE);
                printf("res is:%d\n", res);
                if (res <= 0)
                {
                    FD_CLR(i, &fds);
                    close(i);
                    break;
                }
                else
                {
                    for (int j = 0; j < 5; j++)
                    {
                        if (strncmp(threadName[j], bufRead, res) == 0)
                        {
                            datas[j]->state = &state;
                            datas[j]->clientfd = clients[i].clientfd;
                            datas[j]->wheelChair = &wheelChair;
                            datas[j]->people = &people;
                            pthread_create(&(datas[j]->pid), NULL, threadFunc[j], (void *)datas[j]);
                            FD_CLR(i, &fds);
                            break;
                        }
                    }
                }
            }
        }
    }
    
}


