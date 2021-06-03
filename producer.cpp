#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <iostream>
#include <fstream>

using namespace std;

int main( int argc, char* argv[] )
{   
    if (argv[1] == NULL)
    {
        perror("No arguments listed");
        return 0;
    }

    string status;

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

    //Textfile accessing
    string textfile = argv[1];
    ifstream message (textfile);
    if (!message.is_open())
    {
        perror("Unable to open file.");
        exit(1);
    } 
    
    string line;
    getline(message,line);
    int i = 0;
    while ( true )
    {
        if (i >= line.length()-1)
        {
            i = 0;
            if (!getline(message,line))
            {
                status = "done";
                break;
            }            
        }

        while (shmOccSpace < shmSize)
        {
            //one character is approx one byte in std::string
            if (line.length() < shmSize - shmOccSpace)
            {//whole line can fit in shm
                const char* buffer = line.c_str();
                strcpy( sharedMem,  buffer);
                shmOccSpace += line.length();
                if (!getline(message,line))
                {
                    status = "done";
                    break;
                }  
            }
            else
            {//loop until shm is filled                
                for (shmOccSpace; shmOccSpace < shmSize; shmOccSpace++)
                {
                    string letter(1, line[i]);
                    const char* buffer = letter.c_str();
                    strcpy( sharedMem,  buffer);
                    i++;
                }
            }
        }

        shmOccSpace = 0;

        // -- Semaphore Releasing --
        nOperations = 1;

        sema[0].sem_num = 0;
        sema[0].sem_op = -1;
        sema[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

        opResult = semop( semId, sema, nOperations );

        if( opResult == -1 )
        {
            perror( "semop (decrement)" );
        }

        status = "written";
        //while shared mem B is in state II, wait
    }

    message.close();
       
    return 0;
}

//https://www.cplusplus.com/doc/tutorial/files/
//https://stackoverflow.com/questions/7743356/length-of-a-c-stdstring-in-bytes
//https://stackoverflow.com/questions/347949/how-to-convert-a-stdstring-to-const-char-or-char
//https://stackoverflow.com/questions/8437099/c-convert-char-to-const-char