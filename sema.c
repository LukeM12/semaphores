/*
 * Author : Luke Morrison
 * Contact : luc785@hotmail.com
 * Date Created : Oct 4th 2014 
 * Last updated : Oct 4th 2014
 * Description - Barber Shop problem with Semaphores and processes
 * The Customer Generator is always running
 * When customers come in, it dispatches the customer manager thread
 * Which will 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#define BARBER 208
#define CLIENTS 209
#define MAX_SEATS 10
#define MAX_WALKIN_LENGTH 5
#if !defined(_SEM_SEMUN_UNDEFINED) || _SEM_SEMUN_UNDEFINED
/*Unions*/
/* according to X/OPEN we have to define it ourselves */
union semun {
	int val; /* value for SETVAL */
	struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
	unsigned short int *array; /* array for GETALL, SETALL */
	struct seminfo *__buf; /* buffer for IPC_INFO */
};
#endif

/* Structures */
typedef struct {
	int working;	
} BarberShop ;

typedef struct {
	int clients[MAX_SEATS];
	int seats_occupied;
	int walkins;
} Clients; 

/*Processes*/
void p(void);
void v(void);
void WelcomeUser(void);
/*Customer Generation */
Clients* GenerateCustomers(int);
int BarberSleeps(int);

/*Make the generic sem value */
int make_sem(int);

/*Initialize the sem*/
void init_semVal(int);

/*Create a shared memory context. int is mode*/
int grab_shm(int);

/*Attach a shared memory context to this process*/
/*Our lock*/
sem_t *lock1;
char n[1024];
sem_t len;

/*attach schm, first parameter is the shmid, second is the mode*/
void *attach_shm(int, int);

/*The master key numbers*/
int BarberKey= 2;
int ClientKey = 3;
int LockKey = 32; //unused
sem_t *lock1;

