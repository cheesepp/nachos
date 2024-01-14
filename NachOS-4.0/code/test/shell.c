#include "syscall.h"

#define ConsoleInput 0
#define ConsoleOutput 1

// typedef int SpaceId;

int main()
{
    SpaceId newProc1;
    SpaceId newProc2;
    
    newProc1 = Exec("../test/player_a");
    newProc2 = Exec("../test/player_b");

    Join(newProc1);
    Join(newProc2);

}
