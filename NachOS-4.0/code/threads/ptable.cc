#include "ptable.h"
#include "synch.h"

PTable::PTable()
{

    totalProcesses = MAX_PROCESS;
    reception = new Bitmap(totalProcesses);
    semaphore = new Semaphore("PTable_bmsem", 1);

    for (int i = 0; i < MAX_PROCESS; ++i)
        blocks[i] = NULL;

    // -> Initialize first process
    // -> (start up process, run automatically by Nachos)
    blocks[0] = new PCB(0);
    blocks[0]->parentID = -1;
    blocks[0]->SetFile("./test/shell");
    DEBUG(dbgSys, "go constructerr");
}

PTable::PTable(int size)
{
    totalProcesses = size;
    reception = new Bitmap(totalProcesses);
    semaphore = new Semaphore("PTable_bmsem", 1);

    for (int i = 0; i < totalProcesses; ++i)
        blocks[i] = NULL;

    // -> Initialize first process
    // -> (start up process, run automatically by Nachos)
    blocks[0] = new PCB(0);
    blocks[0]->parentID = -1;
}

PTable::~PTable()
{
    if (reception)
        delete reception;
    if (semaphore)
        delete semaphore;
    for (int i = 0; i < totalProcesses; ++i)
    {
        if (!blocks[i])
            delete blocks[i];
    }
    DEBUG(dbgSys, "go destructor");
}

int PTable::GetCurrentThreadId()
{
    DEBUG(dbgSys, "thread checking...." << kernel->currentThread);
    DEBUG(dbgSys, "PTable: Current thread name " << kernel->currentThread->getName());
    Thread *current = kernel->currentThread;
    if (current != NULL)
        DEBUG(dbgSys, "pid checking...." << current->pid);
        return current->pid;
    return -1;
}

int PTable::GetFreeSlot()
{
    return reception->FindAndSet();
}

bool PTable::IsExist(int pid)
{
    if (pid < 0 || pid >= MAX_PROCESS)
    {
        return FALSE;
    }

    return blocks[pid] != NULL;
}

void PTable::Remove(int pid)
{
    if (IsExist(pid))
    {
        --totalProcesses;
        reception->Clear(pid);
        if (blocks[pid])
            delete blocks[pid];
        blocks[pid] = NULL;
    }
}

