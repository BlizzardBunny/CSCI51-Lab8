#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

//Program's flow should be:
//1. Find the shared resources
//2. Gain control of the semaphore
//3. Do the program's critical section (access and read the shared memory)
//4. Release Semaphore

int main (int argc, char* argv[] )
{
    vector<string> pacertest;
    string filename = argv[1];
    ofstream outputFile(filename);
    //Step 1, find the shared resources
    // -- Semaphore Initialization --

    // Semaphore set ID
    int semId;

    // 1 key = 1 semaphore set (or array)
    // Think of it like a map (the data structure)
    // Put a large constant here, e.g. 1234
    key_t semKey = 1234;

    // Flags to use upon getting the semaphore set.
    // Normally when dealing with flags, you usually bitwise-OR
    // flags together.
    // IPC_CREAT - If no existing semaphore set is associated with the key, create one
    // 0666 - Remember chmod? The 0 in front specifies that the number is in octal
    int semFlag = IPC_CREAT | 0666;

    // Number of semaphores in the semaphore set
    // For this example, we'll just create 1 semaphore set
    int nSems = 0;

    // Attempt to get a semaphore set
    // Returns -1 if there was an error.
    semId = semget( semKey, nSems, semFlag );
    if( semId == -1 )
    {
        perror( "semget" );
        exit( 1 );
    }
    
    // -- Shared Memory Initialization --
    // Id for the shared memory
    int shmId;

    // 1 key = 1 shared memory segment
    // Think of a map (the data structure)
    key_t shmKey = 12341234;

    // Size of the shared memory in bytes.
    // Preferably a power of 2
    // This line of code assigns the size to be
    // 1024 bytes or 1KB
    int shmSize = atoi(argv[2]);

    // Flags + permissions when creating the shared
    // memory segment.
    // IPC_CREAT - If the shared memory does not exist yet, automatically create it
    // 0666 - Remember chmod? The 0 in front indicates that the number is expressed in octal.
    int shmFlags = IPC_CREAT | 0666;

    // Pointer for the starting address of the shared memory segment.
    char* sharedMem;

    // Yes, this is almost the same as semget()
    shmId = shmget( shmKey, shmSize, shmFlags );

    // shmat() returns the starting address of the shared memory
    // segment, so we assign it to sharedMem.
    sharedMem = (char*)shmat( shmId, NULL, 0 );
    
    //Step 2
    //Gain control of semaphore
    // -- Semaphore Accessing --

    // Perform 2 operations
    int nOperations = 2;

    // Create an array of size 2 to hold
    // the operations that we will do on the semaphore set
    struct sembuf sema[nOperations];

    // Definition for the first operation
    // Our first operation will be to wait for the
    // semaphore to become 0
    sema[0].sem_num = 0; // Use the first semaphore in the semaphore set
    sema[0].sem_op = 1; // Wait if semaphore != 0
    sema[0].sem_flg = SEM_UNDO; // See slides

    sema[1].sem_num = 0; // Use the first semaphore in the semaphore set
    sema[1].sem_op = 1; // Increment semaphore by 1
    sema[1].sem_flg = SEM_UNDO | IPC_NOWAIT; // See slides

    // Perform the operations that we defined.
    // Will return -1 on error.
    int opResult = semop( semId, sema, nOperations );

    // If we successfully incremented the semaphore,
    // we can now do stuff.
    if( opResult != -1 )
    {
        printf( "Successfully incremented semaphore!\n" );

        // Step 3
        // Access the shared memory
        
        if( ((int*)sharedMem) == (int*)-1 )
        {
            perror( "shmop: shmat failed" );
        }
        else
        {
            //TODO: made it pass via chunks instead of 1 big buffer

            char buffer[999];
            strcpy( buffer, sharedMem );
            outputFile << buffer;
            printf( "%s\n", buffer );
            //outputFile << buffer << endl;
        }
        
        //Step 4
        // -- Semaphore Releasing --

        // Set number of operations to 1
        nOperations = 1;

        // Modify the first operation such that it
        // now decrements the semaphore.
        sema[0].sem_num = 0; // Use the first semaphore in the semaphore set
        sema[0].sem_op = -1; // Decrement semaphore by 1
        sema[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

        opResult = semop( semId, sema, nOperations );
        if( opResult == -1 )
        {
            perror( "semop (decrement)" );
        }
        else
        {
            printf( "Successfully decremented semaphore!\n" );
        }
    }

}
