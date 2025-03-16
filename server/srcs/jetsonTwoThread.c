#include "../incs/main.h"

//inside

void    *jetsonTwoThread(void *arg)
{
    t_data *jetsonData = (t_data *)arg;
    char buf[BUFSIZE];

    printf("jetson outside thread starting, clientfd is %d\n", jetsonData->clientfd);
    while (1)
    {
        int ret = read(jetsonData->clientfd, buf, BUFSIZE);
        if (ret <= 0)
            printf("read error\n");

        if (strncmp(buf, "1", ret) == 0)
        {
            *(jetsonData->people) = 1;
            printf("people detacted\n");
        }
        else
        {
            *(jetsonData->people) = 0;
            printf("people not detacted\n");
        }
    }
    return (NULL);
}