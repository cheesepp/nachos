// #include "syscall.h"

// int main()
// {
	
// 	int i;
// 	for(i =0; i< 1000; i++)
// 	{
// 		PrintChar('A');
// 	}
	
// }
#include "syscall.h"

int main()
{
    int i;

    if(CreateSemaphore("../test/player_a", 1) == -1) Exit(-1);
    if(CreateSemaphore("../test/player_b", 0) == -1) Exit(-1);
    Exec("../test/pong");
    for(i = 0; i < 1000; i++) {
        Wait("player_a");
        PrintChar('A');
        Signal("player_b");
    }
}