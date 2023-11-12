#include "syscall.h"
#include "copyright.h"
#define maxlen 32
int main()
{
    int len;
    char filename[maxlen + 1];
    /*Create a file*/
    if (Create("text8.txt") == -1)
    {
        PrintString("Tao file that bai!\n\n");
    }
    else
    {
        PrintString("Tao file thanh cong!\n\n");
       
    }
    Halt();
}