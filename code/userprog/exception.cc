// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "stdint.h"
#include "synchconsole.h"
#include "file_descriptors.h"

#define MaxFileLength 32
#define MaxStringLength 256

Table table;
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

/*
	Increase the program counter to point the next instruction
	Copied from SC_Add
*/

void increaseProgramCounter()
{
	/* set previous programm counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(NextPCReg));

	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(NextPCReg) + 4);
}

// Send the data to the server and set the timeout of 20 seconds
//! For advanced part, we don't use this function
int SocketSend(char *buffer, int charCount, int fileId)
{
	int shortRetval = -1;
	struct timeval tv;
	tv.tv_sec = 20; /* 20 Secs Timeout */
	tv.tv_usec = 0;
	if (setsockopt(fileId, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)) < 0)
	{
		printf("Time Out\n");
		return -1;
	}
	shortRetval = send(fileId, buffer, charCount, 0);

	return shortRetval;
}

// receive the data from the server
//! For advanced part, we don't use this function
int SocketReceive(char *buffer, int charCount, int fileId)
{
	int shortRetval = -1;
	struct timeval tv;
	tv.tv_sec = 20; /* 20 Secs Timeout */
	tv.tv_usec = 0;
	if (setsockopt(fileId, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)) < 0)
	{
		printf("Time Out\n");
		return -1;
	}
	shortRetval = recv(fileId, buffer, charCount, 0);
	cout << "Response: " << buffer << "(" << charCount << ")" << endl;
	return shortRetval;
}

