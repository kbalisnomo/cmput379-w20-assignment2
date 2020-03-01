#include "prodcon.h"

sem_t *queue_sem, *empty, *full, *file_sem;
FILE *file;
clock_t start;
struct Queue *queue;
int finished = 0;
int ask = 0, receive = 0, complete = 0;

/* Writes to log file. 
    - id:   the ID of thread
    - mode: string to specify the mode to format
    - Q:    number of work left in queue (0 when Q is not needed)
    - n:    number taken from queue (0 when n is not needed) */
void write_to_log(int id, char *mode, int Q, int n) {
    clock_t stop;
    stop = clock();
    sem_wait(file_sem);
    if (strcmp(mode, "Ask") == 0 || strcmp(mode, "End") == 0) {
        fprintf(file, "\t%1.3f ID= %d      %-10s\n", ((double)(stop-start)/CLOCKS_PER_SEC), id, mode);
    }
    else if (strcmp(mode, "Receive") == 0 || strcmp(mode, "command") == 0) {
        fprintf(file, "\t%1.3f ID= %d Q= %d %-10s %5d\n", ((double)(stop-start)/CLOCKS_PER_SEC), id, Q, mode, n);
    }
    else if (strcmp(mode, "Sleep") == 0 || strcmp(mode, "Complete") == 0) {
        fprintf(file, "\t%1.3f ID= %d      %-10s %5d\n", ((double)(stop-start)/CLOCKS_PER_SEC), id, mode, n);
    }
    sem_post(file_sem);
}

void *consumer(void* args_p) {
    int *id = (int *) args_p;
    clock_t stop;
    int n, Q;
    int *work_count = malloc(sizeof(int));
    *work_count = 0;
    while(finished == 0) {
        //ask
        sem_wait(full);
        write_to_log(*id, "Ask", 0, 0);
        ask++;
        //receive
        sem_wait(queue_sem);
        n = dequeue(queue);
        Q = queue->size;
        sem_post(queue_sem);
        sem_post(empty);
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
    clock_t stop;
    int num_consumers = atoi(argv[1]);
    int SIZE = num_consumers * 2; //queue's size
    int ID[num_consumers];
    pthread_t TID[num_consumers];
    queue = createQueue(SIZE);
    char command[3];
    int work, sleep;
    
    //initialize semaphores
    // for non-mac (remove pointer, change all functions)
    // sem_init(&queue_sem, 0, 1);
    // sem_init(&file_sem, 0, 1);
    // sem_init(&empty, 0, SIZE);
    // sem_init(&full, 0, 0);
    //for mac (add pointer, change all functions)
    queue_sem = sem_open("/queue_sem", O_CREAT, 0644, 1);
    file_sem = sem_open("/file_sem", O_CREAT, 0644, 1);
    empty = sem_open("/empty", O_CREAT, 0644, SIZE);
    full = sem_open("/full", O_CREAT, 0644, 0);
    
    //Create log file
    char filename[20];
    if (argc == 3) {
        strcpy(filename, "prodcon.");
        strcat(filename, argv[2]);
        strcat(filename, ".log");
    } else {
        strcpy(filename, "prodcon.log");
    }
    file = fopen(filename, "w");

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
            sem_wait(empty);
            sem_wait(queue_sem);
            enqueue(queue, atoi(&command[1]));
            write_to_log(0, "command", queue->size, atoi(&command[1]));
            sem_post(queue_sem);
            sem_post(full);
            work++;
        }
        else if (command[0] == 'S') {
            write_to_log(0, "Sleep", 0, atoi(&command[1]));
            Sleep(atoi(&command[1]));
            sleep++;
        }
    }
    //End 
    finished = 1;
    write_to_log(0, "End", 0, 0);
    int work_counts[num_consumers];
    void *status;
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(TID[i], &status);
        work_counts[i] = *(int*)status;
    }
    stop = clock();
    //Write summary
    fprintf(file, "Summary:\n");
    fprintf(file, "\t%-10s%5d\n", "Work", work);
    fprintf(file, "\t%-10s%5d\n", "Ask", ask);
    fprintf(file, "\t%-10s%5d\n", "Receive", receive);
    fprintf(file, "\t%-10s%5d\n", "Complete", complete);
    fprintf(file, "\t%-10s%5d\n", "Sleep", sleep);
    for (int i = 0; i < num_consumers; i++) {
        fprintf(file, "\t%-5s %-4d%4d\n", "Thread", ID[i], work_counts[i]);
    }
    fprintf(file, "Transactions per second: %.2f", work / ((double)(stop-start)/CLOCKS_PER_SEC));

    sem_unlink("/queue_sem");
    sem_unlink("file_sem");
    sem_unlink("/empty");
    sem_unlink("/full");
    fclose(file);

    return 0;
}