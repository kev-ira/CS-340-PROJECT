//Kevin Iraheta
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

//Compile using: gcc iraheta.c -o iraheta.exe -pthread

//Summation: Finds the sum of i to j
int sum(int i, int j){
	int sum = 0;
	for (int k = i; k <= j; k++){
		sum += k;
	}
	return sum;
}


//Product: Finds the product of i to j
int product(int i, int j){
	int product = 1;
	for (int k = i; k <= j; k++){
		product *= k;
	}
	return product;
}

//Power: Finds i^j
int power(int i, int j){
	int power = 1;
	for(int k = 0; k < j; k++){
		power *= i;
	}
	return power;
}

//Fibonacci: Finds the jth fibonacci number (input i is ignored)
int fibonacci(int i, int j){
	if(j <= 1)
		return j;
	return fibonacci(i,j-1) + fibonacci(i, j-2);
}

//Function array containing the four operations
int (*funcArr[]) (int, int) = {sum, product, power, fibonacci};

//The mutex is intialized, made so that both the logger
//and scheduler dispatcher don't access the fifo at the same time
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


typedef struct {
//These are all specified in the Project outline	
	int pid;
	int schedule;
	int operation;
	int inp1;
	int inp2;
	int retrieval;
	//This is left at 0 if the schedule isn't priority
	int priority;
	//This is left at 0 if the schedule isn't SJF
	int cpuTime;
//This is not, but is used so that the scheduling functions know if a certain miniPCB was already "processed"
//If processed=0, miniPCB hasn't been processed, if processed=1, it has
	int proccessed;
} miniPCB; 


void dispatch(miniPCB *pcb){
	int x = pcb->inp1;
	int y = pcb->inp2;
	int z = pcb->operation;
	//Dereferencing the function array, invokes it
	int answer = (*funcArr[z])(x,y);
	//This answer is then stored in the miniPCB itself
	pcb->retrieval = answer;
}

//The main thread is responsible for all the initialization required
//Creating the fifo that the scheduler dispatcher and logger use to communicate
miniPCB *fifo[15];
//Creating the ready queue
miniPCB rq[15];

void send(miniPCB* mp, int i){
	//The mutex is used here, so that if the scheduler dispatcher is changing the fifo, the logger cannot access it
	pthread_mutex_lock(&lock);
	fifo[i]= mp;
	pthread_mutex_unlock(&lock);
}

miniPCB* fcfs_scheduler(miniPCB* mp){
		int nextinLine = INT_MAX;
		miniPCB *temp;
		for(int i = 0; i <15; i++){
			//If the miniPCB has been processed already go to the next one
			if(mp->proccessed==1){

			}
			//IF the miniPCB hasn't been processed, check if it has the lowest pid of all unproccessed miniPCBs
			else if (mp->pid < nextinLine){
				nextinLine = mp->pid;
				temp = mp;
			}
			mp++;
		}	
		//Returns the miniPCB* with the lowest pid of the unprocessed miniPCBs
		return temp;
}

miniPCB* sjf_scheduler(miniPCB* mp){
		int shortestBurstTime = INT_MAX;
		miniPCB *temp;
		for(int i = 0; i <15; i++){
			//If the miniPCB has been processed already go to the next one
			if(mp->proccessed==1){

			}
			//If the miniPCB hasn't been processed, check if it has the shortest burst time
			else if (mp->cpuTime < shortestBurstTime){
				shortestBurstTime = mp->cpuTime;
				temp = mp;
			}
			mp++;
		}	
		//Returns the miniPCB* with the lowest burst time of the unprocessed miniPCBs
		return temp;
}

miniPCB* priority_scheduler(miniPCB* mp){
		miniPCB *temp;
		int p = 10;
		int nextinLine = INT_MAX;
		for(int i = 0; i <15; i++){
			//If the miniPCB has been processed already go to the next one
			if(mp->proccessed==1){

			}
			//If the miniPCB hasn't been processed then check if the priority is less
			//the priority of the other unprocessed miniPCB
			else if (mp->priority < p){
					nextinLine = mp->pid;
					p = mp->priority;
					temp = mp;
			}
			//if there are multiple miniPCBs with the same lowest priority, the one with the lower pid gets chosen
			else if (mp->priority == p){
					if(mp->pid< nextinLine){
						nextinLine = mp->pid;
						p = mp->priority;
						temp = mp;
					}
			}
			mp++;
		}	
		//Returns the miniPCB with the lowest priority(if there are multiple, the one with lowest pid is returned)
		return temp;
}

//Schedule array containing the three schedulers
miniPCB* (*schedArr[])(miniPCB*) = {fcfs_scheduler, sjf_scheduler, priority_scheduler};

miniPCB* scheduler(miniPCB* mp){
	//Checks the schedule of the first miniPCB
	int s = mp->schedule;
	//Dereferencing the scheduler array, invokes it
	miniPCB* address = (*schedArr[s])(mp);
	return address;
}

miniPCB* receive(int i){
	//The mutex ensures that if the logger is reading the fifo, it is not modified
	pthread_mutex_lock(&lock);
	miniPCB* temp = fifo[i];
	pthread_mutex_unlock(&lock);
	return temp;
}

