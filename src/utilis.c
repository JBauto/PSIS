

#include "utilis.h"

poolANDqueue* pqueue;
request_queue *queue;
request_form *client_request;
pthread_mutex_t mutex_clean = PTHREAD_MUTEX_INITIALIZER;
int mthr;
int PIPE_ERR;

/* Função chamada quando o sevidor recebe o sinal para terminar. esta função trata de fechar o socket, de termninar as threads e de liberta a memória */
void free_resources(){

    //int i;

    pool_status = 1;
    pthread_mutex_lock(&mutex_clean);
    //printf("starting clean up\n");
    close(fsocket);
    /*if(client_request!=NULL)
        free(client_request);
    
    for(i=0;i<pqueue->temp_pool->nthreads;i++){
        pthread_cancel(pqueue->temp_pool->threads[i]);
    }
    for(i=0;i<pqueue->temp_pool->nthreads/5;i++){
        pthread_cancel(pqueue->temp_pool->master_thread[i]);
    }*/

    //if(!pthread_detach(pqueue->temp_pool->master_thread))
    //      printf("Cant detact thread\n");
    //pthread_cond_destroy(&pqueue->temp_queue->notempty);
    //pthread_cond_destroy(&pqueue->temp_queue->notfull);
    //pthread_mutex_destroy(&pqueue->temp_queue->lock);
    //pthread_join(pqueue->temp_pool->master_thread,NULL);
    free(DOCUMENT_ROOT);
    free(CGI_ROOT);
    //free((void*)pqueue->temp_pool->threads);
    //free((void*)pqueue->temp_pool->master_thread);
    //free((void*)pqueue->temp_pool);
    //free((void*)queue);
    //free((void*)pqueue);
    printf("finished clean up\n");
    signal(SIGCHLD, SIG_IGN);
    kill(getpid(),SIGHUP);
}


/* Função que faz a leitura do ficheiro de configuração que define a DOCUMENT_ROOT e a CGI_ROOT, se não houver ficheiro para lr predefine ambas as directorias */
void config_file(){

    FILE* conf;
    int sizeconf;
    char* tmp_str;
    char* buf;
    int i;

    buf = (char*) malloc(sizeof(char)*100);
    if(buf == NULL){
        perror("Nao foi possivel alocar memoria em config_file\n");
        exit(-30);
    }
    conf = fopen("www.config","r");
    if(conf == NULL){
        printf("Nao existe ficheiro de configuração. A usar http_docs por default.\n");
        DOCUMENT_ROOT = (char*) malloc(sizeof(char)*100);
        if(DOCUMENT_ROOT == NULL){
            perror("Nao foi possivel alocar memoria em config_file\n");
            exit(-30);
        }
        CGI_ROOT = (char*) malloc(sizeof(char)*100);
        if(CGI_ROOT == NULL){
            perror("Nao foi possivel alocar memoria em config_file\n");
            exit(-30);
        }
        bzero(CGI_ROOT,sizeof(CGI_ROOT));
        bzero(DOCUMENT_ROOT,sizeof(DOCUMENT_ROOT));
        strcpy(DOCUMENT_ROOT,"http_docs");
        strcpy(CGI_ROOT,"\0");
    }else{
        DOCUMENT_ROOT = (char*) malloc(sizeof(char)*100);
        CGI_ROOT = (char*) malloc(sizeof(char)*100);
        fseek(conf,0,SEEK_END);
        sizeconf = ftell(conf);
        //printf("%d\n",sizefp);
        fseek(conf,0,SEEK_SET);
        bzero(DOCUMENT_ROOT,sizeof(DOCUMENT_ROOT));
        bzero(CGI_ROOT,sizeof(CGI_ROOT));
        if(DOCUMENT_ROOT == NULL){
            perror("Nao foi possivel alocar memoria em config_file\n");
            exit(-30);
        }
        if(CGI_ROOT == NULL){
            perror("Nao foi possivel alocar memoria em config_file\n");
            exit(-30);
        }
        fread(buf,1,sizeconf,conf);
        tmp_str = strtok(buf,"\n");
        i=1;
        DOCUMENT_ROOT[0]='\0';
        CGI_ROOT[0]='\0';
        while(tmp_str !=NULL){
            if(i==1)
                strcpy(DOCUMENT_ROOT,tmp_str);
            else
                if(i==2)
                    strcpy(CGI_ROOT,tmp_str);
            tmp_str = strtok(NULL,"\n");
            i++;
        }
        printf("Directoria de ficheiros : %s\n",DOCUMENT_ROOT);
        printf("Directoria de CGI's : %s\n",CGI_ROOT);
    }
    free(buf);
}

/* FUnção de tratamento de sinais, incluindo SIGINT, SIGTERM, SIGPIPE e SIGUSR1*/
void handler(int sign){



    if(sign == SIGINT || sign == SIGTERM ){
        //printf("\nReceived SIGINT or SIGTERM\n");
        free_resources();
        //printf("Clean up finished. Exiting...\n");
        exit(0);
    }
    if(sign == SIGPIPE){
        PIPE_ERR = 1;
    }
    if(sign == SIGUSR1)
        //printf("signal\n");
        config_file();
}

/*Função que retorna o numer maximo de threads que se podem criar cnsoant a maquina utilizada*/
int max_threads(){

    FILE * fp;
    int m_threads;
    char buffer[100];

    fp = fopen("/proc/sys/kernel/threads-max","r");
    fgets(buffer, 100, fp);
    sscanf(buffer, "%d", &m_threads);
    fclose(fp);
    return m_threads;
}


/*Função que retorna o numer maximo que o listen pode receber*/
int backlog(){

    FILE * fp;
    int b_log;
    char buffer[100];

    fp = fopen("/proc/sys/net/ipv4/tcp_max_syn_backlog","r");
    fgets(buffer, 100, fp);
    sscanf(buffer, "%d", &b_log);
    fclose(fp);
    return b_log;
}
