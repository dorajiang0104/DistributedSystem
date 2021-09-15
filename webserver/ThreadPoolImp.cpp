/**
 * ThreadPool.cpp
 *
 * @author Xiaowen Jiang
 *
 * Implementation file for the Thread Pool
 *
 */

#include "ThreadPool.h"

using namespace std;

#define PTHREAD_MUTEX_INITIALIZER {_PTHREAD_MUTEX_SIG_init, {0}}
#define _PTHREAD_MUTEX_SIG_init 0x32AAABA7

// initialize
queue<Task*> ThreadPool::taskQueue;
bool ThreadPool::readyToExit = false;
pthread_mutex_t ThreadPool::threadMutex;
pthread_cond_t ThreadPool::threadCond;

ThreadPool::ThreadPool(int count) {
  this->workerCount = count;
  pthread_mutex_init(&threadMutex, NULL);
  pthread_cond_init(&threadCond,NULL);
  Create();
}

void* ThreadPool::workerStartRtn(void* data) {
  pthread_t workId = pthread_self();
  while(1) {
    pthread_mutex_lock(&threadMutex);
    
    // when no task in queue, start waiting.
    if (taskQueue.size() == 0) {
      pthread_cond_wait(&threadCond, &threadMutex);
    }

    // get notification that needs to exit
    if (readyToExit) {
      // unlock before exiting
      pthread_mutex_unlock(&threadMutex);
      pthread_exit(NULL);
    }

    // grab task
    if (taskQueue.size() != 0) {
      Task* task = taskQueue.front();
      taskQueue.pop();
      pthread_mutex_unlock(&threadMutex);
      task->Run();
    } else {
      // if not grab task failed unlock the mutex so
      // other threads can have the write access
      pthread_mutex_unlock(&threadMutex);
    }
  }

  return (void*) 0;
}


void ThreadPool::Create() {  
  this->workers = (pthread_t*)malloc(sizeof(pthread_t) * this->workerCount);
  for (int i = 0; i < this->workerCount; i++) {  
    pthread_create(&(this->workers[i]), NULL, workerStartRtn, NULL);  
  }  
  return;
}

void ThreadPool::AddTask(Task *task) {  
  pthread_mutex_lock(&threadMutex); 
  taskQueue.push(task);
  pthread_mutex_unlock(&threadMutex); 
  pthread_cond_signal(&threadCond);
  return;  
}

ThreadPool::~ThreadPool() {
  if (!readyToExit) {
    readyToExit = true;
    // clear task queue
    queue<Task*> e;
    swap(e, taskQueue);
    // awake all worker threads
    pthread_cond_broadcast(&threadCond);

    // wait for all threads exist
    for (int i = 0; i < this->workerCount; i++) {  
      pthread_join(this->workers[i], NULL);    
    }

    // free workers array
    free(this->workers);  
    this->workers = NULL;

    // destory lock & condition variable
    pthread_mutex_destroy(&threadMutex);  
    pthread_cond_destroy(&threadCond);
  }
}









