/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MyExecutor.cpp
 * Author: kevin
 * 
 * Created on December 18, 2015, 5:54 PM
 */

#include <iostream>
#include <thread>
#include <chrono>
#include "MyExecutor.h"

#include <random>

using namespace std;
using namespace mesos;

MyExecutor::~MyExecutor() {

}

void MyExecutor::registered(
    ExecutorDriver* driver, 
    const ExecutorInfo& executorInfo, 
    const FrameworkInfo& frameworkInfo, 
    const SlaveInfo& slaveInfo) 
{
    cout << "My Executor Registered: "
            << executorInfo.name()
            << " With Framework ID: "
            << frameworkInfo.id().value()
            << " on Slave ID: "
            << slaveInfo.id().value() << endl;
}

void MyExecutor::reregistered(ExecutorDriver* driver, const SlaveInfo& slaveInfo) {
    cout << "My Executor re-registered" << endl;
}

void MyExecutor::disconnected(ExecutorDriver* driver) {
    cout << "My Executor Disconnected" << endl;
}

void MyExecutor::launchTask(ExecutorDriver* driver, const TaskInfo& task) {
    cout << "My Executor Launching Task ID: " << task.task_id().value() << endl;
    
    TaskStatus status;
    status.mutable_task_id()->MergeFrom(task.task_id());
    status.set_state(TaskState::TASK_STARTING);
    driver->sendStatusUpdate(status);
    
    thread myThread ([driver, task]() -> void {
        TaskStatus status;
        status.mutable_task_id()->MergeFrom(task.task_id());
        cout << "TASK ID: " << task.task_id().value() << endl;
        //status.set_state(TaskState::TASK_STARTING);
        //driver->sendStatusUpdate(status);
        
        // Do Work
        mt19937 gen ( default_random_engine(chrono::system_clock::now().time_since_epoch().count())() );
        uniform_real_distribution<float> fNum (0.f, 1.f);
        
        size_t inside = 0, outside = 0;
        
        status.set_state(TaskState::TASK_RUNNING);
        driver->sendStatusUpdate(status);
        
        cout << "Running!!" << endl;
        
        for (size_t n = 0; n < 3000000000; ++n) {
            float x = fNum(gen);
            float y = fNum(gen);
            if (x*x + y*y <= 1.f) {
                // Inside Circle
                ++inside;
            } else {
                ++outside;
            }
        }
        
        double result = static_cast<double>(inside) / (inside + outside);
        result *= 4;
        
        cout << "FINISHH!!: Sending Data: " << to_string(result) << " Actual: " << result << endl;
        
        //this_thread::sleep_for(chrono::seconds(10));
        
        // End
        status.set_state(TaskState::TASK_FINISHED);
        *status.mutable_data() = to_string(result);
        cout << "My Executor Task Ready" << endl;
        driver->sendStatusUpdate(status);
    });
    myThread.detach();
}

void MyExecutor::killTask(ExecutorDriver* driver, const TaskID& taskId) {
    cout << "My Executor Killing Task ID: " << taskId.value() << endl;
//    TaskStatus status;
//    status.mutable_task_id()->MergeFrom(taskId);
//    status.set_state(TaskState::TASK_KILLED);
//    driver->sendStatusUpdate(status);
}

void MyExecutor::frameworkMessage(ExecutorDriver* driver, const string& data) {
    cout << "My Executor Framework Message: " << data << endl;
}

void MyExecutor::shutdown(ExecutorDriver* driver) {
    cout << "My Executor Shutdown" << endl;
}

void MyExecutor::error(ExecutorDriver* driver, const string& message) {
    cout << "My Executor Error: " << message << endl;
}

