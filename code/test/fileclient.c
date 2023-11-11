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

    // Mở file "text.txt"
    id1 = Open("text.txt", 0);
    if (id1 == -1)
    {
        Write("Error opening file 'text.txt'\n", len("Error opening file 'text.txt'\n"), CONSOLE_OUTPUT);
        Halt();
    }

    // Đọc nội dung từ file
    result = Read(Content, 256, id1);
    if (result == -1)
    {
        Write("Error reading from file 'text.txt'\n", len("Error reading from file 'text.txt'\n"), CONSOLE_OUTPUT);
        Close(id1);
        Halt();
    }

    // Tạo socket
    SocketID1 = SocketTCP();
    if (SocketID1 == -1)
    {
        Write("Error creating socket\n", len("Error creating socket\n"), CONSOLE_OUTPUT);
        Close(id1);
        Halt();
    }

    // Kết nối tới máy chủ
    result = Connect(SocketID1, "127.0.0.1", 8081);
    if (result == -1)
    {
        Write("Error connecting to server\n", len("Error connecting to server\n"), CONSOLE_OUTPUT);
        Close(id1);
        Close(SocketID1);
        Halt();
    }

    // Gửi nội dung qua socket
    Send(SocketID1, Content, len(Content));
    // Nhận nội dung từ socket
    Receive(SocketID1, Content, len(Content));
    
    // Mở file "text2.txt"
    id2 = Open("text2.txt", 0);
    if (id2 == -1)
    {
        Write("Error opening file 'text2.txt'\n", len("Error opening file 'text2.txt'\n"), CONSOLE_OUTPUT);
        Close(id1);
        Close(SocketID1);
        Halt();
    }

    // Ghi nội dung vào file "text2.txt"
    result = Write(Content, len(Content), id2);
    if (result == -1)
    {
        Write("Error writing to file 'text2.txt'\n", len("Error writing to file 'text2.txt'\n"), CONSOLE_OUTPUT);
        Close(id1);
        Close(SocketID1);
        Close(id2);
        Halt();
    }

    // Đóng các tài nguyên
    Close(id1);
    Close(SocketID1);
    Close(id2);
    Halt();
}