//Function ran by the pthread log
//The logger is responsible for the reading of the fifo and writing to the output file
void* logger(void *arg){
	//Accepts pointer passed in from the main thread
	int* f = (int*) arg;
	miniPCB* next;
	char* string;
	char arr[32];
	for(int i = 0; i <15; i++){
		//Reads from the fifo
		next = receive(i);
		if(next->operation == 0){
			string = "sum, ";
		}
		else if(next->operation == 1){
			string = "product, ";
		}
		else if(next->operation == 2){
			string = "power, ";
		}
		else if (next->operation == 3){
			string = "fibonacci, ";
		}
		//Writes to the file
		size_t length = strlen(string);
		write(*f, string, length);
		sprintf(arr, "%d, ", next->inp1);
		string = arr;
		length = strlen(string);
		write(*f, string, length);
		sprintf(arr, "%d, ", next->inp2);
		string = arr;
		length = strlen(string);
		write(*f, string, length); 
		sprintf(arr, "%d \n", next->retrieval);
		string = arr;
		length = strlen(string);
		write(*f, string, length);
	}	
}

//The function run by the pthread sched
//The schedule dispatcher is responsible for reading the ready queue and sending addresses to the fifo
void* sched_disp(void *arg){
	miniPCB *mpptr = (miniPCB *)arg;
	miniPCB *next;
	for(int i = 0; i <15; i++){
		next = scheduler(mpptr);
		dispatch(next);
		send(next, i);
		next->proccessed = 1;
	}
	
}

int main(int argc, char* argv[]){
	//Opening the input file
	FILE *fp;
	fp = fopen(argv[2],"r");
	if(NULL == fp){
		printf("File couldn't be opened \n");
		exit(1);
	}
	char str[30];
	char *token;
	int count = 0;
	//Creating the output file
	int fd;
	fd = open(argv[3], O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if( fd == -1){
    	printf("File couldn't be created \n");
        return 2;
    }

	//Populating the ready queue
	if (strcmp(argv[1], "FCFS") == 0){
		while(fgets(str, 30, fp)!= NULL){
			//Enters the pid of the miniPCB
			token = strtok(str, ",");
			rq[count].pid  = atoi(token); 
			//Enters the operation
			//Changes the string into a number corresponding to the operations spot in the function array
    		token = strtok(NULL, ",");	
    		if(strcmp(token , "sum") == 0){
				rq[count].operation = 0;
			}
			else if(strcmp(token , "product") == 0){
				rq[count].operation = 1;
			}
			else if(strcmp(token , "power") == 0){
				rq[count].operation = 2;
			}
    		else rq[count].operation = 3;
    		//Enters the first input
    		token = strtok(NULL, ",");	
    		rq[count].inp1  = atoi(token);
    		//Enter the second input
    		token = strtok(NULL, ",");	
    		rq[count].inp2  = atoi(token);
    		//Sets the schedule to 0, FCFS spot in the scheduling array
    		rq[count].schedule = 0;	
    		//Sets processed to 0, meaning it has not been processed
    		rq[count].proccessed=0;
    		count++;
		}		
	}
	else if(strcmp(argv[1], "SJF") == 0){
			while(fgets(str, 30, fp)!= NULL){
				//Enters the pid
				token = strtok(str, ",");
				rq[count].pid  = atoi(token);
				//Enters the CPU burst time
				token = strtok(NULL, ",");	
    			rq[count].cpuTime  = atoi(token);
    			//Enters the operation
				//Changes the string into a number corresponding to the operations spot in the function array
    			token = strtok(NULL, ",");	
    			if(strcmp(token , "sum") == 0){
					rq[count].operation = 0;
				}
				else if(strcmp(token , "product") == 0){
				rq[count].operation = 1;
				}
				else if(strcmp(token , "power") == 0){
					rq[count].operation = 2;
				}
    			else rq[count].operation = 3;
    			//Enters the first input
    			token = strtok(NULL, ",");	
    			rq[count].inp1  = atoi(token);
    			//Enters the second input
    			token = strtok(NULL, ",");	
    			rq[count].inp2  = atoi(token);
    			//Sets the schedule to 1, SJF spot in the scheduling array
    			rq[count].schedule = 1;	
    			//Sets processed to 0, meaning it has not been processed
    			rq[count].proccessed=0;
    			count++;
			} 
		}
	else if(strcmp(argv[1], "PRIORITY") == 0){
			while(fgets(str, 30, fp)!= NULL){
				//Enters the pid
				token = strtok(str, ",");
				rq[count].pid  = atoi(token);
				//Enters the priority
				token = strtok(NULL, ",");	
    			rq[count].priority  = atoi(token);
    			//Enters the operation
				//Changes the string into a number corresponding to the operations spot in the function array
    			token = strtok(NULL, ",");	
    			if(strcmp(token , "sum") == 0){
					rq[count].operation = 0;
				}
				else if(strcmp(token , "product") == 0){
				rq[count].operation = 1;
				}
				else if(strcmp(token , "power") == 0){
					rq[count].operation = 2;
				}
    			else rq[count].operation = 3;
    			//Enters the first input
    			token = strtok(NULL, ",");	
    			rq[count].inp1  = atoi(token);
    			//Enters the second input
    			token = strtok(NULL, ",");	
    			rq[count].inp2  = atoi(token);
    			//Sets the schedule to 2, PRIORITY spot in the scheduling array
    			rq[count].schedule = 2;	
    			//Sets processed to 0, meaning it has not been processed
    			rq[count].proccessed=0;
    			count++;
			} 
		}

	fclose(fp);
	
	//Making of the two new threads
	//Passes in the address of ready queue
	pthread_t sched;
	pthread_create(&sched, NULL, sched_disp, &rq);
	
	//Passes in the address of the file descriptor of the output file
	pthread_t log;
	pthread_create(&log, NULL, logger, &fd);

	pthread_join(sched, NULL);
	pthread_join(log, NULL);

	close(fd);
return 0;
}	