/*

Problem 1 Life on a Chip

In future, water will be a scarce resource and most of the humans will have to generate water
on their own.
For this purpose, a chip was developed that will combine Hydrogen and Oxygen to make
water. This chip
has n sites where the reaction will take place, and each reaction will generate 1 E Mj of
energy. For safety
purpose, no two consecutive sites should be used for the reaction, and total energy generation
at any moment
should not exceed than some threshold value. As for reaction, each hydrogen atom must
combine with another hydrogen atom and an oxygen atom at a site.
For the purpose of this problem, each atom can be assumed to be a thread, which will be in
sleep state and at every state of reaction you have to wake up the corresponding threads
(atom) at particular site, and join them. Assume that a reaction take sleep (3) time to
complete.

Your task is to write an algorithm using semaphores, which efficiently uses atoms, and gives
a faster way to generate water.

Input:
./a.out <No of H atoms> <No of O atoms> <No of sites> <Threshold Energy(in  Units)>
./a.out 12 6 3 2

Output:
Write lines, such that ith line represents a reaction giving the details of site and total
energy generated. Each line also specifies number of H and O atoms left (indexing
from 1 to m).

*/

/* HEADER FILES */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

/* GLOBAL VARIABLES */
void *thread_site(void *arg3);
int *site_arr;
int *thresh;
int *sites;
int *hydrogens;
int *oxygens;
int exitCase = 0;
/* SEMAPHORE VARIABLES*/
sem_t h_sem;
sem_t o_sem;
sem_t th_sem;
sem_t s_sem;
sem_t *site_sem;
sem_t energy_sem;
sem_t mutex;
//sem_t mutex1;

int main(int argc, char *argv[]){
	//Improper Number of Arguments
	if(argc != 5){
		printf("Usage: <Executable> <No. of H atoms> <No. of O atoms> <No. of sites> <Threhold Energy(in units)>\n");
		exit(1);
	}
		
	int res, i, j, k, th, l, th_e, p, s_init;
	/* INITIALIZING THE ARGUMENTS */
	int hCount = atoi(argv[1]);
	int oCount = atoi(argv[2]);
	int sCount = atoi(argv[3]);
	th_e = atoi(argv[4]);
	/* THREAD FOR ALL THE SITES */
	pthread_t site[sCount];
	/* SEMAPHORE ARRAY */
	site_sem = (sem_t*)malloc(sizeof(sem_t) * sCount );
	/* ARRAY TO MAINTAIN THE CONDITION */
	site_arr = (int*)malloc(sizeof(int) * sCount);
	
	for(i = 0 ; i < sCount ;i++){
		site_arr[i] = 0;
	}
	//srand (time(NULL));
	
	/* INITIALIZE THE INPUTS*/
	sites = &sCount;
	hydrogens = &hCount;
	oxygens = &oCount;
	thresh = &th_e;
	void *thread_result;
	/* USED FOR THE CONDITION*/
	if(sCount == 1){
		s_init = 1;
	}else {
		s_init = sCount / 2;
	}
	/* INITIALIZATION OF SEMAPHORES */
	/* SEMAPHORE FOR NUMBER FOR HYDROGEN ATOMS INITIALIZED TO NUMBER OF ATOMS*/
	res = sem_init(&h_sem , 0 , hCount);	
	if(res != 0){
		perror("semaphore initialization failed");
		exit(1);
	}
	/* SEMAPHORE FOR NUMBER FOR OXYGEN ATOMS INITIALIZED TO NUMBER OF ATOMS*/
	res = sem_init(&o_sem , 0 , oCount);
	if(res != 0){
                perror("semaphore initialization failed");
                exit(1);
        }
	/* SEMAPHORE FOR THRESHOLD ENERGY */
	res = sem_init(&th_sem , 0 ,th_e);
	if(res != 0){
                perror("semaphore initialization failed");
                exit(1);
        }
	/* SEMAPHORE INITIALIZED TO HALF OF THE SITES */
	res = sem_init(&s_sem , 0 ,s_init);
	if(res != 0){
                perror("semaphore initialization failed");
                exit(1);
        }
	/* SEMAPHORE FOR THE ENERGY GENERATED */
	res = sem_init(&energy_sem , 0 , 0);
	if(res != 0){
                perror("semaphore initialization failed");
                exit(1);
        }
	/* SEMAPHORE FOR THE MUTEX*/
        res = sem_init(&mutex , 0 , 1);
        if(res != 0){
                perror("semaphore initialization failed");
                exit(1);
        }

	/* INITAILIZING SEMAPHORE FOR THE INDIVIDUAL SITES*/
	for(l = 0 ; l < sCount; l++){
		res = sem_init(&(site_sem[l]), 0, 0);
		if(res != 0){
                	perror("semaphore initialization failed");
                exit(1);
        	}
	}
	
	/* SITES THREAD CREATED */
	for(k = 0 ; k < sCount ;k++){
                  res = pthread_create(&(site[k]), NULL, thread_site, (void *) k);
	}
	/* LOGIC FOR THE CHECKING OF PARTICULAR SITES AND SIGNAL IT*/
	//srand (time(NULL));
	int ra;
	while(*hydrogens > 1 && *oxygens > 0){
		/* RANDOMLY SITES ARE GENERATED*/
		ra = rand() % sCount;						
		//srand (1);
		if(ra != sCount -1 && ra != 0){
			if(site_arr[ra - 1] == 0 && site_arr[ra + 1] == 0 && site_arr[ra] == 0){
				sem_post(&site_sem[ra]);
			}	
		}else if(ra == sCount -1){
			 if(site_arr[ra - 1] == 0 && site_arr[0] == 0 && site_arr[ra] == 0){
				sem_post(&site_sem[ra]);
                        }
		}else if(ra == 0){
			 if(site_arr[sCount -1] == 0 && site_arr[ra + 1] == 0 && site_arr[ra] == 0){
				sem_post(&site_sem[ra]);
                        }
		}
	}		
	
	/* ENSURES THAT MAIN IS WAITING FOR THE SITE THREADS TO COMPLETE*/
	for(p = 0 ; p < sCount ;p++){
		pthread_join(site[p], NULL);
	}
	/* EXECUTION COMPLETED*/
	printf("Final left hydrogen : %d, oxygen : %d\n",*hydrogens, *oxygens);
	printf("ALL DONE\n");
	
return 0;
}

