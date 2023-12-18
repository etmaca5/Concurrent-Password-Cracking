#include "thread_pool.h"
#include "queue.h"
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

/**
 * A pool of threads which perform work in parallel.
 * The pool contains a fixed number of threads specified in thread_pool_init()
 * and a shared queue of work for the worker threads to run.
 * Each worker thread dequeues new work from the queue when its current work is finished.
 */
typedef struct thread_pool{
    queue_t *queue;
    size_t num_workers;
    pthread_t *workers;
} thread_pool_t;


typedef struct{
    work_function_t function;
    void *aux;
} work_t;

// assigns tasks for a thread through the queue until a NULL task is reached
void *thread_tasks(void *curr_queue){
    queue_t *queue = (queue_t *)curr_queue;
    while(true){
        work_t *curr_task = (work_t *) queue_dequeue(queue);
        if(curr_task == NULL){return NULL;}
        (curr_task->function)(curr_task->aux);
        free(curr_task);
    }
}

/**
 * Creates a new heap-allocated thread pool with the given number of worker threads.
 * All worker threads should start immediately so they can perform work
 * as soon as thread_pool_add_work() is called.
 *
 * @param num_worker_threads the number of threads in the pool
 * @return a pointer to the new thread pool
 */
thread_pool_t *thread_pool_init(size_t num_worker_threads){
    thread_pool_t *pool = malloc(sizeof(thread_pool_t));
    pool->queue = queue_init();
    pool->num_workers = num_worker_threads;
    pthread_t *workers = malloc(sizeof(pthread_t) * num_worker_threads);
    pool->workers = workers;
    for(size_t i = 0; i < num_worker_threads; i++){
        pthread_create(&workers[i], NULL, thread_tasks, pool->queue);
    }
    return pool;
}

/**
 * Adds work to a thread pool.
 * The work will be performed by a worker thread as soon as all previous work is finished.
 *
 * @param pool the thread pool to perform the work
 * @param function the function to call on a thread in the thread pool
 * @param aux the argument to call the work function with
 */
void thread_pool_add_work(thread_pool_t *pool, work_function_t function, void *aux){
    work_t *work = malloc(sizeof(work_t));
    work->function = function;
    work->aux = aux;
    queue_enqueue(pool->queue, (void *)work);
}

/**
 * Waits for all work added to a thread pool to finish,
 * then frees all resources associated with a heap-allocated thread pool.
 * A special value (e.g. NULL) can be put in the work queue to mark the end of the work.
 * thread_pool_add_work() cannot be used on this pool once this function is called.
 *
 * @param pool the thread pool to close
 */
void thread_pool_finish(thread_pool_t *pool){
    for(size_t i = 0; i < pool->num_workers; i++){
        queue_enqueue(pool->queue, NULL);
    }
    for (size_t j = 0; j < pool->num_workers; j++) {
        pthread_join(pool->workers[j], NULL);
    }
    free(pool->queue);
    free(pool->workers);
    free(pool);
}