int PTable::ExecuteUpdate(char *fileName)
{
    DEBUG(dbgSys, "go here ptable");
    //Gọi mutex->P(); để giúp tránh tình trạng nạp 2 tiến trình cùng 1 lúc.
    semaphore->P();
    DEBUG(dbgSys, "PTable::ExecUpdate(\"" << fileName << "\")");

    if (fileName == NULL){
        DEBUG(dbgSys, "PTable::Exec : Can't not execute name is NULL.");
        semaphore->V();
        return -1;
    }

    if (strcmp(fileName, "./test/scheduler") == 0 || strcmp(fileName,kernel->currentThread->getName()) == 0){
        DEBUG(dbgSys, "PTable::Exec : Can't not execute itself.");
        semaphore->V();
        return -1;
    }

    int index = this->GetFreeSlot();

    if (index < 0){
        DEBUG(dbgSys, "PTable::Exec : No free slot.");
        semaphore->V();
        return -1;
    }

    //Nếu có slot trống thì khởi tạo một PCB mới với processID chính là index của slot này
    blocks[index] = new PCB(index);
    blocks[index]->SetFile(fileName);

	// parrentID là processID của currentThread
    blocks[index]->parentID = kernel->currentThread->pid;

    // Gọi thực thi phương thức Exec của lớp PCB.
	int pid = blocks[index]->Exec(fileName,index);

    semaphore->V();

    return pid;

    // // Avoid self-execution
    // DEBUG(dbgSys, "PTable: Checking " << fileName << " for self-execution...");
    // int currentThreadId = GetCurrentThreadId();

    // // Add checks here
    // if (currentThreadId < 0 || currentThreadId >= MAX_PROCESS)
    // {
    //     DEBUG(dbgSys, "Invalid thread ID");
    //     semaphore->V();
    //     return -1;
    // }

    // if (blocks[currentThreadId] == NULL)
    // {
    //     DEBUG(dbgSys, "Block at current thread ID is NULL");
    //     semaphore->V();
    //     return -1;
    // }

    // DEBUG(dbgSys, "block " << blocks[currentThreadId]->GetThread());

    // DEBUG(dbgSys, "block(\"" << blocks[currentThreadId]->GetExecutableFileName() << "\")");
    // if (strcmp(blocks[currentThreadId]->GetExecutableFileName(), fileName) == 0)
    // {
    //     DEBUG(dbgSys, "PTable: %s cannot execute itself.");
    //     cerr << "PTable: %s cannot execute itself.\n", fileName;
    //     semaphore->V();
    //     return -1;
    // }

    // // Allocate a new PCB
    // DEBUG(dbgSys, "PTable: Look for free slot in process table...");
    // int slot = GetFreeSlot();
    // if (slot == -1)
    // {
    //     cerr << "PTable: Maximum number of processes reached.\n";
    //     semaphore->V();
    //     return -1;
    // }

    // // PID = slot number
    // this->blocks[slot] = new PCB(slot);
    // this->blocks[slot]->SetFile(fileName);
    // this->blocks[slot]->parentID = currentThreadId;

    // // Schedule the program for execution
    // DEBUG(dbgThread, "PTable: Schedul program for execution...");

    // this->totalProcesses++;
    // this->semaphore->V();

    // // Return the PID of PCB->Exec if success, else return -1
    // return this->blocks[slot]->Exec(fileName, slot);
}

int PTable::JoinUpdate(int id)
{
    int currentThreadId = GetCurrentThreadId();
    if (!IsExist(id))
    {
        cerr << "PTable: Join into an invalid process\n";
        return -1;
    }

    if (id == currentThreadId)
    {
        cerr << "PTable: Process with id " << currentThreadId << " cannot join to itself\n";
        return -2;
    }

    if (blocks[id]->parentID != currentThreadId)
    {
        cerr << "PTable: Can only join parent to child process \n";
        return -3;
    }

    // Increment numwait and call JoinWait() to wait for the child process to complete.
    blocks[currentThreadId]->IncNumWait();
    blocks[id]->JoinWait();

    // After the child process is complete, the process is released.
    blocks[id]->ExitRelease();

    return blocks[id]->GetExitCode();
}

int PTable::ExitUpdate(int exitcode)
{
    int currentThreadId = GetCurrentThreadId();
    if (currentThreadId == 0)
    {
        kernel->currentThread->FreeSpace();
        kernel->interrupt->Halt();
        return 0;
    }
    if (!IsExist(currentThreadId))
    {
        DEBUG(dbgSys, "Process " << currentThreadId << " is not exist.");
        return -1;
    }

    blocks[currentThreadId]->SetExitCode(exitcode);
    blocks[currentThreadId]->JoinRelease();
    blocks[currentThreadId]->ExitWait();
    Remove(currentThreadId);
}

const char *PTable::GetFileName(int id)
{
    return this->blocks[id]->GetExecutableFileName();
}

// void PTable::Print()
// {
//     printf("\n\nTime: %d\n", kernel->stats->totalTicks);
//     printf("Current process table:\n");
//     printf("ID\tParent\tExecutable File\n");
//     int currentThreadId = GetCurrentThreadId(kernel->currentThread);
//     for (int i = 0; i < MAX_PROCESSES; ++i)
//     {
//         if (blocks[i])
//         {
//             printf(
//                 "%d\t%d\t%s%s\n",
//                 blocks[i]->GetID(),
//                 blocks[i]->parentID,
//                 blocks[i]->GetExecutableFileName(),
//                 i == currentThreadId ? " (current thread) " : "");
//         }
//     }
// }