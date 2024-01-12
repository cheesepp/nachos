#include "syscall.h"

#define ConsoleInput 0
#define ConsoleOutput 1

int main()
{
    SpaceId newProc1;
    SpaceId newProc2;
    
    newProc1 = Exec("add");
    newProc2 = Exec("add");

    Join(newProc1);
    Join(newProc2);

}
