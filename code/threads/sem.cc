#include "sem.h"
#include "synch.h"
#include <stdio.h>

// Initial the Sem object, the started value is null
// Remember to initial the Sem to use
Sem::Sem(char *na, int i)
{
    strcpy(this->name, na);
    sem = new Semaphore(this->name, i);
}
Sem::~Sem() // Destruct the Sem object
{
    if (sem)
        delete sem;
}
void Sem::wait()
{
    sem->P(); // Conduct the waiting function
}
void Sem::signal()
{
    sem->V(); // Release semaphore
}

char *Sem::GetName() // Return the semaphore name
{
    return this->name;
}