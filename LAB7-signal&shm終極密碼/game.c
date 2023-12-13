#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>

int shmid;
int correct_ans;
int sig = 0;
key_t key;
typedef struct {
    int guess;
    char result[8];
}data;

void stop_handler(int signum) {
    if(shmctl(shmid, IPC_RMID, NULL) < 0){
        perror("shmctl() failed\n");
    }     
    printf("Share memory has been remove\n");
    sig++;
}

void checker_handler (int signal_number){
    data *checker_shm;
    int checker_shmid;
    if((checker_shmid = shmget(key, sizeof(data), IPC_CREAT | 0666)) < 0){
        perror("checker shmget() failed\n");
        exit(-1);
    }
    if((checker_shm = shmat(checker_shmid, NULL, 0)) == (data*) -1){
        perror("checker shmat() failed\n");
        exit(-1);
    }
    if(checker_shm -> guess == correct_ans){
        memset(checker_shm -> result,'\0',8);
        strcpy(checker_shm -> result, "bingo");
        printf("[game] Guess %d, bingo\n",checker_shm -> guess);
    }
    else if(checker_shm -> guess > correct_ans){
        memset(checker_shm -> result,'\0',8);
        strcpy(checker_shm -> result, "smaller");
        printf("[game] Guess %d, smaller\n",checker_shm -> guess);
    }
    else if(checker_shm -> guess < correct_ans){
        memset(checker_shm -> result,'\0',8);
        strcpy(checker_shm -> result, "bigger");
        printf("[game] Guess %d, bigger\n",checker_shm -> guess);
    }
    if(shmdt(checker_shm) < 0){
        perror("checker shmdt() failed\n");
        exit(-1);
    }
}

int main (int argc,char *argv[]){   
    key = atoi(argv[1]);
    data *shm, initial_data;
    struct sigaction sa;
    struct timespec req;
    int retval;
    correct_ans = atoi(argv[2]);
    printf("[game] Game PID: %d\n",getpid());
    initial_data.guess = 0;
    memset(initial_data.result,'\0',8);
    if((shmid = shmget(key, sizeof(data), IPC_CREAT | 0666)) < 0){
        perror("shmget() failed\n");
        exit(-1);
    }
    // printf("%d\n",shmid);
    if((shm = shmat(shmid, NULL, 0)) == (data*) -1){
        perror("shmat() failed\n");
        exit(-1);
    }
    memcpy(shm, &initial_data,sizeof(initial_data));
    if(shmdt(shm) < 0){
        perror("shmdt() failed\n");
        exit(-1);
    }
    signal(SIGINT, stop_handler);

    /* set the sleep time to 25 sec */
    memset(&req, 0, sizeof(struct timespec));
    req.tv_sec = 20;
    req.tv_nsec = 0;
    /* register handler to SIGUSR1 */
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = checker_handler;
    sigaction (SIGUSR1, &sa, NULL);
    // printf("Process (%d) is catching SIGUSR1 ...\n", getpid());
    /* sleep 10 sec */
    do{
        retval = nanosleep(&req, &req);
    } while(retval && sig == 0);

    if(shmctl(shmid, IPC_RMID, NULL) < 0){
        perror("shmctl() failed\n");
    }
    return 0;
}