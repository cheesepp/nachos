#include "syscall.h"
#define MAX_LENGTH 32

int main()
{
	//OpenFileId of source file and destination file
	int srcFileId;
	int destFileId;
	int i; //Index for loop
    char c;
	char source[MAX_LENGTH];
	char dest[MAX_LENGTH];
    // save string buffer
	char buffer[256];
	
	PrintString("\n\t\t\tSAO CHEP FILE\n\n");
	PrintString("Nhap ten file nguon: ");
	ReadString(source, MAX_LENGTH); // call ReadString to read the source file
	
	PrintString("Nhap ten file dich: ");
	ReadString(dest, MAX_LENGTH);  // call ReadString to read the destination file
	srcFileId = Open(source, 0); // open source file
	if (srcFileId != -1) // check if source file is opened
	{
		//create new destination file
		destFileId = Create(dest);
		Close(destFileId);
		
		destFileId = Open(dest, 0); // open dest file
		if (destFileId != -1) 
		{
			// read source file id and save to buffer
			Read(buffer,256, srcFileId);
			
			Seek(0, srcFileId); // Seek to start source file
			Seek(0, destFileId); // Seek to start of dest file
			i = 0;
			
			// iterate each character in source file and write to dest file
			for (; i < len(buffer); i++) 
			{
				Read(&c, 1, srcFileId); // Read each char of source file
				Write(&c, 1, destFileId); // write to dest
			}
			
			PrintString("Sao chep thanh cong!\n\n");
			Close(destFileId); // close dest file after writing
		}
		else
		{
			PrintString("\nKhong the tao file dich!\n\n");
		}
		
		Close(srcFileId); // Goi ham Close de dong file nguon
	}
	else
	{
		PrintString("\nMo file that bai!");
	}
	
	Halt();
}