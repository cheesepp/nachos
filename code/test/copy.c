#include "syscall.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main() {
    int id1, id2;
    char buffer[256];
    char *Content;

    id1 = Open("text.txt", 0);
    id2 = Open("text1.txt", 0);
    Read(buffer,256, id1);
    Write(buffer, len(buffer), id2);
    Close(id1);
    Close(id2);
    Halt();
}