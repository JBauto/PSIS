
#include "thrpool.h"

poolANDqueue* pqueue;
request_queue *queue;
pthread_mutex_t mthread_mutex= PTHREAD_MUTEX_INITIALIZER;

int PIPE_ERR;
int maxtime;
int mintime;
extern int mthr;
extern double avg_check;
extern double avg;
double total = 0;

/*A função master thread trata a master thread fazendo com que esta esteja encarregue das estatisticas dos pedidos recebidos e tratados.*/
/*As threads que tratam os pedidos enviam a sua informação para esta thread através de um pipe e esta guarda a mesma informação num ficheiro txt*/
/*Através de uma variável, a master thread pode ou não enviar para o browser todos os pedidos (Lista de pedidos) ou limpar a mesma lista (Clear All)*/
void* master_thread(void* tmp_pqueue){

    FILE* fp;
    poolANDqueue* tmpaux2 = (poolANDqueue*) tmp_pqueue;
    //request_queue* tmpaux = tmpaux2->temp_queue;
    request_form tmp;
    char *tmp_str;
    char *buf;
    int err = 3;
    int sizefp;
    //char file_line[BUFFSIZE];

    /********************************/
    /**       CRIAR MUTEX          **/
    /********************************/

    remove("pedidos.txt");
    fp = fopen("pedidos.txt","a+");
    if(fp == NULL){
        perror("master_thread(): Nao foi possivel abrir o ficheiro \"pedidos.txt\"");
        exit(-17);
    }
    fclose(fp);
    while(!pool_status){
        fp = fopen("pedidos.txt","a+");
        //printf("master thread a espera\n");
        read(tmpaux2->temp_pool->fd[0],(void*)&tmp,sizeof(tmp));
        //printf("master thread a ler\n");
        if(tmp.error<=2){
            err = tmp.error;
            tmp.error = 200;
        }
        fprintf(fp,"%-15s %-25s %-5d %-15s %-5lf#\n",tmp.data,tmp.buffer,tmp.error,tmp.request_IP,tmp.time);
        fclose(fp);
        tmp.error = err;
		/* erro 1 indica que deve listar todos os pedidos*/
        if(tmp.error == 1){
            fp = fopen("pedidos.txt","r");
            fseek(fp,0,SEEK_END);
            sizefp = ftell(fp);
            //printf("%d\n",sizefp);
            fseek(fp,0,SEEK_SET);
            buf = (char*) malloc(sizeof(char)*sizefp);
            bzero(buf,sizeof(buf));
            if(buf == NULL){
                perror("Nao foi possivel alocar memoria em master_thread\n");
                exit(-30);
            }
            fread(buf,1,sizefp,fp);
            tmp_str = strtok(buf,"#");
            dprintf(tmp.socket_fd, "HTTP/1.1 200 OK\n");
            dprintf(tmp.socket_fd, "Content-Type:text/html\n");
            dprintf(tmp.socket_fd, "\n");
            dprintf(tmp.socket_fd, "<HTML><HEADER><b>Lista de Pedidos</b><br><br></HEADER>\n");
            while(tmp_str !=NULL){
                dprintf(tmp.socket_fd, "<BODY>%s<br></BODY></HTML>\n",tmp_str);
                tmp_str = strtok(NULL,"#");
            }
            free(buf);
            close(tmp.socket_fd);
        }else{
		/* erro 2 indica que deve eliminar todos os pedidos*/
            if(tmp.error == 2){
            remove("pedidos.txt");
            dprintf(tmp.socket_fd, "HTTP/1.1 200 OK\n");
            dprintf(tmp.socket_fd, "Content-Type:text/html\n");
            dprintf(tmp.socket_fd, "\n");
            dprintf(tmp.socket_fd, "<HTML><HEADER><b>Todos os registos eliminados</b><br></HEADER></HTML>\n");
            close(tmp.socket_fd);
            }
        }
    }
    return;
}


