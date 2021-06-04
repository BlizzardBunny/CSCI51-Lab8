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
#include <unistd.h>

using namespace std;

//Program's flow should be:
//1. Initialize the shared resources
//2. Do the entry part of the program (in this case, open the text file and get the content)
//3. Gain control of the semaphore
//4. Do the program's critical section (access and write to shared memory)
//5. Release Semaphore

int main( int argc, char* argv[] )
{
    string input;
    
    //Step 1
    //Semaphore creation
    int semId;
    key_t semKey = 1234;
    int nSems = 1;
    int semFlag = IPC_CREAT | 0666;    
    
    semId = semget( semKey, nSems, semFlag );
    if( semId == -1 )
    {
        perror( "semget" );
        exit(1);
    }
    
    //shared memory creation - A
    // Id for the shared memory
    int shmId;
    int shmIdB;
    
    // 1 key = 1 shared memory segment
    // Think of a map (the data structure)
    key_t shmKey = 12341234;
    key_t shmKeyB = 32432432;
    
    // Size of the shared memory in bytes.
    // Preferably a power of 2
    // This line of code assigns the size to be
    // 1024 bytes or 1KB
    int shmSize = atoi(argv[2]);    
    int shmSizeB = 8;
    
    // Flags + permissions when creating the shared
    // memory segment.
    // IPC_CREAT - If the shared memory does not exist yet, automatically create it
    // 0666 - Remember chmod? The 0 in front indicates that the number is expressed in octal.
    int shmFlags = IPC_CREAT | 0666;
    int shmFlagsB = IPC_CREAT | 0666;

    // Pointer for the starting address of the shared memory segment.
    char* sharedMem;
    char* sharedMemB;

    // Yes, this is almost the same as semget()
    shmId = shmget( shmKey, shmSize, shmFlags );
    shmIdB = shmget( shmKeyB, shmSizeB, shmFlagsB );
    
    // shmat() returns the starting address of the shared memory
    // segment, so we assign it to sharedMem.
    sharedMem = (char*)shmat( shmId, NULL, 0 );
    sharedMemB = (char*)shmat( shmIdB, NULL, 0 );

    //Step 2
    //Entry Section of the program
    if (argv[1] == NULL)
    {
        perror("No arguments listed");
        return 0;
    }
    string filename = argv[1];
    vector <string> fileContent;
    string line;
    ifstream myfile(filename);
    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            fileContent.push_back(line);
        }
        myfile.close();
    }

    else cout << "Unable to open file";
    
    //loop that moves the file content into a string
    for (int i = 0; i <= int(fileContent.size())-1; i++)
    {
        input += fileContent[i];
    }
    
    int i = 0;
    while (true)
    {      
        
        //Step 3
        //Gain control of semaphore

        int nOperations = 2;
        struct sembuf sema[nOperations];

        sema[0].sem_num = 0; 
        sema[0].sem_op = 0; 
        sema[0].sem_flg = SEM_UNDO; 

        sema[1].sem_num = 0;
        sema[1].sem_op = 1;
        sema[1].sem_flg = SEM_UNDO | IPC_NOWAIT; 

        int opResult = semop( semId, sema, nOperations );    

        if( opResult == -1 )
        {
            perror( "semop (increment)" );
            exit(1);
        }
        else
        {            
            //Step 4
            //Write to shared memory
            if(( ((int*)sharedMem) == (int*)-1 ) || ( ((int*)sharedMemB) == (int*)-1 ))
            {
                perror( "shmop: shmat failed" );
                exit(1);
            }
            else
            {                    
                const char* bufferB = "w"; //status: written
                strcpy(sharedMemB, bufferB);
                
                int maxShm = i + shmSize;
                for (i; ((i < input.length()) && (i < maxShm)); i++)
                {
                    string line(input, i);
                    const char* buffer = line.c_str();
                    // We can now write to shared memory
                    strcpy( sharedMem, buffer );
                }                
                
                cout << "successfully wrote to shared memory"<< endl;
                cout << "i: " << i << endl;
                cout << "shmSize: " << shmSize << endl;
            }

            if (i >= input.length())
            {                
                const char* bufferB = "d"; //status: done
                strcpy(sharedMemB, bufferB);
                return 0;
            }
            
            //Step 5
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

        while (sharedMemB != "*")
        {
            sleep(1);
        }
    }

    return 0;
}

//https://www.cplusplus.com/doc/tutorial/files/
//https://stackoverflow.com/questions/7743356/length-of-a-c-stdstring-in-bytes
//https://stackoverflow.com/questions/347949/how-to-convert-a-stdstring-to-const-char-or-char
//https://stackoverflow.com/questions/8437099/c-convert-char-to-const-char
//https://stackoverflow.com/questions/7352099/stdstring-to-char/7352131
//https://www.softwaretestinghelp.com/cpp-sleep/
