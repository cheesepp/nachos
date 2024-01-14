// #include "syscall.h"

// int main()
// {
// 	int i;	
// 	for(i =0; i< 1000; i++)
// 	{
// 		PrintChar('B');
// 	}
// }
#include "syscall.h"

int main()
{
    int i;

    for(i = 0; i < 1000; i++) {
	Wait("../test/player_b");
	PrintChar('B');
	Signal("../test/player_a");
    }
}