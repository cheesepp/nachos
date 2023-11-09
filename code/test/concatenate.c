#include "syscall.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main() {
    int id1, id2, id3;
    char buffer[256],buffer2[256];
    char *Content;
    id1 = Open("text1.txt", 0);
    id2 = Open("text2.txt", 0);
    id3 = Open("text3.txt", 0);
    Read(buffer,256, id1);
    Write(buffer, len(buffer), id3);
    buffer[0]='\n';
    Read(buffer2,256, id2);
    Seek(-1,id3);
    Write(buffer2, len(buffer2), id3);
    Close(id1);
    Close(id2);
    Close(id3);
    Halt();
}
