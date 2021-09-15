/**
 * Task.h
 *
 * @author Xiaowen Jiang
 *
 * Header file for Task class, which will be exeuted by workers in thread pool.
 *
 */

#ifndef __TASK_H
#define __TASK_H

#include <string.h>

using namespace std;  

class Task {
protected:
    string taskName;
 
public:
    Task() {};
    Task(string &name): taskName(name) {}
    virtual int Run() = 0; // virtual function which will be executed by worker threads.
    virtual ~Task() {};
};

#endif