/*Our Main Process*/
int main (void){

	/*Get some semaphore keys for shared memory */
	int BarberKey = make_sem(BARBER);
	int ClientKey = make_sem(CLIENTS);
	int Barber = grab_shm(BARBER);

	Clients *customers;
	BarberShop* barberShop;

    WelcomeUser();
	lock1 = sem_open("pSema", O_CREAT | O_EXCL, 0644, 1);
	if( lock1 == SEM_FAILED){
		printf("Semaphore initialization Failed!\n");
		exit(1);
	}
	sem_unlink("pSema");
	/*Customer Generator Process */
	int CustomerGenerate = fork();
	if (CustomerGenerate == 0){
		p();
		while (1){
			
			customers = GenerateCustomers(MAX_WALKIN_LENGTH); //10 is the max amount of time we can wait
			v();//Wake up the customer manager process
		}

	}
	/*Customer Manager Process*/
	int CustomerManager = fork();
	if (CustomerManager == 0){
		sleep(1);
		/*This thread waites until it is dispatched*/
		p(); //lock
		printf("I am unlocked!!");
		int shmid = grab_shm(CLIENTS);
		//Grab our contexts 
		customers = (Clients*) attach_shm(shmid, CLIENTS);
		barberShop = (BarberShop*) attach_shm(Barber, BARBER);
		while(1){
			//The customer behavior. This thread only wakes up when it is called, 
			if (customers->walkins > 0){
				if ( (customers->walkins + customers->seats_occupied) > MAX_SEATS ){
					int walkouts = (customers->seats_occupied + customers->walkins) - MAX_SEATS;
					printf("%d people walked out\n", walkouts);
					customers->seats_occupied = MAX_SEATS;

				}
				else {
					//printf("There are %d new customers sitting in the waiting room\n", customers->walkins);
					customers->seats_occupied += customers->walkins;
				} 
				//Deal with the barber 
				if ( barberShop->working == 0 ) {
					printf("Customer Manager wakes up barber");
					v(); //wake up barber. He can do the rest
					printf("Customer Manager going to sleep");
					sleep(1);
					p(); // go to sleep
				}
				else {
					printf("Cusotmer Manager going to sleep");
					p(); //go back to sleep
				}					
			}
		
		}
	}
	/*Barber Process*/
	int BarberWork = fork();
	if (BarberWork == 0){

		int shmid = grab_shm(CLIENTS);
		customers = (Clients*) attach_shm(shmid, CLIENTS);
		/*Barber shm*/
	    BarberShop *barberShop;
	   	barberShop = (BarberShop*) attach_shm(Barber, BARBER); //Attach the shared memory context
	    while(1) {
	    	/*This thread waites until it is dispatched*/
	    	sleep(2);
	    	p(); 
	    	/*If there are no customers, Barber goes to sleep*/
			while (1){
				if (customers->seats_occupied == NULL || customers->seats_occupied <= 0){
					printf("No customers are here, Barber is going to sleep\n");
					barberShop->working = 0;
					p();
				}
				else if (barberShop->working == 0){
					srand(time(NULL));
					int max = 3;
					int r = rand() % max;
			    	printf("Barber wakes up, and gets the next customer from the waiting room!\n Barber starts cutting hair for %d seconds\n", r);
			    	sleep(r);
			    	printf("Barber finished and goes to get another customer from the waiting room\n");
			    	customers->seats_occupied--;
					barberShop->working = 1;
			    }
			    else {
			    	srand(time(NULL));
					int max = 3;
					int r = 2 + rand() % max;
			    	printf("Barber gets Customer.\n Barber Starts Cutting Hair\n");
			    	sleep(r);
			    	customers->seats_occupied--;
			    	printf("Barber finished and goes to get another customer from the waiting room\n");
			    	printf("There are %d customers left\n", customers->seats_occupied);	 
		    		barberShop->working = 1;
			    }
		    }
	    }
	}
	printf("PRESS Q AT ANY TIME TO EXIT\n");
	char a = 'v';
	while( a != 'Q') {
		a = getchar();
	}
	printf("Program Exited Come Back to the Barber Shop Soon Now Ya hear?");
	sleep(1);
	kill(0, SIGKILL);

	exit(1);
}
/*********************************************************************/
/*****************Kernal Shared Memory*******************/
/*
* function : make_sem
* description : Creates a semaphore chunk
* return : ID of the Semaphore
*/
int make_sem(int mode){
	int semaphore;
	if (mode == BARBER){
		semaphore = semget(BarberKey, 1, IPC_CREAT | 0600);
		if (semaphore == -1) {
			// error occured so print an error message and exit 
			printf("error : semget");
			return -1;
		}
		return semaphore;
	}
	if (mode == CLIENTS){
		semaphore = semget(ClientKey, 1, IPC_CREAT | 0600);
		if (semaphore == -1) {
			// error occured so print an error message and exit 
			printf("error : semget");
			return -1;
		}
	}
	return semaphore;
}
/*
* function : init_sem
* description : Treat a semaphore as a binary semaphore (as done above) and just set it to a value 
* @param : ID of the Semaphore
* @return : ID of the Semaphore
*/
void init_semVal(int val){
	int rc; /* return code */
	union semun sem_val;
	/* Initialize the value to the input */
	sem_val.val = val;
	rc = semctl(BarberKey, 0, SETVAL, sem_val);
	if (rc == -1) {
		/* error occured */
		perror("semctl");
		exit(1);
	}
}
/*
* function : grab_shm
* description : returns a semaphore ID
* @param : ID of the Semaphore
* return : ID of the memory space
*/
int grab_shm(int mode ){
	int shmid;
	if (mode == BARBER){
		shmid = shmget(BarberKey, sizeof(BarberShop), IPC_CREAT | 0600);
		if (shmid == -1) {
			printf("Barbers\n");
			perror("shmget");
		}	
	}
	if (mode == CLIENTS){
		shmid = shmget(ClientKey, sizeof(Clients), IPC_CREAT | 0600);
		if (shmid == -1) {
			printf("Clients\n");
			perror("shmget");
		}	
	}
	return shmid;
}
/*
* function : attach_schm
* description : returns a semaphore ID
* @param : ID of the Semaphore
* @param : mode. This will determine which data structure it will create. 
* return
*/
void *attach_shm(int shmid, int mode){
	void *ret;
	if (mode == BARBER){
		ret = (BarberShop*) shmat(shmid, NULL, 0);
		if (ret == NULL) { 
			perror("shmat");
			exit(1);
		}
		return ret;
	}
	if (mode == CLIENTS){
		ret = (Clients*) shmat(shmid, NULL, 0);
		if (ret == NULL) {
			perror("shmat");
			exit(1);
		}
		return ret;
	}
}

/*****************Process Signalling*******************/
/*
* function : p()
* description : Decrements the lock counter by 1
*/
void p(){
	sem_wait(lock1);
}
/*
* function : v()
* description : Increments the lock counter by 1
*/
void v(){
	sem_post(lock1);
}

/*****************Randome Customers*******************/
/*
* function : GenerateCustomers
* description : GenerateCustomers
* @param : int max
* return : Client Shared Memory Chunk
*/
Clients *GenerateCustomers(int max){
	srand(time(NULL));
	int r = 1 + rand() % max;
	int sleeper = 8 + rand() % max;
	sleep(sleeper + 2);
	printf("%d Customers just walked in\n", r);
	int shmid = grab_shm(CLIENTS);
	Clients *ret = (Clients*) attach_shm(shmid, CLIENTS);
	
	ret->walkins = r;
	return ret;
}


/*****************Sleeping Barber*******************/
/*
* function : GenerateCustomers
* description : 
* @param : int max
* return :
*/
int BarberSleeps(int max){
	srand(time(NULL));
	int r = rand() % max;
	printf("Barber now sleeping for %d seconds\n", r);
	sleep(r);
	return r;
}
/****************WelcomeUser******************/
/*
* function : WelcomeUser()
* description : A brief hello message to tell the user what is going on in the program
*/

void WelcomeUser(void){
    printf("Welcome to the Barber Shop\n");
    printf("Shhhh! The barber is sleeping. The programs are waiting for the customers to come in\n");
}

