#include <getopt.h>
#include <signal.h>
#include <atomic>
#include <cstdlib>
#include <iostream>

#include "MyScheduler.h"

using namespace std;
using namespace mesos;

// Use unnamed namespaces in C++ instead
// of static specifier as used in C
namespace 
{
    //volatile sig_atomic_t interrupted = false;
    atomic<bool> interrupted (false);
    MesosSchedulerDriver * msd;
    void SigIntHandler (int signum) {
            if (interrupted) {
                    return;
            }
            interrupted = true;
            msd->stop();
    }

    void registerSigInt(void sigIntHandler(int)) 
    {
            struct sigaction sa {}; // zero-initialize
            sa.sa_handler = sigIntHandler;
            sigfillset(&sa.sa_mask);
            sigaction(SIGINT, &sa, nullptr);
    }
    
    const char * const defaultMaster = "127.0.1.1:5050";
    const char * const defaultExecutorUri = "http://kevinnapoli.com/mesos/mesosmyexecutor";
    const int defaultNumTasks = 1;
    const int defaultCpusPerTask = 1;
    const int defaultMemSizePerTask = 32;
}

int main (int argc, char ** argv) {

    string master = defaultMaster;
    string executorUri = defaultExecutorUri;
    int numTasks = defaultNumTasks;
    int numCpuPerTask = defaultCpusPerTask;
    int memSizePerTask = defaultMemSizePerTask;
    bool help = false;
    
    // Handle arguments
    
    // name, hasArg, flag, val
    option options[] = {
        {"master", required_argument, nullptr, 'm'},
        {"tasks",  required_argument, nullptr, 't'},
        {"exuri",  required_argument, nullptr, 'e'},
        {"cpus",   required_argument, nullptr, 'c'},
        {"mem",    required_argument, nullptr, 'r'},
        {"help",   no_argument,       nullptr, 'h'}
    };
    
    int c;
    int opIndex;
    while ((c = getopt_long_only(argc, argv, "", options, &opIndex)) != -1) {
        switch (c) {
            case 'm':
                master = optarg;
                break;
            case 't':
                numTasks = atoi(optarg);
                break;
            case 'e':
                executorUri = optarg;
                break;
            case 'h':
                help = true;
                break;
            case 'c':
                numCpuPerTask = atoi(optarg);
                break;
            case 'r':
                memSizePerTask = atoi(optarg);
                break;
            case ':':
            case '?':
                help = true;
                cout << "Unknown option or missing argument: " 
                        << (char)c << " : " << optarg << endl;
            default:
                help = true;
                cout << "Unknown Error while Parsing Arguments- " << 
                        (char)c << " : " << optarg << endl;
        }
    }

    if (help || 
        master.length() == 0 || 
        numTasks < 1 || 
        executorUri.length() == 0 ||
        numCpuPerTask < 1 ||
        memSizePerTask < 16
        ) 
    {
        cout << "Usage:" << endl 
             << "\t--master=[ip:port], default: " << defaultMaster << endl
             << "\t--tasks=[num],      default: " << defaultNumTasks << endl
             << "\t--exuri=[URI],      default: " << defaultExecutorUri << endl
             << "\t--cpus=[num],       default: " << defaultCpusPerTask << endl
             << "\t--mem=[num],        default: " << defaultMemSizePerTask << endl
             << "\t--help (Shows this Usage Information)" << endl;
        return EXIT_SUCCESS;
    }
    
    cout << "Will use:" << endl 
         << "\tMaster:    " << master << endl 
         << "\tNum Tasks: " << numTasks << endl
         << "\tExctr URI: " << executorUri << endl
         << "\tNum Cpus per Task: " << numCpuPerTask << endl
         << "\tMemory size per Task: " << memSizePerTask << "MB" << endl;
    
    // Describe My Framework
    FrameworkInfo frameworkInfo;
    frameworkInfo.set_user("");
    frameworkInfo.set_name("My Mesos Framework");
    frameworkInfo.set_principal("myframework");
    
    // Describe Executor
    ExecutorInfo execInfo;
    {
        CommandInfo_URI cUri; 
        cUri.set_cache(false);
        cUri.set_executable(true);
        cUri.set_extract(false);
        cUri.set_value(executorUri);

        Environment_Variable envVar;
        envVar.set_name("LD_LIBRARY_PATH");
        envVar.set_value("/home/kevin/Documents/mesos/mesos-0.25.0/kevinbuild/lib");

        Environment env;
        env.add_variables()->MergeFrom(envVar);

        CommandInfo cmdInfo;
        // && /home/kevin/NetBeansProjects/MesosMyExecutor/dist/Debug/GNU-Linux/mesosmyexecutor
        cmdInfo.add_uris()->MergeFrom(cUri);
        cmdInfo.mutable_environment()->MergeFrom(env);
        cmdInfo.set_value("./mesosmyexecutor");


        execInfo.mutable_executor_id()->set_value("mesos-my-executor");
        execInfo.mutable_command()->MergeFrom(cmdInfo);
        execInfo.set_name("My Executor");
    }
    
    // Describe Resources per Task
    
    Resources resourcesPerTask = Resources::parse(
            "cpus:" + to_string(numCpuPerTask) +
            ";mem:" + to_string(memSizePerTask)
            ).get(); 
    //resourcesPerTask.cpus().get() = (double)numCpuPerTask;
    //resourcesPerTask.mem().get() = memSizePerTask * 1024 * 1024;
    
    // Instantiate our custom scheduler containing the information
    // about the executor who is supposed to execute the task
    MyScheduler myScheduler (execInfo, resourcesPerTask, static_cast<size_t>(numTasks));

    MesosSchedulerDriver msd (&myScheduler, frameworkInfo, master);
    ::msd = &msd;
    registerSigInt(SigIntHandler);
    
    // Block here and make our scheduler available
    Status status = msd.run();

    if (interrupted) {
        cout << "Have been interrupted - Shutting Down" << endl;
    }

    cout << "My Framework is exiting" << endl;
    
    //Status_IsValid(status) == true
    return status == DRIVER_STOPPED ? EXIT_SUCCESS : EXIT_FAILURE;
}