// Code copied from file
//"[2] Giao tiep giua HDH Nachos va chuong trinh nguoi dung.pdf"
//  Input: - User space address (int)
//  - Limit of buffer (int)
//  Output:- Buffer (char*)
//  Purpose: Copy buffer from User memory space to System memory space
char *User2System(int virtAddr, int limit)
{
	int i; // index
	int oneChar;
	char *kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; // need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);
	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Code copied from file
//"[2] Giao tiep giua HDH Nachos va chuong trinh nguoi dung.pdf"
// Input: - User space address (int)
// - Limit of buffer (int)
// - Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

#define MAX_NUM_LENGTH 11

//-2147483648 <= int32 <=2147483647
// so the maximum length of an int32 string is 11
// A character buffer to read and write int32 number
char characterBuffer[MAX_NUM_LENGTH + 1];

// Check if a character is empty space
char isEmptySpace(char ch)
{
	return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\x0b');
}

/**
 * Read characters from console and store them in characterBuffer
 * Stops when meeting an empty space character:
 *  '\n' Line Feed
 *  '\r' Carriage Return
 *  '\t' Horizontal Tab
 *  ' ' Space
 *  '\x0b' 	Vertical Tab
 *  EOF
 **/
void readCharacters()
{
	memset(characterBuffer, 0, sizeof(characterBuffer));
	char ch = kernel->synchConsoleIn->GetChar();

	// Read the first character and check
	if (ch == EOF || isEmptySpace(ch))
	{
		DEBUG(dbgSys, "Illegal character found, ascii code " << (int)ch << '\n');
		return;
	}

	int len = 0;
	// Reads until meeting an empty space or EOF
	while (!(isEmptySpace(ch) || ch == EOF))
	{
		characterBuffer[len++] = ch;
		if (len > MAX_NUM_LENGTH)
		{
			DEBUG(dbgSys, "The string is too long to fit in int32");
			return;
		}
		ch = kernel->synchConsoleIn->GetChar();
	}
}

// Read an integer from console and return
// Uses the above function to read characters from console and store them in characterBuffer
// Return 0 if meeting errors:
// - Read letters instead of number
// - integer overflow
int ReadNumFromConsole()
{
	readCharacters();

	int len = strlen(characterBuffer);
	// Return 0 if kernel didn't read any character
	if (len == 0)
		return 0;

	// 2147483648 doesn't fit in int32 so we can't store it in an int32 integer
	// Therefore, the string "-2147483648" must be compare manually
	if (strcmp(characterBuffer, "-2147483648") == 0)
		return INT32_MIN;

	bool negative = characterBuffer[0] == '-';

	int num = 0;

	for (int i = negative; i < len; ++i)
	{

		char c = characterBuffer[i];

		if (c < '0' || c > '9')
		{
			DEBUG(dbgSys, "Illegal character found " << characterBuffer << ", number expected");
			return 0;
		}
		num = num * 10 + (c - '0');
	}

	if (negative)
		num = -num;

	// If negative is true but num is positive
	// or if negative is false but num is negative
	// then integer overflow happened
	if ((negative && num > 0) || (!negative && num < 0))
	{
		DEBUG(dbgSys, "Number " << characterBuffer << " doesn't fit in int32");
		return 0;
	}

	// Num is safe to return
	return num;
}

// Print an integer to console
// Uses synchConsoleOut->PutChar to print every digit
void PrintNumToConsole(int num)
{
	// Print out '0' if num is 0
	if (num == 0)
		return kernel->synchConsoleOut->PutChar('0');

	// Because 2147483648 doesn't fit in int32, we must print -2147483648 (INT32_MIN) manually
	if (num == INT32_MIN)
	{
		kernel->synchConsoleOut->PutChar('-');
		for (int i = 0; i < 10; ++i)
			kernel->synchConsoleOut->PutChar("2147483648"[i]);
		return;
	}
	// If num < 0, print '-' and reverse the sign of num
	if (num < 0)
	{
		kernel->synchConsoleOut->PutChar('-');
		num = -num;
	}

	int n = 0;
	// Store num's digits in characterBuffer
	while (num)
	{
		characterBuffer[n++] = num % 10;
		num /= 10;
	}
	// Print to console
	for (int i = n - 1; i >= 0; --i)
		kernel->synchConsoleOut->PutChar(characterBuffer[i] + '0');
}

// Read and return a character from console
char ReadCharFromConsole()
{
	return kernel->synchConsoleIn->GetChar();
}

// Print a character to console
void PrintCharToConsole(char ch)
{
	kernel->synchConsoleOut->PutChar(ch);
}

// Return a random positive integer between 1 and INT32_MAX (inclusive)
int GetRandomNumber()
{
	// Call rand from stdlib to create a random number
	int num = rand();
	// GetRandomNumber must return a positive integer
	if (num == 0)
		++num;
	return num;
}

// Read and return a string from console
// Stop when reaching max length or meeting '\n'
char *ReadStringFromConsole(int len)
{
	char *str;
	str = new char[len + 1];
	for (int i = 0; i < len; ++i)
	{
		str[i] = kernel->synchConsoleIn->GetChar();
		// If str[i] = '\n' then assign str[i] = '\0' and return the string
		if (str[i] == '\n')
		{
			str[i] = '\0';
			return str;
		}
	}
	str[len] = '\0';
	return str;
}

// Print a string to console
// Stop when reaching maxLen or meeting '\0'
void PrintStringToConsole(char *str, int maxLen)
{
	int i = 0;
	while (str[i] != '\0' && i < maxLen)
	{
		kernel->synchConsoleOut->PutChar(str[i]);
		++i;
	}
}

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
		// Handle exceptions listed in machine / machine.h
	case NoException:
		DEBUG(dbgSys, "No exception.\n");
		return;

	case PageFaultException:
		DEBUG(dbgSys, "No valid translation found.\n");
		cerr << "No valid translation found. ExceptionType " << which << '\n';
		SysHalt();

		ASSERTNOTREACHED();
		break;
	case ReadOnlyException:
		DEBUG(dbgSys, "Write attempted to page marked read-only.\n");
		cerr << "Write attempted to page marked read-only. ExceptionType " << which << '\n';
		SysHalt();

		ASSERTNOTREACHED();
		break;
	case BusErrorException:
		DEBUG(dbgSys, "Translation resulted in an invalid physical address.\n");
		cerr << "Translation resulted in an invalid physical address. ExceptionType " << which << '\n';
		SysHalt();

		ASSERTNOTREACHED();
		break;
	case AddressErrorException:
		DEBUG(dbgSys, "Unaligned reference or one that was beyond the end of the address space.\n");
		cerr << "Unaligned reference or one that was beyond the end of the address space. ExceptionType " << which << '\n';
		SysHalt();

		ASSERTNOTREACHED();
		break;
	case OverflowException:
		DEBUG(dbgSys, "Integer overflow in add or sub.\n");
		cerr << "Integer overflow in add or sub. ExceptionType " << which << '\n';
		SysHalt();

		ASSERTNOTREACHED();
		break;
	case IllegalInstrException:
		DEBUG(dbgSys, "Unimplemented or reserved instr.\n");
		cerr << "Unimplemented or reserved instr. ExceptionType " << which << '\n';
		SysHalt();

		ASSERTNOTREACHED();
		break;
	case NumExceptionTypes:
		DEBUG(dbgSys, "NumExceptionTypes.\n");
		cerr << "NumExceptionTypes. ExceptionType " << which << '\n';
		SysHalt();

		ASSERTNOTREACHED();
		break;
	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();

		case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			/* Modify return point */
			increaseProgramCounter();

			return;

		case SC_Create:
		{
			// get address and file name
			int virtAddr;
			char *filename;
			DEBUG(dbgSys, "\n SC_Create call ...");
			DEBUG(dbgSys, "\n Reading virtual address of filename");
			// get params from register r4
			virtAddr = kernel->machine->ReadRegister(4);
			DEBUG(dbgSys, "\n Reading filename.");
			// MaxFileLength = 32
			filename = User2System(virtAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				printf("\n Not enough memory in system");
				DEBUG(dbgSys, "\n Not enough memory in system");
				kernel->machine->WriteRegister(2, -1); // return error to user program
				delete filename;
				return;
			}
			DEBUG(dbgSys, "\n Finish reading filename.");
			// If can not create file
			if (!kernel->fileSystem->Create(filename))
			{
				printf("\n Error create file '%s'", filename);
				kernel->machine->WriteRegister(2, -1);
				delete filename;
				return;
			}
			kernel->machine->WriteRegister(2, 0); // successed -> return 0 to user
			delete filename;
			increaseProgramCounter();
			return;
		}

		case SC_Open:
		{
			DEBUG(dbgSys, "GO Open!\n");
			int virtAddr = kernel->machine->ReadRegister(4);
			int type = kernel->machine->ReadRegister(5);
			char *filename;
			filename = User2System(virtAddr, MaxFileLength);

			if (type == 0 || type == 1) // execute when type = 0 or = 1
			{
				int openFileId = table.open(filename, type);

				if (openFileId != -1) // Open file successfully
				{
					DEBUG(dbgSys, "\nOpened file");
					kernel->machine->WriteRegister(2, openFileId); // return OpenFileID
					delete[] filename;
					increaseProgramCounter();
					return;
					ASSERTNOTREACHED();
					break;
				}
			}
			kernel->machine->WriteRegister(2, -1); // Can not open file
			delete[] filename;
			increaseProgramCounter();
			return;
		}

		case SC_Close:
		{

			int fid = kernel->machine->ReadRegister(4);
			if (table.isOpened(fid)) // if open file successfully
			{
				for (int i = 0; i < 20; i++)
				{
					if (table.table[i].getID() == fid)
					{
						table.table[i].CloseFile();
						break;
					}
				}
				kernel->machine->WriteRegister(2, 0);
				increaseProgramCounter();

				return;
			}
			DEBUG(dbgSys, "Closed file!\n");
			kernel->machine->WriteRegister(2, -1);
			increaseProgramCounter();

			return;
		}
		case SC_Remove:
		{
			int virtAddr = kernel->machine->ReadRegister(4);
			// get string from User Space to System Space with MaxFileLength
			char *filename = User2System(virtAddr, MaxFileLength);

			// Handle excetion when openfile
			OpenFileId isOpen = table.isOpened(filename, 0);

			if (isOpen > -1)
			{
				table.closeFile(isOpen);
				increaseProgramCounter();
				return;
			}

			isOpen = table.isOpened(filename, 1);

			if (isOpen > -1)
			{

				table.closeFile(isOpen);
				increaseProgramCounter();
				return;
			}

			kernel->machine->WriteRegister(2, kernel->fileSystem->Remove(filename) ? 0 : -1);

			increaseProgramCounter();
			return;
		}

		case SC_ReadChar:
		{
			// Đọc character từ console và ghi vào thanh ghi số 2
			char character = ReadCharFromConsole();
			kernel->machine->WriteRegister(2, (int)character);

			increaseProgramCounter();
			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_PrintChar:
		{
			// Lấy tham số cần in từ thanh ghi r4
			char character = (char)kernel->machine->ReadRegister(4);
			// In ra console
			PrintCharToConsole(character);

			increaseProgramCounter();

			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_ReadString:
		{
			// Đọc địa chỉ string từ thanh ghi r4
			int userString = kernel->machine->ReadRegister(4);
			// Đọc chiều dài string từ thanh ghi r5
			int len = kernel->machine->ReadRegister(5);

			if (len > MaxStringLength || len < 1)
			{
				DEBUG(dbgSys, "String length must be between 1 and " << MaxStringLength << " (inclusive)\n");
				SysHalt();
			}
			DEBUG(dbgSys, "String length: " << len);

			char *systemString = ReadStringFromConsole(len);
			// Chuyển dữ liệu từ kernel space qua userspace
			System2User(userString, MaxStringLength, systemString);
			delete[] systemString;

			increaseProgramCounter();

			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_PrintString:
		{
			// Đọc địa chỉ string từ thanh ghi r4
			int userString = kernel->machine->ReadRegister(4);

			// Chuyển dữ liệu từ userspace qua kernelspace
			char *systemString = User2System(userString, MaxStringLength);

			// In ra console
			PrintStringToConsole(systemString, MaxStringLength);
			delete[] systemString;

			increaseProgramCounter();

			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_Read:
		{
			DEBUG(dbgSys, "Go read file!\n");
			// get buffer from register 4
			int virtAddr = kernel->machine->ReadRegister(4);
			// get char count from register 5
			int charCount = kernel->machine->ReadRegister(5);
			// copy memory from User Space to System Space with buffer has length charCount
			char *buff = User2System(virtAddr, charCount);
			// get file id from register 6
			int id = kernel->machine->ReadRegister(6);

			DEBUG(dbgSys, "Read " << charCount << " chars from file " << id << "\n");

			// if ((id < 0) || (id > 19))
			// {
			// 	kernel->machine->WriteRegister(2, -1);
			// 	DEBUG(dbgSys, "E: ID is not in table file descriptor\n");
			// 	increaseProgramCounter();
			// 	return;
			// 	ASSERTNOTREACHED();
			// 	break;
			// }

			// file id < 0 -> return error
			if (id < 0)
			{
				kernel->machine->WriteRegister(2, -1);
			}
			// id = 0 -> read from console
			if (id == SysConsoleInput)
			{
				kernel->machine->WriteRegister(2, kernel->synchConsoleIn->GetString(buff, charCount));
			}
			// can not get file id -> return error
			if (!table.getFile(id))
			{
				kernel->machine->WriteRegister(2, -1);
			}
			else
			{
				// successed -> read file
				kernel->machine->WriteRegister(2, table.getFile(id)->ReadFile(buff, charCount));
			}
			DEBUG(dbgSys, "Read "
							  << "file successfully"
							  << "\n");
			// Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer dai charCount
			System2User(virtAddr, charCount, buff);
			delete[] buff;

			increaseProgramCounter();
			return;
		}

		case SC_Write:
		{
			DEBUG(dbgSys, "Write file!\n");
			int virtAddr = kernel->machine->ReadRegister(4);
			int charCount = kernel->machine->ReadRegister(5);
			int id = kernel->machine->ReadRegister(6);
			char *buffer = User2System(virtAddr, charCount);

			// if ((id < 0) || (id > 19))
			// {
			// 	kernel->machine->WriteRegister(2, -1);
			// 	DEBUG(dbgSys, "E: ID is not in table file descriptor\n");
			// 	increaseProgramCounter();
			// 	return;
			// 	ASSERTNOTREACHED();
			// 	break;
			// }
			// id = 1 then print on console
			if (id == SysConsoleOutput)
			{
				DEBUG(dbgSys, "ConsoleOutput\n");
				kernel->machine->WriteRegister(2, kernel->synchConsoleOut->PutString(buffer, charCount));
				increaseProgramCounter();
				return;
			}
			// can not get file -> return error
			if (!table.getFile(id))
			{
				DEBUG(dbgSys, "E: File is not existed\n");
				kernel->machine->WriteRegister(2, -1);
				increaseProgramCounter();
				return;
			}

			// not read only -> read and write
			if (id / 100 != 1)
			{
				// get file successfully -> return writefile to result or -1 if can not get file
				int result = (table.getFile(id) != NULL ? table.getFile(id)->WriteFile(buffer, charCount) : -1);
				if (result == -1)
				{
					cout << "Cannot Write\n";
				}
				// write result to register
				kernel->machine->WriteRegister(2, result);
				increaseProgramCounter();
				return;
			}

			// return error if no condition is matched
			kernel->machine->WriteRegister(2, -1);
			// copy memory from System Space to User Space with buffer has length charCount
			System2User(virtAddr, charCount, buffer);
			// release buffer memory
			delete[] buffer;

			increaseProgramCounter();
			return;
		}

		case SC_Seek:
		{
			// get position and file id
			int pos = kernel->machine->ReadRegister(4);
			int id = kernel->machine->ReadRegister(5);

			// if id for console input and output -> return error
			if (id == 0 || id == 1)
			{

				kernel->machine->WriteRegister(2, -1);
				increaseProgramCounter();
				return;
			}
			// can not get file id -> return errro
			if (!table.getFile(id))
			{
				kernel->machine->WriteRegister(2, -1);
				increaseProgramCounter();
				return;
			}
			// DEBUG(dbgSys, "go seek!\n");
			// seek with position in file
			int result = table.getFile(id)->seekFile(pos);
			// cout << "Seeking to: " << result << endl;
			// get file successed -> write file offset to register
			kernel->machine->WriteRegister(2, table.getFile(id)->getOffset());

			increaseProgramCounter();
			return;
		}

		case SC_Connect:
		{
			DEBUG(dbgSys, "Connect file!\n");
			int arg1 = kernel->machine->ReadRegister(4); // socketid
			int arg2 = kernel->machine->ReadRegister(5); // ip
			int arg3 = kernel->machine->ReadRegister(6); // port

			// convert the IP address from user space to kernel space
			char *kernelIP = new char[MaxStringLength];
			copyStringFromMachine(arg2, kernelIP, MaxStringLength);

			if (table.getFile(arg1)->connectSocket(kernelIP, arg3) < 0)
			{
				DEBUG(dbgSys, "Connection failed!\n");
				return;
			}

			// set the return value
			kernel->machine->WriteRegister(2, arg1);

			delete[] kernelIP;
			increaseProgramCounter();
			return;
		}

		case SC_SocketTCP:
		{
			DEBUG(dbgSys, "Create socket!\n");
			// create socket
			int fileid = table.open(NULL, SOCKET_MODE);
			DEBUG(dbgSys, "Open " << fileid << " socket file!\n");
			// write file id to register
			kernel->machine->WriteRegister(2, fileid);

			increaseProgramCounter();
			return;
		}

		case SC_Send:
		{
			DEBUG(dbgSys, "Send message socket!\n");
			int addrSocketid      = kernel->machine->ReadRegister(4);
			int idbuffer = kernel->machine->ReadRegister(5);
			int len = kernel->machine->ReadRegister(6);

			// get file id of socket
			int socketid = table.getFile(addrSocketid)->getSocketID();
			// create buffer array
			char *buffer = new char[len];
			// copy memory of id buffer from user space to system space and save to buffer
			buffer = User2System(idbuffer, len);
			// send socket
			SocketSend(buffer, len, socketid);

			DEBUG(dbgSys, "Send mess successfully!\n");
			increaseProgramCounter();
			return;
			ASSERTNOTREACHED();
		}

		case SC_Receive:
		{
			DEBUG(dbgSys, "Receive message socket!\n");
			int addrSocketid = kernel->machine->ReadRegister(4);
			int idbuffer = kernel->machine->ReadRegister(5);
			int len = kernel->machine->ReadRegister(6);

			cout << "Address Socket Id: " << addrSocketid << endl;
			// get socket file id
			int socketid = table.getFile(addrSocketid)->getSocketID();
			cout << "Socket Id: " << socketid << endl;
			char *buffer = new char[len];
			// receive message and save to buffer
			SocketReceive(buffer, len, socketid);
			// copy memory from system space to user space and save to id buffer
			System2User(idbuffer, len, buffer);
			DEBUG(dbgSys, "Receive mess successfully!\n");

			increaseProgramCounter();
			return;
			ASSERTNOTREACHED();
		}

		case SC_Exec:
		{
			DEBUG(dbgSys, "go exec");

			int addr = kernel->machine->ReadRegister(4);
			DEBUG(dbgSys, "Register 4: " << addr);

			char *fileName;
			fileName = User2System(addr, 255);
			DEBUG(dbgSys, "Read file name: " << fileName);
			DEBUG(dbgSys, "Scheduling execution..." << kernel->pTable);
			DEBUG(dbgSys, "Scheduling execution...");
			int result = kernel->pTable->ExecuteUpdate(fileName);

			DEBUG(dbgSys, "Writing result to register 2: " << result);
			kernel->machine->WriteRegister(2, result);
			delete fileName;

			increaseProgramCounter();
			return;
			ASSERTNOTREACHED();
		}

		case SC_CreateSemaphore:
		{
			// Load name and value of semaphore
			int virtAddr = kernel->machine->ReadRegister(4);	   // read name address from 4th register
			int semVal = kernel->machine->ReadRegister(5);		   // read type from 5th register
			char *name = User2System(virtAddr, MaxFileLength); // Copy semaphore name charArray form userSpace to systemSpace

			// Validate name
			if (name == NULL)
			{
				// DEBUG(dbgSynch, "\nNot enough memory in System");
				cerr << "Not enough memory in System\n";
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				return;
			}

			int res = kernel->sTable->Create(name, semVal);

			// Check error
			if (res == -1)
			{
				// DEBUG('a', "\nCan not create semaphore");
				cerr << "Can not create semaphore" << endl;
			}

			delete[] name;
			kernel->machine->WriteRegister(2, res);

			increaseProgramCounter();
			return;
			ASSERTNOTREACHED();
		}

		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}
	DEBUG(dbgSys, "hehe\n");
	ASSERTNOTREACHED();
}