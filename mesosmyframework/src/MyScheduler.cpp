#include <iostream>
#include <algorithm>
#include <thread>
#include "MyScheduler.h"

using namespace std;
using namespace mesos;


MyScheduler::MyScheduler (const ExecutorInfo& executorInfo, 
                          const Resources& resourcesPerTask, 
                          size_t numTasks) 
    :   executorInfo (executorInfo), 
        resourcesPerTask (resourcesPerTask), 
        numTasks (numTasks),
        nextTaskId (numTasks + 1)
{
    for (size_t i = 1; i <= numTasks; ++i) {
        TaskInfo taskInfo;
        taskInfo.set_name(string("My Framework Task #") + to_string(i));
        taskInfo.mutable_task_id()->set_value(to_string(i));
        //taskInfo.mutable_slave_id()->MergeFrom(offer.slave_id());
        taskInfo.mutable_resources()->MergeFrom(resourcesPerTask);
        
        // Either execute command or executor
        taskInfo.mutable_executor()->MergeFrom(executorInfo);
        //taskQueue.push();
        waitingTasks.push_back(taskInfo);
    }
}

MyScheduler::~MyScheduler() {
    cout << "My Framework: Scheduler Destructor" << endl;
}

void MyScheduler::registered (
	SchedulerDriver* driver,
    const FrameworkID& frameworkId,
    const MasterInfo& masterInfo
)
{
    cout    << "My Framework: Registered with Master ID: " 
            << masterInfo.id()
            << " and Framework "
            << frameworkId.SerializeAsString()
            << endl;
    
    if (numTasks == 0) {
        cout << "My Framework: Stopping Driver as no tasks were assigned" << endl;
        driver->stop();
    }
}

void MyScheduler::reregistered (
	SchedulerDriver* driver,
	const MasterInfo& masterInfo
)
{
    if (numTasks == 0) {
        driver->stop();
        cout << "re-Stopping Driver as no tasks were assigned" << endl;
    }
    cout    << "My Framework: Re-Registered" << endl;
}

void MyScheduler::disconnected (SchedulerDriver* driver) {
    cout    << "My Framework: Disconnected" << endl;
}

void MyScheduler::resourceOffers (
	SchedulerDriver* driver, 
	const vector<Offer>& offers
)
{
    cout << "My Framework: Resource Offers" << endl;
    
    // Call stop here as calling it from statusUpdate is problematic
    if (isFinished()) {
        cout << "My Framework: Stopping Driver, nothing left to schedule!" << endl;
        driver->stop();
        return;
    }
    
    for (const Offer& offer : offers) {
        
        if (waitingTasks.size() == 0) {
            driver->declineOffer(offer.id());
            cout << "My Framework: Declined offer, no tasks" << endl;
            continue;
        }
        
        vector<TaskInfo> taskInfoList;
        Resources offeredResources = offer.resources();
        while (offeredResources.contains(resourcesPerTask) && waitingTasks.size() > 0) {
            waitingTasks.front().mutable_slave_id()->MergeFrom(offer.slave_id());
            taskInfoList.push_back(waitingTasks.front());
            runningTasks.push_back(waitingTasks.front());
            offeredResources -= resourcesPerTask;
            waitingTasks.pop_front();
        }
        
        if (taskInfoList.size() == 0) {
            driver->declineOffer(offer.id());
            cout << "My Framework: Declined offer, not suitable for the task/s" << endl;
        } else {
            cout << "My Framework: Accepting some or all of resources from offer:" << endl;
            for (const Resource& resource : offer.resources()) {
                cout << resource.name() << "-" << resource.scalar().value() << endl;
            }
            cout << "My Framework: Will run task/s on slave ID: " << offer.slave_id().value() << endl;
            /*Status status = */driver->launchTasks(offer.id(), taskInfoList);

            // Either execute command or executor
            //taskInfo.mutable_executor()->MergeFrom(executorInfo);
            //taskInfo.mutable_command()->set_value("sleep 30");
            //cout << "Driver Status: " << status << endl;
        }
        
        
    }
    
}

void MyScheduler::offerRescinded ( 
	SchedulerDriver* driver,
        const OfferID& offerId
)
{
    // are there running tasks which have been launched and the offer was
    // rescinded?
    // We would need to pair offerIds with Tasks, so I don't think
    // this will ever be the case
//    stable_partition(runningTasks.begin(), runningTasks.end(), 
//        [](const TaskInfo& taskInfo) -> bool {
//            return true;
//        }
//    );
    
    
    cout    << "My Framework: Offer Rescinded" << endl;
}

