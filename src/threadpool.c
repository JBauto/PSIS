#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include "utilis.h"

#define PORT 8080
#define MAX_PENDING 512
#define MAX_THREAD 10
#define MAX_PROC 10

int mpend;
int maxthr;
int mthr;
int process_monitor;
int PIPE_ERR;
double avg_check;
double avg;
pthread_t pthread_self();
//int vect[MAX_QUEUE];


/***********************/
poolANDqueue* pqueue;
request_queue *queue;
request_form *client_request;
/***********************/

/*Função que cria o socket que faz a conexão entre o cliente e o servidor*/
int create_socket(){
    int sock;
    struct sockaddr_in server;
    char host[1024];
    struct hostent *host_entry;
    char * local_IP;
    int port;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Erro ao abrir socket");
        exit(-1);
    }
    port = 8080;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /*do{
		port++;
        	server.sin_port = htons(port);       //server port

	 //Bind the server socket

	}while(bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) < 0);
	*/

    if (bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_in))) {
        perror("Erro ao fazer o binding do socket");
        exit(-2);
    }

    gethostname(host, 1024);
    host_entry = gethostbyname(host);
    local_IP = (char*) inet_ntoa (*(struct in_addr *)*host_entry->h_addr_list);
    printf("\n\n\t Use um dos seguintes enderecos no navegador: \n");
    printf("\t\thttp://%s:%d/\n", host,port);
    printf("\tou\n");
    printf("\t\tServidor em: http://%s:%d/\n", local_IP,port);

    if (listen(sock, mpend) < 0) {
        perror("Impossivel criar o socket. O servidor vai sair.\n");
        exit(-3);
    }

    return sock;
}

/*Função que através do socket espera a conexão de clientes ao servidor*/
int  espera_pedido(int serversock, FILE** fp_read, FILE ** fp_write, char *remote_address){

	int clientsock;
	struct sockaddr_in  echoclient;

	unsigned int clientlen = sizeof(echoclient);

	/* Wait for client connection */
    if(!pool_status){
        if ((clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
                perror("Impossivel estabelecer ligacao ao cliente. O servidor vai sair.\n");
                exit(1);
            }
        sprintf(remote_address, "%s", (char*)inet_ntoa(echoclient.sin_addr));
    }
	return clientsock;

}


int main(int argc, char *argv[]){

    int numbytes,j;
    int pid;
    int fd_fork[2];
    int sock_ind;
    int inc_proc;
    char request_IP[32];
    char buffer[BUFFSIZE];
    time_t now;
    struct tm *ts;

    FILE * fp_read, *fp_write;

	/* Se apanhar um dos seguintes sinais é chamada a funçao handler que
	 para sinais diferentes executa funçoes diferentes */


    if(signal(SIGINT, handler) == SIG_ERR){
        printf("Nao é possivel tratar o sinal\n. A continuar...\n");
    }
    if(signal(SIGPIPE, handler) == SIG_ERR){
        printf("Nao é possivel tratar o sinal\n. A continuar...\n");
    }
    if(signal(SIGTERM, handler) == SIG_ERR){
        printf("Nao é possivel tratar o sinal\n. A continuar...\n");
    }
    if(signal(SIGUSR1, handler) == SIG_ERR){
        printf("Nao é possivel tratar o sinal\n. A continuar...\n");
    }

	/* Determina o valor máximo para a funçao listen */
    mpend = backlog();
	/* Numero maximo de threads possiveis */
    maxthr = max_threads();
	/* Cria o socket */
    fsocket = create_socket();


    printf("\n       (estabilidade verificada até 1800 users)\n\n");
    config_file();

    if(pipe(fd_fork)<0){
        perror("pipe");
        exit(-31);
    }
    j = 0;
    process_monitor = 50;
    while(!pool_status){
        while(j<process_monitor){
		/* Cada processo tem uma pool de threads, MAX_QUEUE listas e master threads responsaveis pelas estatisticas */
                if((pid=fork())==0){
                        int child_i = 0;
                        int pc_count=1;
			int pc_check = 0;
                        tpool_thread* pool;
                        queue = create_queue();
                        queue = request_queue_init(queue,MAX_PENDING);
                        pool = create_threadpool(queue);
                        //printf("%d\n",j);
                        while(!pool_status){
						/* é feito o accept do pedido*/
                            sock_ind = espera_pedido(fsocket, &fp_read, &fp_write, request_IP);
                            client_request = (request_form*) malloc(sizeof(request_form));
                            if(client_request == NULL){
                                    perror("client_request");
                                    exit(-25);
                            }
                            client_request->socket_fd = sock_ind;
							/* Se o recv retornar um numero de bits menor que zero entao nao leu informaçao
							valida e fecha este socket */
                            if((numbytes=recv(client_request->socket_fd,buffer,BUFFSIZE,0)) < 0 ){
                                printf("%s\n", strerror(errno));
                                printf("o cliente desligou a ligacao\n");
                                close(client_request->socket_fd);
                            }else{
                                now = time(NULL);
								/* data a que o pedido foi recebido */
                                ts = localtime(&now);
                                strftime(client_request->data, sizeof(client_request->data),"%-d/%-m/%-Y %-H:%-M:%-S",ts);
                                /* inicia o cronometro do pedido */
								clock_gettime(CLOCK_MONOTONIC, &(client_request->start));
                                strcpy(client_request->buffer, buffer);
                                strcpy(client_request->request_IP, request_IP);
								/* insere na lista indicada por i*/
                                queue = insert_queue(queue, client_request,child_i);
                                if(queue->curr_request>=25){
                                    if(pc_check == 0){
                                        write(fd_fork[1],&pc_count,sizeof(pc_count));
                                        pc_check = 1;
                                    }
                                }
                                //printf("%d\n",process_monitor);
                            }
                            if(child_i == MAX_QUEUE - 1)
                                child_i = 0 ;
                            else
                                child_i++;
                        }
                }
                j++;
            }

        j = process_monitor;
        read(fd_fork[0],&inc_proc,sizeof(inc_proc));
        if(inc_proc == 1){
			if(process_monitor<500)
				process_monitor += 5;
		}
    }

    return 0;
}
