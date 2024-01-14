#include "syscall.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main()
{
    char *Content1, *Content2, *Content3, *Content4, *ByeContent;
    int SocketID1, SocketID2, SocketID3, SocketID4;
    int result1, result2, result3, result4;
    int closeResult1, closeResult2, closeResult3, closeResult4;
    Content1 = "Hello Socket1!\n";
    Content2 = "Hello Socket2!\n";
    Content3 = "Hello Socket3!\n";
    Content4 = "Hello Socket4!\n";
    ByeContent = "exit";
    // Create sockets
    SocketID1 = SocketTCP();
    if (SocketID1 == -1)
    {
        Write("Error creating SocketID1\n", len("Error creating SocketID1\n"), CONSOLE_OUTPUT);
        Halt();
    }

    SocketID2 = SocketTCP();
    if (SocketID2 == -1)
    {
        Write("Error creating SocketID2\n", len("Error creating SocketID2\n"), CONSOLE_OUTPUT);
        Halt();
    }

    SocketID3 = SocketTCP();
    if (SocketID3 == -1)
    {
        Write("Error creating SocketID3\n", len("Error creating SocketID3\n"), CONSOLE_OUTPUT);
        Halt();
    }

    SocketID4 = SocketTCP();
    if (SocketID4 == -1)
    {
        Write("Error creating SocketID4\n", len("Error creating SocketID4\n"), CONSOLE_OUTPUT);
        Halt();
    }

    // Connect sockets
    result1 = Connect(SocketID1, "127.0.0.1", 8081);
    if (result1 == -1)
    {
        Write("Error connecting SocketID1\n", len("Error connecting SocketID1\n"), CONSOLE_OUTPUT);
    }

    result2 = Connect(SocketID2, "127.0.0.1", 8081);
    if (result2 == -1)
    {
        Write("Error connecting SocketID2\n", len("Error connecting SocketID2\n"), CONSOLE_OUTPUT);
    }

    result3 = Connect(SocketID3, "127.0.0.1", 8081);
    if (result3 == -1)
    {
        Write("Error connecting SocketID3\n", len("Error connecting SocketID3\n"), CONSOLE_OUTPUT);
    }

    result4 = Connect(SocketID4, "127.0.0.1", 8081);
    if (result4 == -1)
    {
        Write("Error connecting SocketID4\n", len("Error connecting SocketID4\n"), CONSOLE_OUTPUT);
    }

    Send(SocketID1, Content1, len(Content1));
    Receive(SocketID1, Content1, len(Content1));
    Write(Content1, len(Content1), CONSOLE_OUTPUT);
    Send(SocketID1, ByeContent, len(ByeContent));
    // closeResult1 = Close(SocketID1);
    if (closeResult1 == -1)
    {
        Write("Error closing SocketID1\n", len("Error closing SocketID1\n"), CONSOLE_OUTPUT);
    }

    Send(SocketID2, Content2, len(Content2));
    Receive(SocketID2, Content2, len(Content2));
    Write(Content2, len(Content2), CONSOLE_OUTPUT);
    Send(SocketID2, ByeContent, len(ByeContent));
    // closeResult2 = Close(SocketID2);
    if (closeResult2 == -1)
    {
        Write("Error closing SocketID2\n", len("Error closing SocketID2\n"), CONSOLE_OUTPUT);
    }

    Send(SocketID3, Content3, len(Content3));
    Receive(SocketID3, Content3, len(Content3));
    Write(Content3, len(Content3), CONSOLE_OUTPUT);
    Send(SocketID3, ByeContent, len(ByeContent));
    // closeResult3 = Close(SocketID3);
    if (closeResult3 == -1)
    {
        Write("Error closing SocketID3\n", len("Error closing SocketID3\n"), CONSOLE_OUTPUT);
    }

    Send(SocketID4, Content4, len(Content4));
    Receive(SocketID4, Content4, len(Content4));
    Write(Content4, len(Content4), CONSOLE_OUTPUT);
    Send(SocketID4, ByeContent, len(ByeContent));
    // closeResult4 = Close(SocketID4);
    if (closeResult4 == -1)
    {
        Write("Error closing SocketID4\n", len("Error closing SocketID4\n"), CONSOLE_OUTPUT);
    }

    Halt();
}