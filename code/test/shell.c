#include "syscall.h"
#define SysConsoleInput	0  
#define SysConsoleOutput	1  
typedef int OpenFileId;
typedef int SpaceId;

int main()
{
    SpaceId newProc;
    OpenFileId input = SysConsoleInput;
    OpenFileId output = SysConsoleOutput;
    char prompt[2], ch, buffer[60];
    int i;

    prompt[0] = '-';
    prompt[1] = '-';

    while( 1 )
    {
	    Write(prompt, 2, output);

	    i = 0;
	
    	do {
            
	        Read(&buffer[i], 1, input); 

	    } while( buffer[i++] != '\n' );

	    buffer[--i] = '\0';

	    if( i > 0 ) {
		    newProc = Exec(buffer);
		    Join(newProc);
	    }
    }

    Halt();
}

