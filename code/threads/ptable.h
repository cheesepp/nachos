#ifndef PTABLE_H
#define PTABLE_H

#include "main.h"
#include "pcb.h"
#include "synch.h"
#include "thread.h"
#include "bitmap.h"

#include <stdio.h>

#define MAX_PROCESS 10

class Semaphore;
class PCB;

// Process Table
class PTable {

private:
    // The array of system-wide PCBs to manage
    PCB *blocks[MAX_PROCESS];
    // Number of alive processes in the system
    int totalProcesses;
    // Manages free slots in the PCBs table
    Bitmap *reception;
    // Semaphore
    Semaphore *semaphore;
    
public:
    // Initializes process table with `size` entries
    PTable(int size);
    PTable();

    ~PTable();

    // Allocates the start up process (first executed by Nachos)
    //void InitializeFirstProcess(const char* fileName, Thread* thread);

    int GetCurrentThreadId();

    // Sets up PCB and schedules the program stored in `fileName`.
    // Returns the process id on success, -1 otherwise
    int ExecuteUpdate(char *fileName);

    // Updates the process table when the current thread join into the thread
    // `id`
    int JoinUpdate(int id);

    // Updates the process table when the current thread exit with exit code
    int ExitUpdate(int exitcode);

    // Return an index to a free slot in the table, returns -1
    // if the table is full
    int GetFreeSlot();

    // Returns TRUE if pid exists
    bool IsExist(int pid);

    // Removes a block that has process id = 'pid'
    void Remove(int pid);

    const char* GetFileName(int pid);
};

#endif // PTable_H