#include "syscall.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main()
{

    char *Content1,*Content2,*Content3,*Content4, *ByeContent;
    int SocketID1, SocketID2, SocketID3, SocketID4;
    int result1, result2, result3, result4;
    Content1 = "Hello Socket1!\n";
    Content2 = "Hello Socket2!\n";
    Content3 = "Hello Socket3!\n";
    Content4 = "Hello Socket4!\n";
    
    ByeContent = "exit";

    SocketID1 = SocketTCP();
    SocketID2 = SocketTCP();
    SocketID3 = SocketTCP();
    SocketID4 = SocketTCP();

    result1 = Connect(SocketID1, "127.0.0.1", 8081);
    result2 = Connect(SocketID2, "127.0.0.1", 8081);
    result3 = Connect(SocketID3, "127.0.0.1", 8081);
    result4 = Connect(SocketID4, "127.0.0.1", 8081);

    Send(SocketID1, Content1, len(Content1));
    Receive(SocketID1, Content1, len(Content1));
    Write(Content1, len(Content1), CONSOLE_OUTPUT);
    Send(SocketID1, ByeContent, len(Content1));
    Close(SocketID1);

    Send(SocketID2, Content2, len(Content2));
    Receive(SocketID2, Content2, len(Content2));
    Write(Content2, len(Content2), CONSOLE_OUTPUT);
    Send(SocketID2, ByeContent, len(Content2));
    Close(SocketID2);

    Send(SocketID3, Content3, len(Content3));
    Receive(SocketID3, Content3, len(Content3));
    Write(Content3, len(Content3), CONSOLE_OUTPUT);
    Send(SocketID3, ByeContent, len(Content3));
    Close(SocketID3);

    Send(SocketID4, Content4, len(Content4));
    Receive(SocketID4, Content4, len(Content4));
    Write(Content4, len(Content4), CONSOLE_OUTPUT);
    Send(SocketID4, ByeContent, len(Content4));
    Close(SocketID4);

    Halt();
}