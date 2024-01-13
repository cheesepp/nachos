#include "addrspace.h"
#include "main.h"
#include "synch.h"
#include "thread.h"

#include <stdio.h>
#include "pcb.h"

// Entry point for executing processes
static void StartProcess(void *args)
{
    int id;
    id = *((int *)args);
    // Get the fileName of this process id
    char *fileName = strcpy(fileName, kernel->pTable->GetFileName(id));

    AddrSpace *addrspace;
    addrspace->Load(fileName);

    if (addrspace == NULL)
    {
        cerr << "PCB::Exec() : Can't create AddSpace.\n";
        return;
    }

    addrspace->Execute(); // kernel->currentThread->space = space;
                          // space->InitRegisters();	// set the initial register values
                          // space->RestoreState();	// load page table register
                          // kernel->machine->Run();	// jump to the user progam

    ASSERT(FALSE); // machine->Run never returns;
                   // the address space exits
                   // by doing the syscall "exit"
}

PCB::PCB()
{
    this->pid = -1;
    this->parentID = -1;
    this->file = NULL;
    this->numwait = 0;
    this->exitCode = 0;
    this->thread = NULL;
    this->joinsemaphore = new Semaphore("joinsemaphore", 0);
    this->exitsemaphore = new Semaphore("exitsemaphore", 0);
    this->mutex = new Semaphore("mutex", 1);
}

PCB::PCB(const char *fileName, Thread *thread) : PCB()
{
    this->pid = 0;
    strcpy(this->file, fileName);
    this->thread = thread;
}

PCB::PCB(int id)
{
    this->pid = kernel->currentThread->pid;
    this->joinsemaphore = new Semaphore("joinsem", 0);
    this->exitsemaphore = new Semaphore("exitsem", 0);
    this->mutex = new Semaphore("multex", 1);
}

PCB::~PCB()
{
    if (this->file)
        delete this->file;
    if (this->thread)
    {
        this->thread->Finish();
        delete this->thread;
    }
    if (this->joinsemaphore)
        delete this->joinsemaphore;
    if (this->exitsemaphore)
        delete this->exitsemaphore;
    if (this->mutex)
        delete this->mutex;
}

int PCB::GetID()
{
    return this->pid;
}

const char *PCB::GetExecutableFileName() // -> Lay file name
{
    return this->file;
}

const Thread *PCB::GetThread()
{
    return this->thread;
}

int PCB::GetNumWait()
{
    return this->numwait;
}

int PCB::GetExitCode()
{
    return this->exitCode;
}

void PCB::SetExitCode(int exitCode)
{
    this->exitCode = exitCode;
}

void PCB::IncNumWait()
{
    this->mutex->P();
    this->numwait++;
    this->mutex->V();
}

void PCB::DecNumWait()
{
    this->mutex->P();
    this->numwait--;
    this->mutex->V();
}

int PCB::Exec(char *fileName, int pid)
{
    this->mutex->P();
    DEBUG(dbgThread, "PCB: Setting things up for " << fileName << "...");

    this->pid = pid;

    // Copy the executable file name into local storage, since `fileName`
    // is going to be reused elsewhere
    this->file = new char[strlen(fileName)];
    strcpy(this->file, fileName);

    DEBUG(dbgThread, "PCB: Forking " << this->file << "...");
    this->thread = new Thread(this->file);
    this->thread->Fork(StartProcess, file);
    this->mutex->V();
    return this->pid;
}

void PCB::JoinWait()
{
    this->joinsemaphore->P();
}

void PCB::ExitWait()
{
    this->exitsemaphore->P();
}

void PCB::JoinRelease()
{
    this->joinsemaphore->V();
}

void PCB::ExitRelease()
{
    this->exitsemaphore->V();
}