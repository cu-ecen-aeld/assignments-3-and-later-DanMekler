#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{   

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    struct thread_data* threadfunc_arg = (struct thread_data*) thread_param;

    usleep(threadfunc_arg->wait_to_obtain_ms * 1000);

    if (pthread_mutex_lock(threadfunc_arg->mutex) != 0){
        ERROR_LOG("obtain faild");
        threadfunc_arg->thread_complete_success = false;
        return NULL;
    }

    DEBUG_LOG("mutex obtained");

    usleep(threadfunc_arg->wait_to_release_ms * 1000);

    if (pthread_mutex_unlock(threadfunc_arg->mutex) != 0){
        ERROR_LOG("release faild");
        threadfunc_arg->thread_complete_success = false;
        return NULL;
    }

    DEBUG_LOG("mutex released");

    threadfunc_arg->thread_complete_success = true;

    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    struct thread_data* malloc_data = (struct thread_data*) malloc(sizeof(struct thread_data));

    if (!malloc_data){
        ERROR_LOG("malloc failed");
        return false;
    }

    malloc_data->mutex = mutex;
    malloc_data->wait_to_obtain_ms = wait_to_obtain_ms;
    malloc_data->wait_to_release_ms = wait_to_release_ms;
    malloc_data->thread_complete_success = false;

    if (pthread_create(thread, NULL, threadfunc, (void*) malloc_data) != 0){
        ERROR_LOG("create thread failed");
        free(malloc_data);
        return false;
    }

    DEBUG_LOG("thread created");

    return true;
}

