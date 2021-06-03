#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <cstdio>
#include <cstring>

using namespace std;

int main( int argc, char* argv[] )
{
    // Id for the shared memory
    int shmId;

    // 1 key = 1 shared memory segment
    // Think of a map (the data structure)
    key_t shmKey = 1234;

    // Size of the shared memory in bytes.
    // Preferably a power of 2
    // This line of code assigns the size to be
    // 1024 bytes or 1KB
    int shmSize = 1 << 10;

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

    if( ((int*)sharedMem) == (int*)-1 )
    {
        perror( "shmop: shmat failed" );
    }
    else
    {
        const char* buffer = "Hello!";

        // We can now write to shared memory...
        strcpy( sharedMem, buffer );

        char buffer2[50];

        // Or read from shared memory.
        strcpy( buffer2, sharedMem );

        printf( "%s\n", buffer2 );

        char buffer3[50];
        
        // We can also use memcpy
        memcpy( buffer3, sharedMem, 3 );
        buffer3[3] = '\0';

        printf( "%s\n", buffer3 );
    }

    return 0;
}