void *thread_site(void *arg3){
	//sem_wait(&mutex);
	int siteid  = (int *)arg3;	
	int i;
	int counter = 0;
	int hydro = 0 , oxy = 0;
	//printf("test");
	while( *hydrogens > 1 && *oxygens > 0 ){
		/* INITIALIZES THE SITES TO WAIT */	
		sem_wait(&site_sem[siteid]);
		/* CHECK FOR THE NUMBER OF THREADS TO PASS SIMULTANEOUSY AT MAX*/
		if(*sites >= 2 * (*thresh)){
			sem_wait(&th_sem);
		}else{
			sem_wait(&s_sem);
		}
		if (*hydrogens > 1 && *oxygens > 0) {
			/* WAIT DECREASE THE THE HYDROGEN SEM BY 2 AND OXYGEN SEM BY 1*/
			sem_wait(&h_sem);
                        sem_wait(&h_sem);
                        sem_wait(&o_sem);
			/* PUTS THE NUMBER 1 AT THE INDEX OF SITE(THREAD ID) IN THE SITE_ARRAY */
			site_arr[siteid] = 1;
			/* INCREASES THE ENERGY PRODUCED BY 1 */
			sem_post(&energy_sem);
			sem_getvalue(&h_sem, &hydro);
			sem_getvalue(&o_sem, &oxy);
			sem_getvalue(&energy_sem, &counter);
			/* OUTPUT STATEMENT */
		printf("Site :%d , H atoms left :%d ,O atoms left :%d,Energy : %d EMJ\n",siteid,hydro, oxy,counter);
			*hydrogens = *hydrogens - 2;
			*oxygens = *oxygens - 1;
		}else{
			break;
		}
		/* AFTER COMPLTETION */
		site_arr[siteid] = 0;
		/* SLEEPS THREADS*/
		sleep(3);
		
		if(*sites >= 2 * (*thresh)){
                        sem_post(&th_sem);
                }else{
                        sem_post(&s_sem);
                }
	}
	//sem_post(&mutex);
	//pthread_exit(0);
}
 

