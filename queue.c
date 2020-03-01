/* A queue implementation. */
#include "queue.h"
 
struct Queue* createQueue(unsigned capacity) 
{ 
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue)); 
    queue->capacity = capacity; 
    queue->front = queue->size = 0;  
    queue->rear = capacity - 1;  // This is important, see the enqueue 
    queue->array = (int*) malloc(queue->capacity * sizeof(int)); 
    return queue; 
} 

// Check if the queue is full
int isFull(struct Queue* queue) 
{  return (queue->size == queue->capacity);  } 
  
// Check if the queue is empty
int isEmpty(struct Queue* queue) 
{  return (queue->size == 0); } 

// Add an item to the queue
void enqueue(struct Queue* queue, int item) 
{ 
    if (isFull(queue)) 
        return; 
    queue->rear = (queue->rear + 1)%queue->capacity; 
    queue->array[queue->rear] = item; 
    queue->size = queue->size + 1; 
} 

// Remove an item from the queue
int dequeue(struct Queue* queue) 
{ 
    if (isEmpty(queue)) 
        return INT_MIN; 
    int item = queue->array[queue->front]; 
    queue->front = (queue->front + 1)%queue->capacity; 
    queue->size = queue->size - 1; 
    return item; 
} 
  
// Get the front of the queue
int front(struct Queue* queue) 
{ 
    if (isEmpty(queue)) 
        return INT_MIN; 
    return queue->array[queue->front]; 
} 
  
// Get the rear of the queue
int rear(struct Queue* queue) 
{ 
    if (isEmpty(queue)) 
        return INT_MIN; 
    return queue->array[queue->rear]; 
} 