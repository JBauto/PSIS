/*COMMENTS ACABADOS*/
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <pthread.h>
#include "request.h"

request_queue* request_queue_init(request_queue*, int);
request_queue* create_queue();
request_queue* insert_queue(request_queue*, request_form*,int);


#endif
