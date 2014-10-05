/*
 * Author : Luke Morrison
 * Contact : luc785@hotmail.com
 * Date Created : Oct 4th 2014 
 * Last updated : Oct 4th 2014
 * Description - Barber Shop problem with Semaphores and processes
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
#define BARBER 10 
#define CLIENTS 11
#define NUMBER_OF_FREE_SEATS 10
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
	int clients[NUMBER_OF_FREE_SEATS];
	int seats_left;
} Clients; 

/*Processes*/
void p(void);
void v(void);

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
int BarberKey= 10;
int ClientKey = 11;
int LockKey = 12; //unused
sem_t *lock1;

/*Our Main Process*/
int main (void){
	/*We can contain 1 thread */
	//sem_init(&lock, LockKey, 1);

	/*Configure the semaphore to use it as a mutex */
	lock1 = sem_open("pSem", O_CREAT , 0644, 1);
	sem_unlink("pSem");

	/*Get some semaphore keys for shared memory */
	int BarberKey = make_sem(BARBER);
	int ClientKey = make_sem(CLIENTS);

	/*This could be a shared memory counter - not used */
	init_semVal(1); //Semaphore with 2 initial values

	int Barber = grab_shm(BARBER);
	Clients *customers;

	int rc;
	int BarberWork = fork();
    if (BarberWork == 0){
    	/*Barber Process*/
    	int shmid = grab_shm(CLIENTS);
		customers = (Clients*) attach_shm(shmid, CLIENTS);
	    while(1) {
	    	BarberShop *barberShop;
	    	barberShop = (BarberShop*) attach_shm(Barber, BARBER); //Attach the shared memory context
	    	
	    	/*This thread waites until it is dispatched*/
	    	p(); 
	    	/*If there are no customers, Barber goes to sleep*/
			
			if (customers->seats_left == NULL || customers->seats_left <= 0){
				printf("No customers are here, Barber is going to sleep\n");
				barberShop->working = 0;
				p();
				//On wakes up if a client walks in 
			}
			srand(time(NULL));
			int max = 5;
			int r = rand() % max;
	    	printf("Barber is awake\n");
	    	printf("Barber cutting hair for %d seconds\n", r);
	    	sleep(r);
	    	customers->seats_left--;

 
	    	barberShop->working = 1;
	    	v(); //Release the process and allow for more seating

	    }
    }

    else {
 
    	int CustomerGenerate = fork();
    	if (CustomerGenerate == 0){
    		p();

    		while (1){
    			customer = GenerateCustomers(10); //10 is the max amount of time we can wait
    			if ()
    			printf("Seats left = %d\n", customers->seats_left);
    		}

    	}
    	else{
    		//Seating the customers 
    		p();
    		sleep(1);
    		int shmid = grab_shm(CLIENTS);
			customers = (Clients*) attach_shm(shmid, CLIENTS);
			barberShop = (BarberShop*) attach_shm(Barber, BARBER);

    		p();
    		while (1){
    			printf("saaa");
    		}



    	}

	}
	return 0;
}

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
* description : returns a semaphore ID
* @param : ID of the Semaphore
* @return : ID of the Semaphore
*/
void init_semVal(int val){
	int rc; /* return code */
	union semun sem_val;
	/* Initialize the value to the input */
	sem_val.val = val;
	rc = 1;//semctl(MasterKey, 0, SETVAL, sem_val);
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
* function : init_sem
* description : returns a semaphore ID
* @param : ID of the Semaphore
* @param : mode. This will determine which data structure it will create. 
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
* function : init_sem
* description : returns a semaphore ID
* @param : ID of the Semaphore
* @param : ID of the Semaphore
*/
void p(){
	sem_wait(lock1);
}
/*
* function : GenerateCustomers
* description : 
* @param : int max
* return :
*/
void v(){
	sem_post(lock1);
}

/*****************Randome Customers*******************/
/*
* function : GenerateCustomers
* description : GenerateCustomers
* @param : int max
* return :
*/
Clients *GenerateCustomers(int max){
	srand(time(NULL));
	//int r = rand() % max;;
	int r = rand() % max;
	sleep(r);
	//Let's grab some shared memory for the customers
	printf("%d Customers just walked in\n", r);
	int shmid = grab_shm(CLIENTS);
	Clients *ret = (Clients*) attach_shm(shmid, CLIENTS);
	ret->seats_left += r;
	printf("There are %d customers left", ret->seats_left);
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
	printf("Barber now sleeping for %d seconds", r);
	sleep(r);
	return r;
}


/****************SeatChair******************/
/*
* function : GenerateCustomers
* description : 
* @param : int max
* return :
*/
int SeatChair(int max){
	srand(time(NULL));
	int r = rand() % max;
	return r;
}