/* Funçao que as threads executam. Nesta funçao a threads correm um ciclo enquanto o programa for suposto correr*/
/* As threads removem um pedido da lista, analisam o e enviam a resposta para o socket */
void* fulfill_req(void* id){

    int thread_id = (int) id;
    poolANDqueue* tmp2 = (poolANDqueue*) pqueue;
    //tpool_thread* tmp3 = tmp2->temp_pool;
    //request_queue* tmp = tmp2->temp_queue;
    request_form request;
    double time_elapsed;
    //int i=0;
    request.error = 0;
    request.buffer[0] = '\0';
    request.request_IP[0] = '\0';
    while(!pool_status){
		/* remove pedido da lista*/
        request = queue_remove(queue,thread_id);
        /* analisa o pedido*/
		request = handle_request(request,request.socket_fd);
		/* para o cronometro do pedido */
        clock_gettime(CLOCK_MONOTONIC, &(request.finish));
        time_elapsed = ((request.finish).tv_sec - (request.start).tv_sec);
        time_elapsed += ((request.finish).tv_nsec - (request.start).tv_nsec) / 1000000000.0;
        request.time = time_elapsed;
		/* escreve para a master thread o pedido */
        write(tmp2->temp_pool->fd[1],(void*)&request,sizeof(request));
		if(request.error<3)
			request.error = 200;
        if(PIPE_ERR == 0){
		/* se nao recebeu o sinal sigpipe entao o pedido foi respondido antes da ligaçao fechar*/
           printf("%-25s %-5d %-20s %-10lf\n",request.buffer,request.error,request.request_IP,request.time);
        }else{
           printf("Ligacao fechada antes de responder\n");
           PIPE_ERR = 0;
        }
		/* reset das variaveis */
        request.error = 0;
        request.time = 0;
        request.buffer[0] = '\0';
        request.request_IP[0] = '\0';
        request.socket_fd = 0;
        (request.start).tv_nsec = 0;
        (request.start).tv_sec = 0;
        (request.finish).tv_nsec = 0;
        (request.finish).tv_sec = 0;
    }
}

/* Criaçao da pool de threads */
tpool_thread* create_threadpool(request_queue* queue){

    tpool_thread* pool;
    //request_form* req_pool;
    pthread_attr_t attr;
    int calc;
    //int id;
    int nthreads = 20;

	/* estrutura que tem um ponteiro para uma pool e uma queue de pedidos */
    pqueue =(poolANDqueue*) malloc(sizeof(poolANDqueue));
    if(pqueue == NULL){
        perror("create_threadpool(): Impossivel criar a struct poolANDqueue. A sair...\n");
        exit(-16);
    }
    memset(pqueue,0,sizeof(pqueue));
    /* aloca espaço para a pool de threads */
	pool = (tpool_thread*) malloc(sizeof(tpool_thread));
    if(pool == NULL){
        perror("create_threadpool(): Impossivel criar thread pool. A sair...\n");
        exit(-6);
    }
    memset(pool,0,sizeof(pool));
	/* aloca vector para as threads */
    pool->threads = (pthread_t*) malloc(nthreads*sizeof(pthread_t));
    if((pool->threads) == NULL){
        perror("create_threadpool(): Impossivel criar threads. A sair...\n");
        exit(-7);
    }
    memset(pool->threads,0,sizeof(pool->threads));
	/* cria as master threads, 1 master thread por 5 threads */
    pool->master_thread = (pthread_t*) malloc((nthreads/5)*sizeof(pthread_t));
    if((pool->master_thread) == NULL){
        perror("create_threadpool(): Impossivel criar master threads. A sair...\n");
        exit(-7);
    }
    memset(pool->master_thread,0,sizeof(pool->master_thread));
    int i,j,k;
    pqueue->temp_queue = queue;
    pqueue->temp_pool = pool;
	/* cria as threads como detached */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pool->nthreads = nthreads;
    calc = pool->nthreads / MAX_QUEUE;
    k=0;

    for(i=0,j=0;i<pool->nthreads;i++,j++){
	/* inicializa as threads */
        if((pthread_create(&(pool->threads[i]), &attr, &fulfill_req, (void*)k))<0){
            perror("Cant create threads\n");
            exit(-13);
        }
        if(j == calc - 1){
            j=-1;
            k++;
        }

    }
    for(i=0;i<pool->nthreads;i++){
        if(pthread_detach(pqueue->temp_pool->threads[i])==0)
           printf("detact thread\n");
    }
	/* pipe entre as master threads e as threads*/
    if(pipe(pool->fd)<0){
        perror("Cant create pipe between slaves and master\n");
        exit(-15);
    }

    pqueue->temp_queue = queue;
    pqueue->temp_pool = pool;
    /*inicializa as master threads */
    for(i=0;i<pool->nthreads/5;i++){
        if((pthread_create(&(pool->master_thread[i]),&attr, &master_thread,(void*)pqueue))<0){
            perror("Cant create threads\n");
            exit(-13);
        }
        if(pthread_detach(pqueue->temp_pool->master_thread[i])==0)
            printf("detact m thread\n");
    }
    return pool;
}
