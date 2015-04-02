
#include "request.h"

//int vec[MAX_QUEUE]={0};
//int vect[MAX_QUEUE]={0};

//int id = -1;
request_form *aux[MAX_QUEUE];
pthread_mutex_t mlocal = PTHREAD_MUTEX_INITIALIZER;

/*Função que trata da execução dos cgis através de um processo filho*/
void resolve_cgi(char * strficheiro, char * strfile, int fd_socket){

    int status;
    pid_t pid = fork();

    if(pid == 0){
        close(1);
        dup2(fd_socket,1);
        alarm(5);
        execl(strficheiro, strfile, NULL);
    }else{
        waitpid(pid, &status, 0);
    }

 return;
}

/* Função que recebe a informaçao contida num pedido (request_form) e resolve o mesmo consoanteo pretendido sendo o resultado enviado para o socket,e por sua vez para o browser*/
request_form handle_request(request_form req, int fd_socket){

    struct stat sb;
    int ficheiro;
    char buffer[BUFFSIZE];
    //char tmp_buffer[BUFFSIZE+1];
    char namefile[BUFFSIZE];
    char extension[BUFFSIZE];
    char content_type[100];
    char cgi_teste[BUFFSIZE];
    char cgi_ficheiro[BUFFSIZE];
    char strget[BUFFSIZE], strfile[BUFFSIZE], strhttp[BUFFSIZE], strext[BUFFSIZE], strficheiro[BUFFSIZE];
    char *tok_str, *tok_tmp;
    DIR *d;
    struct dirent *directory;
    int termerro=0;
    request_form tmp = req;
    int flag = 0;
    int ret;
    strfile[0]='\0';
    strext[0]='\0';
    strhttp[0]='\0';
    strget[0]='\0';
    strficheiro[0]='\0';
    namefile[0]='\0';
    extension[0]='\0';
    termerro=0;

   count++;
   //printf("Request #%d\n",count);

  // printf("scanning buffer\n");

   if((sscanf(tmp.buffer, "%s %[^' '] %s", strget, strfile, strhttp))!=3){
       strcpy(strfile,"N/A");
       strcpy(extension,"Outro");
       strcpy(strhttp,"HTTP/1.1"); /* REVER */
       dprintf(fd_socket, "HTTP/1.1 400 Bad Request\n");
       dprintf(fd_socket, "Content-Type:text/html\n");
       dprintf(fd_socket, "\n");
       dprintf(fd_socket, "<HTML><HEAD><TITLE>%s 400 Bad Request</TITLE></HEAD>\n",strhttp);
       dprintf(fd_socket, "<BODY> %s 400 Bad Request </BODY></HTML>\n",strhttp);
       termerro = 400;
       req.error = termerro;
       strcpy(req.buffer,strfile);
       close(fd_socket);
       return req;
   }

   if((strstr(tmp.buffer,"\r\n") == NULL))
       printf("1\n");
   /*printf("GET ---> %s\n", strget);
   printf("FICHEIRO ---> %s\n", strfile);
   printf("PROTOCOLO ---> %s\n", strhttp);
   //printf("ERRO ---> %d\n",termerro);
   //printf("EXTENSAO ---> %s\n",strext);
   //printf("erro : %d\n",req.error);*/
   sscanf(strfile, "%[^'.'].%[^' ']",namefile,strext);
   //printf("orig file %s\n",strfile);
   strcpy(cgi_ficheiro,namefile);
   if(strcmp(cgi_ficheiro,"/")!=0){
     tok_str = strtok(cgi_ficheiro,"/");
     while(tok_str!=NULL){
        tok_tmp = tok_str;
        tok_str = strtok(NULL,"/");
     }
     strcat(tok_tmp,".");
     strcat(tok_tmp,strext);
   }
   //printf("1g\n");
   /*Verifica Bad Request*/
   if(strcmp(strget,"GET")!=0 || (strcmp(strhttp,"HTTP/1.1")!=0 && strcmp(strhttp,"HTTP/1.0")!=0)){
       strcpy(strfile,"N/A");
       strcpy(extension,"Outro");
       strcpy(strhttp,"HTTP/1.1"); /* REVER */
       dprintf(fd_socket, "HTTP/1.1 400 Bad Request\n");
       dprintf(fd_socket, "Content-Type:text/html\n");
       dprintf(fd_socket, "\n");
       dprintf(fd_socket, "<HTML><HEAD><TITLE>%s 400 Bad Request</TITLE></HEAD>\n",strhttp);
       dprintf(fd_socket, "<BODY> %s 400 Bad Request </BODY></HTML>\n",strhttp);
       termerro = 400;
   }else{
    //printf("1f\n");
       if(!strcmp(strfile,"/estatisticas/Pedidos")){
        //printf("1\n");
           termerro = 1;
           req.error = termerro;
           strcpy(req.buffer,strfile);
           return req;
       }else{
        //printf("1d\n");
           if(!strcmp(strfile,"/estatisticas/ClearAll")){
               termerro = 2;
               req.error = termerro;
               strcpy(req.buffer,strfile);
               return req;
           }else{
            //printf("1s\n");
                  //printf("file %s %d\n",strfile,sizeof(strfile));
                  if(strchr(strfile,'.')==NULL && (strlen(strfile)>1)){
                    strcat(namefile,"/");
                  }
			/* se o ultimo caracter for / assume-se que é uma directoria e tenta-se listar o seu interior*/
             if(namefile[strlen(namefile)-1]=='/'){
                 strcpy(strficheiro,DOCUMENT_ROOT);
                 strcat(strficheiro,namefile);
                 d = opendir(strficheiro);
                 if(d != NULL){
                     //printf("directory\n");
                     dprintf(fd_socket, "%s 200 OK\n",strhttp);
                     dprintf(fd_socket, "Content-Type:text/html\n");
                     dprintf(fd_socket, "\n");
                     dprintf(fd_socket, "<HTML><HEAD><TITLE></TITLE></HEAD>\n");
                     while ((directory = readdir(d)) != NULL){
                         dprintf(fd_socket, "<HTML><BODY> <a href=\"%s\"</a>%s<br></BODY></HTML>\n",directory->d_name,directory->d_name);
                     }
                     closedir(d);
                     termerro = 200;
                     strcpy(req.buffer,strfile);
                     req.error = termerro;
                     close(fd_socket);
                     return req;
                 }else{
				/* nao consegue listar o interior entao a directoria nao existe*/
                     termerro=404;
                     strcpy(strhttp,"HTTP/1.1");
                     dprintf(fd_socket, "%s 404 Not Found\n",strhttp);
                     dprintf(fd_socket, "Content-Type:text/html\n");
                     dprintf(fd_socket, "\n");
                     dprintf(fd_socket, "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\n");
                     dprintf(fd_socket, "<BODY> directoria %s nao existe. </BODY></HTML>\n", strficheiro);
                     closedir(d);
                     strcpy(req.buffer,strfile);
                     req.error = termerro;
                     close(fd_socket);
                     return req;
                 }
             }
			  /* a string cgi_teste contem o caminho correcto para os cgi's*/
               strcat(cgi_teste,DOCUMENT_ROOT);
               strcat(cgi_teste,"/");
               strcat(cgi_teste,CGI_ROOT);
               if(strcmp(CGI_ROOT,"\0")!=0)
					strcat(cgi_teste,"/");
               if(flag == 0){
                            strcpy(strficheiro,DOCUMENT_ROOT);
                            strcat(strficheiro,namefile);
                            strcat(strficheiro,".");
                            strcat(strficheiro,strext);
                            strcpy(extension,strext);
                            //printf("strficheiro1 = %s\n", strficheiro);
                }
               if (stat(strficheiro, &sb) == 0 && sb.st_mode & S_IXUSR){
               /* se os primeiros caracteres da string que contem o ficheiro coincidirem com a string que tem o caminho correcto do
					cgi entao esta-se a tentar executar o cgi na directoria correcta, CGI_ROOT. Caso contrario o caminho é
					invalido e é 403 Forbidden */
                    if(strncmp(cgi_teste,strficheiro,strlen(cgi_teste))!=0){
                        termerro=403;
                        dprintf(fd_socket, "%s 403 Forbidden\n",strhttp);
                        dprintf(fd_socket, "Content-Type:text/html\n");
                        dprintf(fd_socket, "\n");
                        dprintf(fd_socket, "<HTML><HEAD><TITLE>403 Forbidden</TITLE></HEAD>\n");
                        dprintf(fd_socket, "<BODY> ficheiro %s nao foi encontrado na directoria %s. </BODY></HTML>\n", strficheiro,CGI_ROOT);
                        req.error = termerro;
                        strcpy(req.buffer,strfile);
                        close(fd_socket);
                        return req;
                    }else{
                        resolve_cgi(strficheiro, strfile, fd_socket);
                        termerro = 200;
                        req.error = termerro;
                        strcpy(req.buffer,strfile);
                        close(fd_socket);
                        return req;
                    }
              }else{
				/* nao é executavel verifica se a extensao*/
                  if(strcmp(strext,"html")!=0 && strcmp(strext,"png")!=0){

                           /* Qualquer extensão diferente de html e png dá erro 415*/
                           termerro=415;
                           dprintf(fd_socket, "%s 415 Unsupported Media Type\n",strhttp);
                           dprintf(fd_socket, "Content-Type:text/html\n");
                           dprintf(fd_socket, "\n");
                           dprintf(fd_socket, "<HTML><HEAD><TITLE>%s 415 Unsupported Media Type</TITLE></HEAD>\n",strhttp);
                           dprintf(fd_socket, "<BODY> %s 415 Unsupported Media Type </BODY></HTML>\n", strhttp);
                           dprintf(fd_socket, "\n");

                   }else{
                        if(strcmp(strext,"html") == 0){
                            strcpy(content_type, "text/html");
                            ficheiro = open (strficheiro,O_RDONLY);
                        }else{
                            strcpy(content_type, "image/png");
                            ficheiro = open (strficheiro,O_RDONLY);
                        }
					/* se nao se conseguir abrir o ficheiro entao 404 Not Found */
                        if(ficheiro < 0){
                           termerro=404;
                           dprintf(fd_socket, "%s 404 Not Found\n",strhttp);
                           dprintf(fd_socket, "Content-Type:text/html\n");
                           dprintf(fd_socket, "\n");
                           dprintf(fd_socket, "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD>\n");
                           dprintf(fd_socket, "<BODY> ficheiro %s nao foi encontrado. </BODY></HTML>\n", strficheiro);
                        }else{
                           termerro=200;
                           dprintf(fd_socket, "%s 200 OK\n",strhttp );
                           dprintf(fd_socket, "Content-Type:%s\n",content_type);
                           dprintf(fd_socket, "\n");
                           memset(buffer, '\0', BUFFSIZE+1);
                           while ((ret = read(ficheiro, buffer, BUFFSIZE)) > 0){
                               (void)write(fd_socket,buffer,ret);
                               memset(buffer, '\0', BUFFSIZE+1);
                           }
                            close(ficheiro);
                         }
                        }
                        }
                }
            }
        }

   flag = 0;
   strcpy(req.buffer,strfile);
   req.error = termerro;
   /*printf("GET ---> %s\n", strget);
   printf("FICHEIRO ---> %s\n", strfile);
   printf("PROTOCOLO ---> %s\n", strhttp);
   printf("ERRO ---> %d\n",termerro);
   printf("EXTENSAO ---> %s\n",strext);*/
   //printf("erro : %d\n",req.error);
   close(fd_socket);
   return req;
}


