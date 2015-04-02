/*COMMENTS ACABADOS*/
#ifndef  __UTILIS_H__
#define  __UTILIS_H__

#include "request.h"
#include "thrpool.h"
#include <sys/socket.h>
#include <signal.h>

int fsocket;

void free_resources();
void config_file();
void handler(int);
int max_threads();
int backlog();

#endif
