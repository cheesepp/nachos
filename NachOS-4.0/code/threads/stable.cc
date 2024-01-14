#include "stable.h"

STable::STable()
{
    this->map = new Bitmap(MAX_SEMAPHORE);

    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        this->table[i] = NULL;
    }
}

STable::~STable()
{
    if (this->map)
    {
        delete this->map;
        this->map = NULL;
    }

    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        if (this->table[i])
        {
            delete this->table[i];
            this->table[i] = NULL;
        }
    }
}

int STable::Create(char *name, int value)
{
    DEBUG(dbgSynch, "STable::Create MySemaphore " << name << " - " << value);
    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        if (this->map->Test(i))
        {
            if (strcmp(name, this->table[i]->GetName()) == 0)
            {
                return -1;
            }
        }
    }

    int id = this->FindFreeSlot();

    if (id < 0)
    {
        DEBUG(dbgSynch, "STable: Error table is full");
        return -1;
    }

    this->table[id] = new Sem(name, value);
    DEBUG(dbgSynch, "STable: Create in table[" << id << "]");
    return 0;
}

int STable::Wait(char *name)
{
    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        // Check if a semaphore has been created in slot i
        if (this->map->Test(i))
        {
            // If a semaphore exists, compare its name with the input name
            if (strcmp(name, this->table[i]->GetName()) == 0)
            {
                // If the semaphore exists, call its wait() method
                this->table[i]->wait();
                return 0;
            }
        }
    }
    // cerr <<"A";
    return -1;
}

int STable::Signal(char *name)
{
    DEBUG(dbgSynch, "STable::Signal(\"" << name << "\")");
    for (int i = 0; i < MAX_SEMAPHORE; i++)
    {
        // Check does slot[i] load MySemaphore
        if (this->map->Test(i))
        {
            // if yes then compare nam with name of MySemaphore in table
            if (strcmp(name, table[i]->GetName()) == 0)
            {
                // If exist then make MySemaphore up()
                // DEBUG(dbgSynch, "STable: Find MySemaphore in table[" << i << "]");
                table[i]->signal();
                return 0;
            }
        }
    }
    DEBUG(dbgSynch, "Not exists MySemaphore in signal");
    return -1;
}

int STable::FindFreeSlot()
{
    return (int)this->map->FindAndSet();
}