/* NAO SEI O QUE FAZ A CERTO */
request_form queue_remove(request_queue* queue,int id){

    request_form tmp;
    /* Cada thread é criada com um id associado. Este id indica-lhe de que lista vai retirar o pedido*/
	/* Isto garante que MAX_QUEUE threads podem responder a pedidos. Uma thread entra na funçao e consoanteo
	 o seu id obtem o lock para a sua lista. Se vier outra thread com um id diferente esta nao bloqueia desde
	 que o lock do seu id nao tenha dono */
    aux[id] = queue->queue_beg[id];
    pthread_mutex_lock(&queue->lock[id]);
    while(queue->queue_beg[id] == NULL){
       if(pool_status == 1)
            break;
       pthread_cond_wait(&queue->notempty[id],&queue->lock[id]);
    }
    /* Condiçao que garante que o programa deve continuar */
    if(pool_status == 0){
        tmp.buffer[0] = '\0';
        tmp.request_IP[0] = '\0';
        tmp.data[0] = '\0';
        tmp.socket_fd = 0;
        tmp.error = 0;
        strcpy(tmp.buffer,queue->queue_beg[id]->buffer);
        strcpy(tmp.request_IP, queue->queue_beg[id]->request_IP);
        //printf("remove insere %s\n",tmp.request_IP);
        strcpy(tmp.data,queue->queue_beg[id]->data);
        tmp.socket_fd = queue->queue_beg[id]->socket_fd;
        tmp.error = queue->queue_beg[id]->error;
        tmp.start = queue->queue_beg[id]->start;
        tmp.finish = queue->queue_beg[id]->finish;

        aux[id] = queue->queue_beg[id];
        queue->queue_beg[id] = queue->queue_beg[id]->next;
        aux[id]->next = NULL;
        free(aux[id]);
        aux[id] = NULL;
        queue->curr_request-=1;
    }
    pthread_mutex_unlock(&queue->lock[id]);
    return tmp;
}
