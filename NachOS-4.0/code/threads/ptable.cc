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
    // thực hiện thao tác chờ
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
    
    // set file name cho block (mảng pcb)
    blocks[index]->SetFile(fileName);

	// parrentID là processID của currentThread
    blocks[index]->parentID = kernel->currentThread->pid;

    // Gọi thực thi phương thức Exec của lớp PCB.
    // tạo 1 thread mới có file name và process id 
	int pid = blocks[index]->Exec(fileName,index);

    // thực hiển giải phóng semaphore
    semaphore->V();

    return pid;

}

int PTable::JoinUpdate(int id)
{
    DEBUG(dbgSys, "PTable::joinupdate: go here");

	// có processID là id hay không. Nếu không thỏa, ta báo lỗi hợp lý và trả về -1.
	if(id < 0)
	{
        DEBUG(dbgSys, "PTable::joinupdate: id="<<id);
		return -1;
	}

	// Check if process running is parent process of process which joins
    DEBUG(dbgSys, "PTable::joinupdate: check procces id of currentThread = "<< kernel->currentThread->pid);
    DEBUG(dbgSys, "PTable::joinupdate: check id = "<< id);
    DEBUG(dbgSys, "PTable::joinupdate: check parrent id of block = "<< blocks[id]->parentID);

    // Ta kiểm tra tính hợp lệ của processID id và kiểm tra tiến trình gọi Join có phải là cha của tiến trình
	if(kernel->currentThread->pid != blocks[id]->parentID)
	{
        DEBUG(dbgSys, "PTable::joinupdate: cant join in process which is not its parent process"<<id);
		return -1;
	}

    // tăng số tiến trình chờ bằng IncNumWait
	blocks[blocks[id]->parentID]->IncNumWait();
	

	//pcb[id]->boolBG = 1;
    
    // tiến trình cha đợi tiến trình con kết thúc
	blocks[id]->JoinWait();

	// Xử lý exitcode.	
	int ec = blocks[id]->GetExitCode();

    // cho phép tiến trình con kết thúc
	blocks[id]->ExitRelease();

    // Successfully
	return ec;
}

int PTable::ExitUpdate(int exitcode)
{
    // lấy process id của tiến trình gọi
    int id = kernel->currentThread->pid;

    // nếu tiến trình gọi làm main process thì halt()
    if (id == 0){
        kernel->currentThread->FreeSpace();
        kernel->interrupt->Halt();
        return 0;
    }
    if (IsExist(id) == false){
        return -1;
    }
    
    // nếu không phải tiến trình main process gọi
    // setexitcode để đặt exitcode cho tiến trình gọi
    blocks[id]->SetExitCode(exitcode);
    blocks[blocks[id]->parentID]->DecNumWait();

    // gọi join release để giải phóng tiến trình cha đang đợi
    blocks[id]->JoinRelease();
    // gọi exitwait để xin tiến trình cha cho phép thoát
    blocks[id]->ExitWait();

    Remove(id);
    return exitcode;
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