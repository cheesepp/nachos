#include "syscall.h"


void main()
{
	int pingPID, pongPID;
	PrintString("Ping-Pong test starting...\n\n");
	
    pingPID = Exec("../test/ping");
	


	pongPID = Exec("../test/pong");

	PrintString("Done pong exec\n");

	Join(pingPID);
	Join(pongPID);	
}
