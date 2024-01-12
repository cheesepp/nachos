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

    // OpenFileId input = ConsoleInput;
    // OpenFileId output = ConsoleOutput;
    // char prompt[2], ch, buffer[60];
    // int i;

    // prompt[0] = '-';
    // prompt[1] = '-';

    // while (1)
    // {
    //     Write(prompt, 2, output);

    //     i = 10;

    //     Read(buffer, 32, input);

    //     if (i > 0)
    //     {
    //         newProc = Exec(buffer);
    //         Join(newProc);
    //     }
    // }

}