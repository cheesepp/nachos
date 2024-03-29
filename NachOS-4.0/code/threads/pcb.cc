#include "addrspace.h"
#include "main.h"
#include "synch.h"
#include "thread.h"

#include <stdio.h>
#include "pcb.h"
#include "debug.h"
// Entry point for executing processes
void StartProcess_2(int id){
    const char* fileName = kernel->pTable->GetFileName(id);

    AddrSpace *space;
    space = new AddrSpace((char*)fileName);

    if (space == NULL){
        DEBUG(dbgSys, "PCB::Exec: cant create AddSpace");
        return;
    }

    kernel->currentThread->space = space;

    space->InitRegisters();
    space->RestoreState();

    kernel->machine->Run();
    ASSERT(FALSE);


}

static void StartProcess(void *args)
{
    int id;
    id = *((int *)args);
    // Get the fileName of this process id
	DEBUG(dbgSys, "1 Start process:");
    char *fileName = new char[20];
    DEBUG(dbgSys, "2 Start process:");
    strcpy(fileName, kernel->pTable->GetFileName(id));
	DEBUG(dbgSys, "3 Start process:" << fileName);


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
    DEBUG(dbgSys, "init pcb");
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
    DEBUG(dbgSys, "init pcb -file,thread");
    this->pid = 0;
    strcpy(this->file, fileName);
    this->thread = thread;
}

PCB::PCB(int id)
{
    DEBUG(dbgSys, "init pcb -id");
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
    DEBUG(dbgSys, "check file" << this->parentID);
    if (this->file == NULL) {
        DEBUG(dbgSys, "File is NULL");
        return NULL;
    }
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

void PCB::SetFile(char* filename)
{
    this->file = filename;
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
    DEBUG(dbgSys, "PCB: Exec " << fileName << "...");
    // Gọi mutex->P(); để giúp tránh tình trạng nạp 2 tiến trình cùng 1 lúc.
    mutex->P();

    // Kiểm tra thread đã khởi tạo thành công chưa, nếu chưa thì báo lỗi là không đủ bộ nhớ, gọi mutex->V() và return.             
    this->thread = new Thread(fileName);

    if (this->thread == NULL){
        DEBUG(dbgSys, "PCB::Exec: not enough mem  ");
        mutex->V();
        return -1;
    }
	//  Đặt processID của thread này là pid.
    this->thread->pid = pid;

    this->parentID = kernel->currentThread->pid;

    this->thread->Fork((VoidFunctionPtr) StartProcess_2,(void *)pid);
    mutex->V();

    return pid;


    // // Copy the executable file name into local storage, since `fileName`
    // // is going to be reused elsewhere
    // this->file = new char[strlen(fileName)];
    // strcpy(this->file, fileName);

    // DEBUG(dbgSys, "PCB: Forking " << this->file << "...");
    // this->thread = new Thread(this->file);
    // this->thread->Fork(StartProcess, file);
    // this->mutex->V();
    // return this->pid;
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