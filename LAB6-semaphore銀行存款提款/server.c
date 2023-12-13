#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include "sockop.h"

#define BUFSIZE 1024
#define SEM_KEY 1122334455
#define SEM_MODE 0666

int sockfd, connfd; /* socket descriptor */
pid_t child_pid;
int sem;
int shmid;
key_t key = 5678;
int check = 0;
int passivesock(const char* service, const char* transport, int qlen)
{
    // Store service entry return from getservbyname()
    struct servent *pse;        
    // Service-end socket
    struct sockaddr_in sin;   
    // Service-end socket descriptor and service type
    int s, type;              
    memset(&sin, 0, sizeof(sin));
    // TCP/IP suite
    sin.sin_family = AF_INET; 
    // Use any local IP, need translate to internet byte order
    sin.sin_addr.s_addr = INADDR_ANY; 
    // Get port number
    // service is service name
    if (pse = getservbyname(service, transport))
        // sin.sin_port = htons(ntohs((unsigned short)pse->s_port) + portbase);
         sin.sin_port = htons(ntohs((unsigned short)pse->s_port));
    // service is port number
    else if((sin.sin_port = htons((unsigned short)atoi(service))) == 0)
        errexit("can't get \"%s\" service entry\n", service);
    // Tranport type
    if (strcmp(transport, "udp") == 0 || strcmp(transport, "UDP") == 0)
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;

    s = socket(PF_INET, type, 0);
    if (s < 0)
        errexit("can't create socket: %s \n", strerror(errno));
    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    // Bind socket to service-end address
    if (bind(s, (struct sockaddr*)&sin, sizeof(sin)) < 0)
        errexit("can't bind to %s port: %s \n", service, strerror(errno));
    // For TCP socket, convert it to passive mode
    if (type == SOCK_STREAM && listen(s, qlen) < 0)
        errexit("can't listen on %s port: %s \n", service, strerror(errno));
    return s;
}

void process_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);  //-1等待任何childprocess結束 
}

void stop_handler(int signum) {
    printf("%d\n",check);
    if(check == 0){
        close(sockfd);
        if(semctl(sem, 0 , IPC_RMID, 0) < 0){
            fprintf (stderr, "Unable to remove sem\n");
        }
        // printf("Semaphore has been remove\n");
        if(shmctl(shmid, IPC_RMID, NULL) < 0){
            perror("shmctl() failed\n");
        }     
    }

    check = 1;
    printf("%d\n",check);
    // printf("Share memory has been remove\n");
}

int P(int s)
{
    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0; /* access the 1st (and only) sem in the array */
    sop.sem_op = -1; /* wait..*/
    sop.sem_flg = 0; /* no special options needed */
    if (semop (s, &sop, 1) < 0) {
        fprintf(stderr,"P(): semop failed: %s\n",strerror(errno));
        return -1;
    } 
    else {
        return 0;
    }
}
int V(int s)
{
    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0; /* the 1st (and only) sem in the array */
    sop.sem_op = 1; /* signal */
    sop.sem_flg = 0; /* no special options needed */
    if (semop(s, &sop, 1) < 0) {
        fprintf(stderr,"V(): semop failed: %s\n",strerror(errno));
        return -1;
    } 
    else {
        return 0;
    }
}
void bank(char *mode1, int number1){
    int child_shmid;
    int *shm, *child_data;
    int total_money;
    P(sem);
    if((child_shmid = shmget(key, 8, 0666)) < 0){
        perror("child process shmget() failed\n");
        exit(-1);
    }
    if((shm = shmat(child_shmid, NULL, 0)) == (int *) -1){
        perror("child process shmat() failed\n");
        exit(-1);
    }
    child_data = shm;
    total_money = *child_data;
    if(strcmp(mode1,"deposit") == 0){
        total_money += number1;
        printf("After deposit: %d\n",total_money);
    }
    else if(strcmp(mode1,"withdraw") == 0){
        total_money -= number1;
        printf("After withdraw: %d\n",total_money);
    }
    *child_data = total_money;
    shmdt(shm);
    V(sem);
}

// Allow test multi-service on the machine using different port
// unsigned int portbase = 1024;    

int main(int argc, char *argv []){
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);
    int a = 0;
    int total_money = 0;
    int ack;
    int n;
    int status;
    char rcv [BUFSIZE];
    char *mode;
    int number;
    int *shm, *our_data;
    if (argc != 2)
        errexit ("Usage: %s <port>\n",argv[0]);
    /* create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tcp", 10);
    sem = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if(sem < 0){
        fprintf(stderr, "Semaphore %d creation failed: %s\n", SEM_KEY,strerror(errno));
        exit(-1);
    }
    if(semctl(sem, 0, SETVAL, 1) < 0){
        fprintf(stderr, "Unable to initialize Semaphore: %s\n",strerror(errno));
        exit(-1);
    }
    /*Create the share memory*/
    if((shmid = shmget(key, 8, IPC_CREAT | 0666)) < 0){
        perror("shmget() failed\n");
        exit(-1);
    }
    /*Attach the share memory to our data space*/
    if((shm = shmat(shmid, NULL, 0)) == (int *) -1){
        perror("shmat failed\n");
        exit(-1);
    }
    our_data = shm;
    *our_data = total_money;
    if(shmdt(shm) < 0){
        perror("shmdt() failed\n");
        exit(-1);
    }
    signal(SIGCHLD, process_handler);
    signal(SIGINT,stop_handler);
    while(1){
        /* waiting for connection */
        connfd = accept(sockfd , (struct sockaddr*) &addr_cln , &sLen);
        if(connfd < 0){
            errexit (" Error : accept () \n" );
        }
        n = read(connfd, rcv, BUFSIZE);
        if(n > 0){
            // printf("%s\n",rcv);
            mode = strtok(rcv," ");
            number = atoi(strtok(NULL," "));
            child_pid = fork();
            // printf("child_pid : %d\n", child_pid);
            if (child_pid >= 0){
                if (child_pid == 0){
                    bank(mode, number);
                    // signal(SIGCHLD, process_handler);
                }
                else if(child_pid > 0){
                    // if(a == 0){
                    //     signal(SIGCHLD, process_handler);
                    //     signal(SIGINT,stop_handler);
                    // }
                }
            }
            else{
                perror("fork()");
                exit(-1);
            }
            memset(rcv,0,BUFSIZE);
        }
        close(connfd);
        a++;
    }
    return 0;
}