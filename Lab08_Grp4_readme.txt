CSCI 51.02A-Q4 OPERATING SYSTEMS, LABORATORY
ARCEO, LI NIKO
CO, LANCE MICHAEL O.
Lab #08 - IPC (Local)

description: Local IPC using Semaphores and shared memory

how to run program:
1. download Lab8_Grp4_codeProducer.cpp and Lab8_Grp4_codeConsumer.cpp
2. compile both files with compiler of choice
3. create a sample text file for input
4. run the producer program in terminal using ./[ProgramName] [textfile.txt] [shared memory size]
5. run the consumer program in terminal using ./[ProgramName] [outputtextfile.txt] [shared memory size]

Further Notes:
Once initialized, the shared memory size cannot be altered as the shared memory is not deallocated, leading to a shmat error.
