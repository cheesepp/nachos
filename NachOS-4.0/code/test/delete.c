#include "syscall.h"

#define MAX_LENGTH 32
#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

int main() {
    int res, id;
    char filename[MAX_LENGTH];

    Write("Nhap ten file can xoa\n",30,CONSOLE_OUTPUT);
	ReadString(filename, MAX_LENGTH); // call ReadString to read the source file
	
    res = Remove(filename);
    if (res == -1)
    {
        Write("Khong the xoa file! \n",30,CONSOLE_OUTPUT);
    }
    else
    {
        Write("Xoa file thanh cong!",30,CONSOLE_OUTPUT);
    }
    Halt();
}