void MyScheduler::statusUpdate (
	SchedulerDriver* driver,
	const TaskStatus& status
) 
{
    cout    << "My Framework: Status Update" << endl;
    cout << "\t" << status.message() << " - " << status.state() 
            << " data: " <<  status.data() << endl;
    
    switch (status.state()) {
        case TASK_STAGING:
            break;
        case TASK_STARTING:
            break;
        case TASK_RUNNING:
            break;
        case TASK_FINISHED:
        {
            vector<TaskInfo>::iterator item = find_if(runningTasks.begin(), runningTasks.end(), 
                [&status](const TaskInfo& taskInfo) -> bool {
                    return taskInfo.task_id() == status.task_id();
                }
            );
            if (item == runningTasks.end()) {
                cout << "STATUPDATE ERROR: TASK NOT FOUND!!!!!" << endl;
            } else {
                // move this item
                finishedTasks.push_back(status);
                runningTasks.erase(item);
            }
        }
        cout << "My Framework: Task Finished ID: " << status.task_id().value() << endl;
        if (isFinished()) {
            
            // Generate Result
            double result = 0;
            for (const TaskStatus& taskStatus : finishedTasks) {
                double res = stod(taskStatus.data());
                result += res;
            }
            result /= numTasks;
            cout << "My Framework: Result is: " << result << endl;
            cout << "Awaiting next offer to stop framework" << endl;
            
            //driver->stop();
            
            // Need to call stop asynchronously as status in UI is inconsistent
            // Calling stop now results in TASK_KILLED in some parts of the 
            // Mesos WEB UI. This must be some bug.
            // Calling stop in resourceOffers method
            
        }
            break;
        case TASK_FAILED:
        case TASK_KILLED:
        case TASK_LOST:
        case TASK_ERROR:
        {
            // Restore to the waiting queue
            vector<TaskInfo>::iterator item = find_if(runningTasks.begin(), runningTasks.end(), 
                [&status](const TaskInfo& taskInfo) -> bool {
                    return taskInfo.task_id() == status.task_id();
                }
            );
            if (item == runningTasks.end()) {
                cout << "My Framework: STATUPDATE ERROR: TASK NOT FOUND!!!!! Cannot ReQueue" << endl;
            } else {
                // move this item
                waitingTasks.push_back(move(*item));
                runningTasks.erase(item);
                cout << "My Framework: Task ERROR (ReQueued) ID: " << status.task_id().value() << endl;
            }
        }
            break;
        default:
            cout << "Unknown Task Status Value" << endl;
            break;
    }
    
}

void MyScheduler::frameworkMessage (
	SchedulerDriver* driver,
	const ExecutorID& executorId,
	const SlaveID& slaveId,
	const string& data
)
{
    cout    << "My Framework: Framework Message" << endl;
}

void MyScheduler::slaveLost (
	SchedulerDriver* driver,
	const SlaveID& slaveId
)
{
    // Do we need to reschedule tasks?
    
    cout    << "My Framework: Slave Lost" << endl;
    
    vector<TaskInfo>::iterator item = partition(runningTasks.begin(), runningTasks.end(), 
        [&slaveId](const TaskInfo& taskInfo) -> bool {
            return taskInfo.slave_id() != slaveId;
        }
    );
    
    if (item != runningTasks.end()) {
        cout << "My Framework: Rescheduling some tasks" << endl;
        // insert into waiting list
//        waitingTasks.insert(waitingTasks.begin(), 
//                make_move_iterator(item), 
//                make_move_iterator(runningTasks.end()));
        copy(make_move_iterator(item), make_move_iterator(runningTasks.end()), waitingTasks.begin());
        runningTasks.erase(item, runningTasks.end());
    }
}

void MyScheduler::executorLost (SchedulerDriver* driver,
	const ExecutorID& executorId,
	const SlaveID& slaveId,
	int status
)
{
    cout    << "My Framework: Executor Lost" << endl;
}

void MyScheduler::error (SchedulerDriver* driver, const string& message) {
    cout    << "My Framework: Error" << endl;
}

bool MyScheduler::isFinished() const {
    return  waitingTasks.size() == 0 && 
            runningTasks.size() == 0 && 
            finishedTasks.size() == numTasks;
}
