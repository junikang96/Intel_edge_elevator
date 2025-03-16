#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/select.h>

#define SERVER_PORT 5000
#define LISTEN_BACKLOG 10
#define BUFSIZE 1024

enum e_state 
{
    ARRIVED_DOWNSTAIRS,
    GET_RFID,
    BUTTON_PRESSED,
    ELEVATOR_START,
    SPEAKER_YIELD,
    SPEAKER_TAG_REQUIRED,
};

enum e_thread_order
{
    JETSON_INSIDE,
    JETSON_OUTSIDE,
    RASPBERRY,
    ARDUINO,
    STM,
};

typedef struct s_client
{
    pthread_t pid;
    int clientfd;
} t_client;

typedef struct s_data
{
    pthread_t pid;
    int *state;
    int clientfd;
    bool *wheelChair;
    bool *people;
} t_data;

//socket
int startSocket(struct sockaddr_in *addr_server, socklen_t *addr_len);

//init
t_data **init();

//stmThread
void    *stmThread(void *arg);

//arduinoThread
void    *arduinoThread(void *arg);

//jetsonOneThread
void    *jetsonOneThread(void *arg);

//jetsonTwoThread
void    *jetsonTwoThread(void *arg);

//raspberryThread
void    *raspberryThread(void *arg);


#endif