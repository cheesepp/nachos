#ifndef PCB_H
#define PCB_H

#include "synch.h"
#include "thread.h"
#include "addrspace.h"
#include "main.h"
#include "synch.h"
#include "thread.h"
#include "bitmap.h"
#include "sem.h"

// The process control block
//
//-> Store and manage information necessary to execute and join
//* Process's:
//      ->   Process ID
//      ->   Parent ID
//      ->   The executable file containing the program
//      ->   The thread used to execute the program
//      ->   Information for managing joining
//
// Instances of this class are entries in the system-wide process table,
// manage by PTable
// -> Process Control Block

class PCB
{

private:
    //* Semaphore to use for controlling join process
    Sem *joinsemaphore;
    //* Semaphore to use for controlling exit process
    Sem *exitsemaphore;
    //* exclusive access semaphore
    Sem *mutex;
    //* Process ID
    int pid; 

    //* Name of the executable file to load and run
    char *file;
    //* The thread object to be run
    Thread *thread; 

    //* Number of joined processes
    int numwait; // ->  Note: tell us how many process are waiting
                 
    //* the returned value of the process
    int exitCode; 

public:
    int parentID;
    // Constructor
    PCB();
    PCB(int id);
    // Destructor
    ~PCB();

    // Initialize the first PCB
    //
    // This constructor should be used exclusively for the first program
    // executed by Nachos. Because we are now supporting process control
    // blocks for Nachos, so it's obvious that the first process should be the
    // first one to be allocated to the process table.

    // Creates thread and schedules for program stored in `fileName`
    PCB(const char *fileName, Thread *thread);

    //
    // Returns the process ID on success, -1 on failure
    int Exec(char *fileName, int pid);

    int GetID();

    // Returns the name of the file used to run this process.
    const char *GetExecutableFileName();

    // Returns the current Thread object of this process. Use with care.
    const Thread *GetThread();

    int GetNumWait(); //* Returns the number of processes this process is waiting for.

    // Blocks until JoinRelease() is called. This method is called by the
    // parent process that wants to join into this process (i.e. waiting for
    // this process to finish).
    void JoinWait(); //-> The parent process wait for the child process finishes

    // Releases the parent that are waiting for this process.
    void JoinRelease(); //-> The child process notice the parent process
    //-> The child process finishes
    void ExitWait();
    // The parent process accept to exit the child process
    void ExitRelease(); 

    void IncNumWait();

    void DecNumWait();

    void SetExitCode(int exitCode);

    int GetExitCode();
};

#endif // PCB_H