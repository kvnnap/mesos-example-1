/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: kevin
 *
 * Created on December 18, 2015, 5:23 PM
 */

#include <cstdlib>
#include <iostream>

#include "MyExecutor.h"



using namespace mesos;
using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    MyExecutor myExecutor;
    MesosExecutorDriver execDriver (&myExecutor);
    Status status = execDriver.run();
    cout << "My Executor Exiting" << endl;
    return 0;
}

