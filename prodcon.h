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

double get_time();
void create_log(int argcount, char *id);
void write_to_log(int id, char *mode, int Q, int n);
void print_summary(double total_time, int num_consumers, int ID[], int work_counts[]);
void *consumer(void* args_p);

#endif