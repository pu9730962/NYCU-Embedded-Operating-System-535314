#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
key_t key;
int upperbound;
int lowerbound = 0;
pid_t game_pid;
typedef struct {
    int guess;
    char result[8];
}data;
void guesser_handler (int signum)
{   
    data *guesser_shm;
    int guesser_shmid;
    if((guesser_shmid = shmget(key, sizeof(data), IPC_CREAT | 0666)) < 0){
        perror("guesser shmget() failed\n");
        exit(-1);
    }
    if((guesser_shm = shmat(guesser_shmid, NULL, 0)) == (data*) -1){
        perror("guesser shmat() failed\n");
        exit(-1);
    }

    if(strcmp(guesser_shm -> result, "bingo") == 0){
        exit(0);
    }
    else if(strcmp(guesser_shm -> result, "bigger") == 0){
        lowerbound = guesser_shm -> guess;
        guesser_shm -> guess = (upperbound + lowerbound)/2;
        printf("[game] Guess: %d\n",guesser_shm -> guess);

    }
    else if(strcmp(guesser_shm -> result, "smaller") == 0){
        upperbound = guesser_shm -> guess;
        guesser_shm -> guess = (upperbound + lowerbound)/2;
        printf("[game] Guess: %d\n",guesser_shm -> guess);
    }

    if(shmdt(guesser_shm) < 0){
        perror("guesser shmdt() failed\n");
        exit(-1);
    }
    
    if(kill(game_pid,SIGUSR1) < 0){
        perror("guesser kill() failed");
        exit(-1);
    }
}
int main (int argc, char **argv)
{   
    data *shm;
    int shmid;
    struct sigaction sa;
    struct itimerval timer;
    key = atoi(argv[1]);
    upperbound = atoi(argv[2]);
    game_pid = atoi(argv[3]);
    if((shmid = shmget(key, sizeof(data), IPC_CREAT | 0666)) < 0){
        perror("shmget() failed\n");
        exit(-1);
    }
    // printf("%d\n",shmid);
    if((shm = shmat(shmid, NULL, 0)) == (data*) -1){
        perror("shmat() failed\n");
        exit(-1);
    }
    shm -> guess = (upperbound + lowerbound)/2;
    /* Install timer_handler as the signal handler for SIGVTALRM */
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &guesser_handler;
    sigaction (SIGVTALRM, &sa, NULL);
    /* Configure the timer to expire after 250 msec */
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    /* Reset the timer back to 250 msec after expired */
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;
    /* Start a virtual timer */
    setitimer (ITIMER_VIRTUAL, &timer, NULL);
    printf("[game] Guess: %d\n",shm -> guess);
    if(shmdt(shm) < 0){
        perror("shmdt() failed\n");
        exit(-1);
    }
    /* Do busy work */
    while (1);
    return 0;
}
