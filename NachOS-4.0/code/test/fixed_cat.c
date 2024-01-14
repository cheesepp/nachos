#include "syscall.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main() {
    int id;
    char buffer[256];

    id = Open("text.txt", 0);
    Read(buffer,256, id);
    PrintString(buffer);
    Close(id);
    Halt();
}