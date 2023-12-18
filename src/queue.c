#include "queue.h"
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

// node in the queue
typedef struct node{ 
    void *value;
    struct node *next;
} node_t;

typedef struct queue{
    node_t *head;
    node_t *tail;
    size_t size;
    pthread_cond_t cond;
    pthread_mutex_t lock;
} queue_t;


/**
 * Creates a new heap-allocated FIFO queue. The queue is initially empty.
 *
 * @return a pointer to the new queue
 */
queue_t *queue_init(void){
    queue_t *queue = malloc(sizeof(queue_t));
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

/**
 * Enqueues a value into a queue. There is no maximum capacity,
 * so this should succeed unless the program runs out of memory.
 * This function should be concurrency-safe:
 * multiple threads may call queue_enqueue() or queue_dequeue() simultaneously.
 *
 * @param queue the queue to append to
 * @param value the value to add to the back of the queue
 */
void queue_enqueue(queue_t *queue, void *value){
    pthread_mutex_lock(&queue->lock);
    node_t *new_node = malloc(sizeof(node_t));
    assert(new_node != NULL);
    new_node->value = value;
    new_node->next = NULL;
    if(queue->size == 0){
        queue->head = new_node;
    }
    else{
        queue->tail->next = new_node;  
    }
    queue->tail = new_node;
    queue->size++;
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->lock);
}

/**
 * Dequeues a value from a queue.
 * The value returned is the first enqueued value that was not yet dequeued.
 * If the queue is empty, this thread should block until another thread enqueues a value.
 * This function should be concurrency-safe:
 * multiple threads may call queue_enqueue() or queue_dequeue() simultaneously.
 *
 * @param queue the queue to remove from
 * @return the value at the front of the queue
 */
void *queue_dequeue(queue_t *queue){
    pthread_mutex_lock(&queue->lock);
    if(queue == NULL){return NULL;}
    while(queue->size == 0){
        pthread_cond_wait(&queue->cond, &queue->lock);
    }
    node_t *prev_head = queue->head;
    queue->head = prev_head->next;
    void *value = prev_head->value;
    if(queue->size == 1){
        queue->head = NULL;
        queue->tail = NULL;
    }
    queue->size--;
    free(prev_head);
    pthread_mutex_unlock(&queue->lock);
    return value;
}

/**
 * Frees all resources associated with a heap-allocated queue.
 * You may assume that the queue is already empty.
 *
 * @param queue a queue returned from queue_init()
 */
void queue_free(queue_t *queue){
    free(queue);
}


