#include "syscall.h"

int main()
{
    char fileName[] = "text.txt";
    int length, id;
    int i;

    id = Open(fileName, 1);
    if (id != -1)
    {
        // PrintString("File ");
        // PrintString(fileName);
        // PrintString(" opened successfully!\n");
        // PrintString("Id: ");
        // // PrintNum(id);
        // // PrintString("\n");

    }
    else
        PrintString("Open file failed\n");
    Close(id);

    Halt();
}