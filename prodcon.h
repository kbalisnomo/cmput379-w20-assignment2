#ifndef PRODCON_H
#define PRODCON_H
#include "queue.h"
#include "tands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
void *consumer(void* args_p);
#endif