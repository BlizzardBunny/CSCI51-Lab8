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

int main( int argc, char* argv[] )
{
    //file reading moved to the start
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


    
    string status;
    string input;

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

    //shared memory creation - A
    int shmId;
    key_t shmKey = 6969;
    int shmSize = atoi(argv[2]);
    int shmOccSpace = 0; //occupied space
    int shmFlags = IPC_CREAT | 0666;
    char* sharedMem;

    shmId = shmget( shmKey, shmSize, shmFlags ); 
    sharedMem = (char*)shmat( shmId, NULL, 0 );

    if( ((int*)sharedMem) == (int*)-1 )
    {
        perror( "shmop: shmat failed" );
        exit(1);
    }
    else
    {
        //loop that puts the content of the fileContent vector into the input string
        for (int i = 0; i <= int(fileContent.size())-1; i++)
        {
            input += fileContent[i];
        }
            const char* buffer = input.c_str();
            // We can now write to shared memory...
            strcpy( sharedMem, buffer );

            char buffer2[50];

            // Or read from shared memory.
            strcpy( buffer2, sharedMem );

            printf( "%s\n", buffer2 );
        
    }

    return 0;
}

//https://www.cplusplus.com/doc/tutorial/files/
//https://stackoverflow.com/questions/7743356/length-of-a-c-stdstring-in-bytes
//https://stackoverflow.com/questions/347949/how-to-convert-a-stdstring-to-const-char-or-char
//https://stackoverflow.com/questions/8437099/c-convert-char-to-const-char
//https://stackoverflow.com/questions/7352099/stdstring-to-char/7352131

