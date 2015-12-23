/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MyExecutor.h
 * Author: kevin
 *
 * Created on December 18, 2015, 5:54 PM
 */

#ifndef MYEXECUTOR_H
#define MYEXECUTOR_H

#include <mesos/executor.hpp>

class MyExecutor 
    : public mesos::Executor
{
public:
    
    virtual ~MyExecutor() override;

    
    // Invoked once the executor driver has been able to successfully
    // connect with Mesos. In particular, a scheduler can pass some
    // data to its executors through the FrameworkInfo.ExecutorInfo's
    // data field.
    virtual void registered(
        mesos::ExecutorDriver* driver,
        const mesos::ExecutorInfo& executorInfo,
        const mesos::FrameworkInfo& frameworkInfo,
        const mesos::SlaveInfo& slaveInfo) override;

    // Invoked when the executor re-registers with a restarted slave.
    virtual void reregistered(
        mesos::ExecutorDriver* driver,
        const mesos::SlaveInfo& slaveInfo) override;

    // Invoked when the executor becomes "disconnected" from the slave
    // (e.g., the slave is being restarted due to an upgrade).
    virtual void disconnected(mesos::ExecutorDriver* driver) override;

    // Invoked when a task has been launched on this executor (initiated
    // via Scheduler::launchTasks). Note that this task can be realized
    // with a thread, a process, or some simple computation, however, no
    // other callbacks will be invoked on this executor until this
    // callback has returned.
    virtual void launchTask(
        mesos::ExecutorDriver* driver,
        const mesos::TaskInfo& task) override;

    // Invoked when a task running within this executor has been killed
    // (via SchedulerDriver::killTask). Note that no status update will
    // be sent on behalf of the executor, the executor is responsible
    // for creating a new TaskStatus (i.e., with TASK_KILLED) and
    // invoking ExecutorDriver::sendStatusUpdate.
    virtual void killTask(
        mesos::ExecutorDriver* driver,
        const mesos::TaskID& taskId) override;

    // Invoked when a framework message has arrived for this executor.
    // These messages are best effort; do not expect a framework message
    // to be retransmitted in any reliable fashion.
    virtual void frameworkMessage(
        mesos::ExecutorDriver* driver,
        const std::string& data) override;

    // Invoked when the executor should terminate all of its currently
    // running tasks. Note that after a Mesos has determined that an
    // executor has terminated any tasks that the executor did not send
    // terminal status updates for (e.g., TASK_KILLED, TASK_FINISHED,
    // TASK_FAILED, etc) a TASK_LOST status update will be created.
    virtual void shutdown(mesos::ExecutorDriver* driver) override;

    // Invoked when a fatal error has occured with the executor and/or
    // executor driver. The driver will be aborted BEFORE invoking this
    // callback.
    virtual void error(
        mesos::ExecutorDriver* driver,
        const std::string& message) override;
private:

};

#endif /* MYEXECUTOR_H */

