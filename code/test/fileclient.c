#include "syscall.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main()
{
    // doc file 
    // send content file 
    // recieve content file upper
    // write file
    int result, id1, id2, SocketID1;
    char Content[256];
    id1 = Open("text.txt", 0);
    Read(Content, 256, id1);
    SocketID1 = SocketTCP();
    Connect(SocketID1, "127.0.0.1", 8081);
    Send(SocketID1, Content, len(Content));
    Receive(SocketID1, Content, len(Content));
    id2 = Open("text2.txt", 0);
    Write(Content, len(Content), id2);
    Close(SocketID1);
    Close(id1);
    Close(id2);
    Halt();
}


