#include "syscall.h"

#define CONSOLE_INPUT 0
#define CONSOLE_OUTPUT 1

#define MAX_LENGTH 32
int main() {
    int id1, id2, id3;
    char buffer[256],buffer2[256];
    char *Content;

    char source[MAX_LENGTH];
	char dest[MAX_LENGTH];
	
	PrintString("\n\t\t\tGHEP FILE\n\n");
	PrintString("Nhap ten file nguon: ");
	ReadString(source, MAX_LENGTH); // call ReadString to read the source file
	
	PrintString("Nhap ten file dich: ");
	ReadString(dest, MAX_LENGTH);  // call ReadString to read the destination file
    id1 = Open(source, 0);
    id2 = Open(dest, 0);
    Seek(-1,id2);
    Read(buffer,256, id1);
    Write(buffer, len(buffer), id2);
    Close(id1);
    Close(id2);
    PrintString("Ghep file thanh cong!\n");
    Halt();
}
