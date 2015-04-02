#ifndef  __REQUEST_H__
#define  __REQUEST_H__

#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
/*COMMENTS ACABADOS*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFSIZE 1024
#define MAX_QUEUE 5

int PIPE_ERR;
int count;
int pool_status;
//int vect[MAX_QUEUE]={0};
char* DOCUMENT_ROOT;
char* CGI_ROOT;

/*Estrutura do pedido com informação do mesmo*/
typedef struct request_form{
    double time;
    struct timespec start, finish;
    char buffer[BUFFSIZE];
    char request_IP[50];
    char data[100];
    int error;
    int socket_fd;
    int val;
    struct request_form *next;
}request_form;

/*Estrutura correspondente à queue de pedidos, contém  o vector maximo de pedidos na queue , o pontero para o inicio da queue e para o final, e a informação do mutex */
/* que indica se a queue seencontra cheia ou não */
typedef struct request_queue{
    //request_form queue[MAX_PENDING];
    //int head, tail;
    request_form *queue_end[MAX_QUEUE], *queue_beg[MAX_QUEUE];
    int curr_request;
    int vec[MAX_QUEUE];
    sem_t sem;
    pthread_mutex_t lock[MAX_QUEUE];
    pthread_cond_t notempty[MAX_QUEUE];
    pthread_cond_t notfull[MAX_QUEUE];
}request_queue;

request_form handle_request(request_form, int);
request_form queue_remove(request_queue*,int);

#endif
