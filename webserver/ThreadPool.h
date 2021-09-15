/**
 * ThreadPool.h
 *
 * @author Xiaowen Jiang
 *
 * Header file for Task class, which will be exeuted by workers in thread pool.
 *
 */

#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <iostream>
#include <queue>
#include <string>  
#include <pthread.h> 
#include "Task.h"

class ThreadPool {
  protected:
    int workerCount; // number of thread workers
    pthread_t *workers; // array to thread workers
    static queue<Task*> taskQueue; // queue of tasks
    static bool readyToExit; // a bool variable to tell thead ready to exist

    static pthread_mutex_t threadMutex; // pthread mutex
    static pthread_cond_t threadCond; // pthread condition variable

    // start_routine argument for pthread_create
    // see https://www.man7.org/linux/man-pages/man3/pthread_create.3.html
    static void* workerStartRtn(void* data);
    // method to create workers array
    void Create();

  public:
    ThreadPool(int count = 10);
    void AddTask(Task *task); // method to add task to taskQueue
    ~ThreadPool();
};

#endif
