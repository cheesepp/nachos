#include "main.h"
#include "synchconsole.h"
#include "machine.h"
#include "post.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Define for mode
#define SOCKET_MODE 2

/*
    Class File duoc dung de quan li cac file dang duoc mo va cac socket
    File se luu FilePointer, file descriptor, type(doc, doc-ghi, socket)
    File ID la 1 so nguyen duong
    duoc tich hop boi gia tri type va file descriptor voi cong thuc:
    ! ID = type*100 + FilePointer->ID()
    ! Cac gia tri type duoc dinh nghia o cac dong: 21,22,23 cua file nay

    => Voi cong thuc nay, ham se tra ve 1 so co 3 chu so voi chu so hang tram la type file
    va 2 chu so con lai la file id duoc tao ra boi Nachos
    !ID se giup phan biet giua file thong thuong va socket-> Giup hoan thanh phan advanced
*/

struct Socket
{
    int socketID;
    ~Socket()
    {
        if (socketID > -1)
        {
            CloseSocket(socketID);
        }
        socketID = -1;
    }
};

struct NormalFile
{
    char *name;
    OpenFile *FilePointer;
    int currentOffset;
    ~NormalFile()
    {
        currentOffset = -1;
    }
};

typedef int OpenFileId;

class FileDescriptor
{
private:
    Socket mySocket;
    NormalFile myFile;
    OpenFileId id;
    int type;

public:
    FileDescriptor()
    {
        this->id = -1;
        this->type = -1;
        this->myFile.currentOffset = -1;
        this->myFile.FilePointer = NULL;
    }

    ~FileDescriptor()
    {
        this->id = -1;
        this->type = -1;
    }

    int getType() { 
        return this->type;
    }

    OpenFileId getID() { 
        return this->id; 
    }

    int getSocketID() { 
        return this->mySocket.socketID;
    }

    char *getName() { 
        return this->myFile.name;
    }

    int getOffset() { 
        return this->myFile.currentOffset; 
    }

    int openFile(char *name, int type)
    {

        this->myFile.FilePointer = kernel->fileSystem->Open(name);

        int fid = this->myFile.FilePointer->GetFileID();

        if (this->myFile.FilePointer != NULL)
        {
            this->myFile.name = name;
            this->type = type;
            this->id = (type*100) + fid;
            this->myFile.currentOffset = 0; // Filesystem Object sets Offset = 0 as default
            return id;
        }
        else
            return -1;
    }

    int openSocket()
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd < 0)
        {
            cout << "create socket failed!";
            return -1;
        }

        this->mySocket.socketID = sockfd;
        this->type = SOCKET_MODE;
        this->id = SOCKET_MODE * 100 + this->mySocket.socketID;
        return this->mySocket.socketID;
    }

    int connectSocket(char *ip, int port)
    {
        // Create a new sockaddr_in struct to represent the remote host
        struct sockaddr_in remote_addr;
        remote_addr.sin_family = AF_INET;            // Use IPv4 protocol
        remote_addr.sin_port = htons(port);          // Convert port to network byte order
        remote_addr.sin_addr.s_addr = inet_addr(ip); // Convert IP address to network byte order

        // Try to connect to the remote host using the existing socket ID
        if (connect(this->mySocket.socketID, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
        {
            perror("Error connecting to remote host");
            CloseSocket(this->mySocket.socketID); // Close socket on failure
            return -1;                           // Return -1 on failure
        }

        cerr << "Connected successfully\n";
        return this->mySocket.socketID; // Return socket file descriptor on success
    }

    int ReadFile(char *buffer, int charCount)
    {
        int result = -1;

        if (!this->myFile.FilePointer)
        {
            cout << "File doesn't exist!\n";
            return -1;
        }
        result = myFile.FilePointer->Read(buffer, charCount);
        if (result < charCount)
            return -2;

        return result;
    }

    int WriteFile(char *buffer, int charCount)
    {
        int result;

        if (this->type == 1) // if read only -> invalid
        {
            cout << "INVALID MODE!\n";
            return -1;
        }

        if (!this->myFile.FilePointer)
        {
            cout << "File doesn't exist!\n";
            return -1;
        }
        
        result = this->myFile.FilePointer->WriteAt(buffer, charCount, myFile.FilePointer->Length() + 1);

        if (result != charCount)
            return -2;
        cout << "Written success!\n";
        return result;
    }

    int seekFile(int position)
    {
        if (this->type == SOCKET_MODE)
            return -1;
        if (position >= myFile.FilePointer->Length()) {
            cout << "vao file pointer!";
            return -1;
        }
        if (position == -1) {
            cout << "vao seek!";
            return myFile.FilePointer->Seek(myFile.FilePointer->Length());
        }
        return myFile.FilePointer->Seek(position);
    }

    int CloseFile()
    {
        if (this->type == -1)
            return -1;
        if (this->type == SOCKET_MODE)
            this->mySocket.~Socket();
        else if (this->myFile.FilePointer)
        {
            myFile.FilePointer->~OpenFile();
            myFile.FilePointer = NULL;
            this->type = -1;
            this->id = -1;
        }
    }
};

class Table
{
public:
    const int MaxSize = 20;
    int currentSize;
    FileDescriptor *table;

public:
    Table()
    {
        this->table = new FileDescriptor[20];
        currentSize = 0;
    }
    ~Table()
    {
        delete[] table;
    }

    OpenFileId isOpened(OpenFileId id)
    {
        for (int i = 0; i < MaxSize; i++)
        {
            if (table[i].getID() == id)
                return table[i].getID();
        }
        return -1;
    }

    // Check file id from file name
    OpenFileId isOpened(char *name, int type)
    {
        for (int i = 0; i < MaxSize; i++)
            if (table[i].getName())
            {
                if (strcasecmp(name, table[i].getName()) == 0 && table[i].getType() == type)
                    return table[i].getID();
            }
        return -1;
    }

    // close file with file id
    int closeFile(OpenFileId id)
    {
        int i = isOpened(id); // Check if it is opened
        if (i < 0)
            return -1;
        // for socket mode
        if (table[i].getType() != SOCKET_MODE)
        {
            table[i].CloseFile();
            currentSize--;
        }
        else
        {
            close(table[i].getSocketID());
        }
        return 0;
    }

    OpenFileId open(char *name, int type)
    {
        if (currentSize == MaxSize)
            return -1; // Mang day
        for (int i = 0; i < MaxSize; i++)
        {
            if (table[i].getID() == -1)
            {
                if (type != SOCKET_MODE)
                    this->table[i].openFile(name, type);
                else
                    this->table[i].openSocket();

                if (this->table[i].getID() != -1)
                {
                    currentSize++;
                    return this->table[i].getID();
                }
                else
                    return -1;
            }
        }
        return -1; // Full
    }

    FileDescriptor *getFile(OpenFileId id)
    {
        for (int i = 0; i < MaxSize; i++)
        {
            if (table[i].getID() == id)
                return &table[i];
        }
        return NULL;
    }
};
