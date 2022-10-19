# CS-340-PROJECT
A project I made for my CS 340 class on operating systems. This project is made in C.

The objective of this project was to practice making multi-threaded programs. In this, you give an input file, and then it is tokenized into different
blocks, called "mini pcb" in the code. These operations are then performed in an ordered determined by the CPU scheduling algorithm you give it. The different threads
include the main thread, which initialized all the data structures that will be used by the other threads, the scheduler, which fills the structures with data from 
the input file, and finally the logger thread, which takes the data from the structures and puts them in the output file. 

The files, FCFS, SJF, and PRIORITY, are examples of input the program would accept, and the out files show the final output.

This program takes command line arguments. To have it run, put
iraheta.exe (CPU Scheduling algorithm) (Name of input file) (Name of output file)
ex: iraheta.exe FCFS FCFS.txt FCFSout.txt
