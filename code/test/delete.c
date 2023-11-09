#include "syscall.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main() {
    int res, id;
    
    res = Remove("text1.txt");
    if (res == -1)
    {
        Write("Delete Failed\n",16,CONSOLE_OUTPUT);
    }
    else
    {
        Write("Success",8,CONSOLE_OUTPUT);
    }
    Halt();
}