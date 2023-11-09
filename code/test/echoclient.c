#include "syscall.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main()
{

    char *Content, *ByeContent;
    int SocketID1, SocketID2, SocketID3, SocketID4;
    int result1, result2, result3, result4;
    Content = "Hello Socket!\n";
    ByeContent = "exit";

    SocketID1 = SocketTCP();
    SocketID2 = SocketTCP();
    SocketID3 = SocketTCP();
    SocketID4 = SocketTCP();

    result1 = Connect(SocketID1, "127.0.0.1", 8081);
    result2 = Connect(SocketID2, "127.0.0.1", 8081);
    result3 = Connect(SocketID3, "127.0.0.1", 8081);
    result4 = Connect(SocketID4, "127.0.0.1", 8081);

    Send(SocketID1, Content, len(Content));
    Receive(SocketID1, Content, len(Content));
    Write(Content, len(Content), CONSOLE_OUTPUT);
    Send(SocketID1, ByeContent, len(Content));
    Close(SocketID1);

    Send(SocketID2, Content, len(Content));
    Receive(SocketID2, Content, len(Content));
    Write(Content, len(Content), CONSOLE_OUTPUT);
    Send(SocketID2, ByeContent, len(Content));
    Close(SocketID2);

    Send(SocketID3, Content, len(Content));
    Receive(SocketID3, Content, len(Content));
    Write(Content, len(Content), CONSOLE_OUTPUT);
    Send(SocketID3, ByeContent, len(Content));
    Close(SocketID3);

    Send(SocketID4, Content, len(Content));
    Receive(SocketID4, Content, len(Content));
    Write(Content, len(Content), CONSOLE_OUTPUT);
    Send(SocketID4, ByeContent, len(Content));
    Close(SocketID4);

    Halt();
}