/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__
#define __USERPROG_KSYSCALL_H__

#include "kernel.h"

void SysHalt()
{
  kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

void copyStringFromMachine(int from, char *to, unsigned size)
{
  ASSERT(size >= 0);

  unsigned i = 0;

  // Loop through the bytes until we find the null terminator or reach the end of the buffer.
  while (i < size && kernel->machine->ReadMem(from + i, 1, (int *)&to[i]))
  {
    if (to[i] == '\0')
      return;
    ++i;
  }

  // If we didn't find the null terminator, make sure the buffer is null-terminated.
  if (i == size)
    --i;
  to[i] = '\0';
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
