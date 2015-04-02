
#include "queue.h"

/*Função que inicializa uma queue de pedidos depois de iniciada, inclui a inicialização dos mutexes correspondentes a cada lista de pedidos
 e das condições dests mesmos mutexes*/
request_queue* request_queue_init(request_queue* queue, int max_request){

    queue->curr_request = 0;
    int i;

    /*if(sem_init(&queue->sem,0,MAX_QUEUE) < 0){
        perror("Nao foi possivel iniciar semaphore.A sair...\n");
        exit(-25);
    }*/

    for(i=0;i<MAX_QUEUE;i++){
        queue->queue_beg[i] = NULL;
        queue->queue_end[i] = NULL;
    }
    for(i=0;i<MAX_QUEUE;i++){
        if(pthread_mutex_init(&queue->lock[i],NULL) != 0){
            perror("Nao foi possivel inicializar mutex\n. A sair...");
            exit(-21);
        }
        if(pthread_cond_init(&queue->notempty[i],NULL) != 0){
            perror("Nao foi possivel inicializar condiçoes do mutex\n. A sair...");
            exit(-22);
        }
        if(pthread_cond_init(&queue->notfull[i],NULL) != 0){
            perror("Nao foi possivel inicializar condiçoes do mutex\n. A sair...");
            exit(-23);
        }
    }
    return queue;
}

/*Função que cria uma queue de pedidos e a inicializa*/
request_queue* create_queue(){

    request_queue* tmp;
    //int i;

    tmp = (request_queue*) malloc(sizeof(request_queue));
    if(tmp == NULL){
        perror("Cant create queue\n");
        exit(-11);
    }
    memset(tmp,0,sizeof(tmp));
    return tmp;
}

/* Funçao que insere o pedido numa lista consoante a variavel i. Sao criadas MAX_QUEUE listas e os pedidos sao distribuidos
igualmente pelas listas. Os dados sao entao copiados para a lista */
/* Se o inicio da lista for NULL é criado uma head da lista caso contrario ja existem pedidos na lista e é inserido no fim */
request_queue* insert_queue(request_queue* queue, request_form* rlist,int i){

    //request_form* tmp;
    //request_form* aux;
	/* cada lista tem um mutex associado */
    pthread_mutex_lock(&queue->lock[i]);
    if(queue->queue_beg[i] == NULL){
       queue->queue_beg[i] = (request_form*) malloc(sizeof(request_form));
        if(queue->queue_beg[i] == NULL){
            perror("Nao foi possivel criar a lista. A sair\n");
            exit(-18);
        }
        //printf("Sou %d e inicio %d \n",getpid(),i);
        memset(queue->queue_beg[i],0,sizeof(queue->queue_beg[i]));
        queue->queue_beg[i]->buffer[0] = '\0';
        queue->queue_beg[i]->request_IP[0] = '\0';
        queue->queue_beg[i]->data[0]='\0';
        strcpy(queue->queue_beg[i]->buffer,rlist->buffer);
        strcpy(queue->queue_beg[i]->request_IP,rlist->request_IP);
        strcpy(queue->queue_beg[i]->data,rlist->data);
        queue->queue_beg[i]->time = rlist->time;
        queue->queue_beg[i]->start = rlist->start;
        queue->queue_beg[i]->finish = rlist->finish;
        queue->queue_beg[i]->error = rlist->error;
        queue->queue_beg[i]->socket_fd = rlist->socket_fd;
        queue->curr_request++;
        queue->queue_beg[i]->next = NULL;
        queue->queue_end[i] = queue->queue_beg[i];
    }else{

        queue->queue_end[i]->next = (request_form*) malloc(sizeof(request_form));
        if(queue->queue_end[i]->next == NULL){
            perror("Não possivel continuar a lista. A sair\n");
            exit(-19);
        }
        //printf("Sou %d e meio %d \n",getpid(),i);
        //printf("meio %d com ponteiro %p\n",i,queue->queue_end[i]->next);
        memset(queue->queue_end[i]->next,0,sizeof(queue->queue_end[i]->next));
        //printf("vou inserir fim\n");
        queue->queue_end[i]->next->buffer[0] = '\0';
        queue->queue_end[i]->next->request_IP[0] = '\0';
        queue->queue_end[i]->next->data[0]='\0';
        rlist->buffer[strlen(rlist->buffer)-1] = '\0';
        rlist->request_IP[strlen(rlist->request_IP)-1] = '\0';
        rlist->data[strlen(rlist->data)-1] = '\0';
        strcpy(queue->queue_end[i]->next->buffer,rlist->buffer);
        strcpy(queue->queue_end[i]->next->request_IP,rlist->request_IP);
        strcpy(queue->queue_end[i]->next->data,rlist->data);
        queue->queue_end[i]->next->time = rlist->time;
        queue->queue_end[i]->next->start = rlist->start;
        queue->queue_end[i]->next->finish = rlist->finish;
        queue->queue_end[i]->next->error = rlist->error;
        queue->queue_end[i]->next->socket_fd = rlist->socket_fd;
        queue->curr_request++;
        queue->queue_end[i] = queue->queue_end[i]->next;
        queue->queue_end[i]->next = NULL;
    }
    free(rlist);
    pthread_mutex_unlock(&queue->lock[i]);
    //printf("acabei de inserir %d\n",getpid());
	/* se existirem threads bloqueadas na funçao queue_remove é enviado um sinal de que foi inserido um pedido */
    pthread_cond_signal(&queue->notempty[i]);
    return queue;
}

