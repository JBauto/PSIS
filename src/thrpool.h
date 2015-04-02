/*COMMENTS ACABADOS*/
#ifndef __THRPOOL_H__
#define __THRPOOL_H__

#include "queue.h"

#define MAX_THREAD 10

/*Estrutura referente à pool de threads, contém o ponteiro para as threads, tal como a master thread, o número de threads da mesma e o file descriptor */
typedef struct tpool_thread{
    pthread_t* threads;
    pthread_t* master_thread;
    int nthreads; /* numero de threads */
    int fd[2]; /* file descriptor */
}tpool_thread;

/* Estrutura que contém o ponteiro para a pool de threads e outro ponteiro para a queue de pedidos*/
typedef struct poolANDqueue{
    tpool_thread* temp_pool;
    request_queue* temp_queue;
}poolANDqueue;



tpool_thread* create_threadpool(request_queue*);
void* fulfill_req(void*);
void* master_thread(void*);

#endif
