/* This deals with the barber */
		p(); //lock
	int shmid = grab_shm(CLIENTS);

	//Grab our contexts 
	customers = (Clients*) attach_shm(shmid, CLIENTS);
	barberShop = (BarberShop*) attach_shm(Barber, BARBER);
	int BarberWork = fork();
	if (BarberWork == 0){
		/*Barber Process*/
		int shmid = grab_shm(CLIENTS);
		customers = (Clients*) attach_shm(shmid, CLIENTS);
		/*Barber shm*/
	    BarberShop *barberShop;
	   	barberShop = (BarberShop*) attach_shm(Barber, BARBER); //Attach the shared memory context
	    while(1) {

	    	/*This thread waites until it is dispatched*/
	    	p(); 
	    	/*If there are no customers, Barber goes to sleep*/
			while (customers->seats_occupied > 0){

				if (customers->seats_occupied == NULL || customers->seats_occupied <= 0){
					printf("No customers are here, Barber is going to sleep\n");
					barberShop->working = 0;
					p();
				}
				else if (barberShop->working == 0){
					srand(time(NULL));
					int max = 5;
					int r = rand() % max;
			    	printf("Barber wakes up!\n");
			    	printf("Barber cutting hair for %d seconds\n", r);
			    	sleep(r);
			    	printf("Barber finished and goes to get another customer from the waiting room\n");
			    	printf("There are %d customers left\n", customers->seats_occupied);	 
			    	customers->seats_left--;
					barberShop->working = 1;
			    }
			    else {
			    	srand(time(NULL));
					int max = 5;
					int r = rand() % max;
			    	printf("Barber Starts Cutting Hair\n");
			    	sleep(r);
			    	printf("Barber finished and goes to get another customer from the waiting room\n");
			    	printf("There are %d customers left\n", customers->seats_occupied);	 
		    		barberShop->working = 1;
			    }

		    }
	    }
	}