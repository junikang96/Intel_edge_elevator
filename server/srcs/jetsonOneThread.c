#include "../incs/main.h"

//outside

void    *jetsonOneThread(void *arg)
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
            *(jetsonData->wheelChair) = 1;
            printf("wheelchair detacted\n");
        }
        else
        {
            *(jetsonData->wheelChair) = 0;
            printf("wheelChair not detacted\n");
        }
    }
    return (NULL);
}