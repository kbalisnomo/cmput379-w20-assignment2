/* CMPUT379 W20 Assignment 2 Submission. Producer-consumer problem.
   Name: Kyle Philip Balisnomo
   CCID: kylephil                                                  */

#include "prodcon.h"

sem_t queue_sem, empty, full, file_sem;
FILE *file;
clock_t start;
struct Queue *queue;
int finished = 0;
int ask = 0, receive = 0, work = 0, complete = 0, sleep = 0;

/* Returns real time elapsed since start of program. */
double get_time() {
    clock_t stop = clock();
    return (double)(stop-start) / CLOCKS_PER_SEC;
}

/* Create and open a log file with a name specified by the requirements. 
    - argcount: how many args the program is run with
    - id:       id that represents the id of the log file to be made */
void create_log(int argcount, char *id) {
    char filename[20];
    if (argcount == 3) {
        strcpy(filename, "prodcon.");
        strcat(filename, id);
        strcat(filename, ".log");
    } else {
        strcpy(filename, "prodcon.log");
    }
    file = fopen(filename, "w");
}

/* Writes to log file. 
    - id:   the ID of thread
    - mode: string to specify the mode to format
    - Q:    number of work left in queue (0 when Q is not needed)
    - n:    number taken from queue (0 when n is not needed) */
void write_to_log(int id, char *mode, int Q, int n) {
    sem_wait(&file_sem);
    if (strcmp(mode, "Ask") == 0 || strcmp(mode, "End") == 0) {
        fprintf(file, "\t%1.3f ID= %d      %-10s\n", get_time(), id, mode);
    }
    else if (strcmp(mode, "Receive") == 0 || strcmp(mode, "Work") == 0) {
        fprintf(file, "\t%1.3f ID= %d Q= %d %-10s %5d\n", get_time(), id, Q, mode, n);
    }
    else if (strcmp(mode, "Sleep") == 0 || strcmp(mode, "Complete") == 0) {
        fprintf(file, "\t%1.3f ID= %d      %-10s %5d\n", get_time(), id, mode, n);
    }
    sem_post(&file_sem);
}

/* Print out a summary. 
    - total_time:    total real time used to execute program
    - num_consumers: max number of consumers
    - ID[]:          array of thread IDs
    - work_counts:   array of individual thread work counts */
void print_summary(double total_time, int num_consumers, int ID[], int work_counts[]) {
    fprintf(file, "Summary:\n");
    fprintf(file, "\t%-10s%5d\n", "Work", work);
    fprintf(file, "\t%-10s%5d\n", "Ask", ask);
    fprintf(file, "\t%-10s%5d\n", "Receive", receive);
    fprintf(file, "\t%-10s%5d\n", "Complete", complete);
    fprintf(file, "\t%-10s%5d\n", "Sleep", sleep);
    for (int i = 0; i < num_consumers; i++) {
        fprintf(file, "\t%-5s %-4d%4d\n", "Thread", ID[i], work_counts[i]);
    }
    fprintf(file, "Transactions per second: %.2f", work / total_time);
}

/* Details the consumer workflow of ask, receive, complete. */
void *consumer(void* args_p) {
    int *id = (int *) args_p;
    clock_t stop;
    int n, Q;
    int *work_count = malloc(sizeof(int));
    *work_count = 0;
    while(finished == 0) {
        //ask
        sem_wait(&full);
        sem_wait(&queue_sem);
        write_to_log(*id, "Ask", 0, 0);
        ask++;
        //receive
        n = dequeue(queue);
        Q = queue->size;
        sem_post(&queue_sem);
        sem_post(&empty);
        write_to_log(*id, "Receive", Q, n);
        receive++;
        //complete
        Trans(n);
        write_to_log(*id, "Complete", 0, n);
        complete++;
        *work_count+= 1;

    }
    return work_count;   
}

int main(int argc, char *argv[]) {
    start = clock();
    int num_consumers = atoi(argv[1]);
    int SIZE = num_consumers * 2; //queue's size
    int ID[num_consumers];
    pthread_t TID[num_consumers];
    queue = createQueue(SIZE);
    char command[3];
    
    // initialize semaphores
    // for non-mac (remove pointer, change all functions)
    sem_init(&queue_sem, 0, 1);
    sem_init(&file_sem, 0, 1);
    sem_init(&empty, 0, SIZE);
    sem_init(&full, 0, 0);
    // for mac (add pointer, change all functions)
    // sem_unlink("/queue_sem");
    // sem_unlink("file_sem");
    // sem_unlink("/empty");
    // sem_unlink("/full");
    // queue_sem = sem_open("/queue_sem", O_CREAT, 0644, 1);
    // file_sem = sem_open("/file_sem", O_CREAT, 0644, 1);
    // empty = sem_open("/empty", O_CREAT, 0644, SIZE);
    // full = sem_open("/full", O_CREAT, 0644, 0);
    
    create_log(argc, argv[2]);

    //Create threads
    for (int i = 0; i < num_consumers; i++) {
        ID[i] = i+1;
    }
    int error;
    for(int i = 0; i < num_consumers; i++) {
        error = pthread_create(&TID[i], NULL, consumer, &ID[i]);
        if (error) {                          
            printf("ERROR; return code from pthread_create() is %d\n", error);                            
            exit(-1);                          
        }  
    }

    //Fill queue
    while(scanf("%s", command) != EOF) {
        if (command[0] == 'T') {
            sem_wait(&empty);
            sem_wait(&queue_sem);
            enqueue(queue, atoi(&command[1]));
            write_to_log(0, "Work", queue->size, atoi(&command[1]));
            sem_post(&queue_sem);
            sem_post(&full);
            work++;
        }
        else if (command[0] == 'S') {
            write_to_log(0, "Sleep", 0, atoi(&command[1]));
            Sleep(atoi(&command[1]));
            sleep++;
        }
    }
    //End
    write_to_log(0, "End", 0, 0);

    // Notify the consumers that there is no more work
    while(finished == 0) {
        if (isEmpty(queue)) {
            finished = 1;
        }
    }

    // Wait for all consumers to finish work
    int work_counts[num_consumers];
    void *status;
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(TID[i], &status);
        work_counts[i] = *(int*)status;
    }
    double total_time = get_time();

    print_summary(total_time, num_consumers, ID, work_counts);

    // for non-mac
    sem_destroy(&queue_sem);
    sem_destroy(&file_sem);
    sem_destroy(&empty);
    sem_destroy(&full);
    // for mac
    // sem_unlink("/queue_sem");
    // sem_unlink("file_sem");
    // sem_unlink("/empty");
    // sem_unlink("/full");

    fclose(file);

    return 0;
}