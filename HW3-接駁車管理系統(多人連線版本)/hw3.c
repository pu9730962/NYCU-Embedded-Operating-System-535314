#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include "sockop.h"


#define BUFSIZE 256
#define SEM_KEY 1122334455
#define SEM_MODE 0666
int sockfd;
int sem;
int shmid;
key_t key = 5678;
struct shmdata_struct{
    int child[9];
    int adult[9];
    int elder[9];
    int total[9];
    int Acar_timechecker[9];
    int Bcar_timechecker[9];
    int Acar_scheduling[9];
    int Bcar_scheduling[9];
};

// Allow test multi-service on the machine using different port
// unsigned int portbase = 1024;    
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

char* trim(char *strIn){
    // printf("trim input:%s\n",strIn);
    int i, j, len;
    i = 0;
    j = strlen(strIn)-1;

    while(strIn[i] == ' '){
        i++;
    }
    while(strIn[j] == ' '){
        j--;
    }
    len = j - i + 2;
    char *strOut = (char*)malloc(len*sizeof(char));
    memset(strOut,0,len);
    strncpy(strOut,strIn + i, len);
    strOut[len-1]=0;
    // printf("trim output:%s\n",strOut);
    return strOut;
}

void stop_handler(int signum) {
    close(sockfd);
    if(semctl(sem, 0 , IPC_RMID, 0) < 0){
        fprintf (stderr, "Unable to remove sem\n");
    }
    printf("Semaphore has been remove\n");
    if(shmctl(shmid, IPC_RMID, NULL) < 0){
        perror("shmctl() failed\n");
    }     
    printf("Share memory has been remove\n");
}

void process_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);  //-1等待任何childprocess結束 
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

int Car_scheduling(struct shmdata_struct *shm, int region_flag){
    int i;
    int delaytime = 0;
    int A_merge = 0;
    int B_merge = 0;
    int A_merge_index = 0; 
    int B_merge_index = 0;
    int A_zero_index = 0;
    int B_zero_index = 0; 
    int Acar_wait_time = 0;
    int Bcar_wait_time = 0;
    for(i = 0;i < 9;i++){
        /*check whether A car can merge or not*/
        if(shm -> Acar_scheduling[i] == region_flag){
            /*A_merge = 1 represent A can merge*/
            A_merge = 1;
            A_merge_index = i;
        }
        /*check whether B car can merge or not*/
        if(shm -> Bcar_scheduling[i] == region_flag){
            B_merge = 1;
            B_merge_index = i;
        }
    }
    while(shm -> Acar_scheduling[A_zero_index] != 0){
        A_zero_index++;
    }
    // printf("A_zero_index = %d\n",A_zero_index);
    while(shm -> Bcar_scheduling[B_zero_index] != 0){
        B_zero_index++;
    }
    // printf("B_zero_index = %d\n",B_zero_index);
    /*condition: can merge*/
    if(A_merge == 1){
        // printf("A CAR can merge\n");
        for(i = 0;i <= A_merge_index;i++){
            delaytime += shm -> Acar_timechecker[i];
        }
    }
    else if(B_merge == 1){
        // printf("B CAR can merge\n");
        for(i = 0;i <= B_merge_index;i++){
            delaytime += shm -> Bcar_timechecker[i];
        }
    }
    /*condition: can't merge*/
    else{
        // printf("can't merge\n");
        for(i = 0;i < A_zero_index;i++){
            Acar_wait_time += shm -> Acar_timechecker[i];
        }
        // printf("A car wait time: %d\n",Acar_wait_time);
        for(i = 0;i < B_zero_index;i++){
            Bcar_wait_time += shm -> Bcar_timechecker[i];
        }
        // printf("B car wait time: %d\n",Bcar_wait_time);
        if(Acar_wait_time < Bcar_wait_time){
            shm -> Acar_scheduling[A_zero_index] = region_flag;
            shm -> Acar_timechecker[A_zero_index] = region_flag;
            for(i = 0;i <= A_zero_index;i++){
                delaytime += shm -> Acar_timechecker[i];
            }
        }
        else{
            shm -> Bcar_scheduling[B_zero_index] = region_flag;
            shm -> Bcar_timechecker[B_zero_index] = region_flag;
            // printf("%d\n",shm -> Bcar_scheduling[B_zero_index]);
            // printf("%d\n",shm -> Bcar_timechecker[B_zero_index]);
            for(i = 0;i <= B_zero_index;i++){
                // printf("yes\n");
                delaytime += shm -> Bcar_timechecker[i];
            }

        }
    }
    return delaytime;          
}

void child_process(int connfd){
    int child_shmid;
    struct shmdata_struct *shm;
    int region_flag = 0;
    int n;
    // int temp = 1;
    int i;
    int index;
    int region_index;
    char snd [BUFSIZE];
    char rcv [BUFSIZE];
    char menu[BUFSIZE];
    char wait_sig[BUFSIZE];
    char *delim = "|" ;
    char *disconnect;
    char *checkmenu;
    char *mode;
    char *region;
    char *people_number;
    char cut_arr[50][50];
    char *checker;
    char *people;
    char *number;
    char temp_str[50];
    int register_table[9][3];
    int delaytime = 0;
    int tem_delaytime = 0;
    clock_t start_time, end_time;

    memset(menu,'\0',BUFSIZE);
    memset(wait_sig,'\0',BUFSIZE);
    strcpy(menu,"1. Search\n2. Report\n3. Exit\n");
    strcpy(wait_sig,"The shuttle bus is on it's way...\n");
    if((child_shmid = shmget(key, sizeof(struct shmdata_struct), 0666)) < 0){
        perror("child process shmget() failed\n");
        exit(-1);
    }
    if((shm = shmat(child_shmid, NULL, 0)) == (struct shmdata_struct *) -1){
        perror("child process shmat() failed\n");
        exit(-1);
    }

    while(1){
        memset(snd,'\0',BUFSIZE);
        memset(rcv,'\0',BUFSIZE);
        n = read(connfd, rcv, BUFSIZE);
        if(n < 0){
            perror("Child process read() failed\n");
            exit(-1);
        }
        else if(n == 0){
            // printf("%d client disconnect!\n",connfd);
            shmdt(shm);
            close(connfd);
            printf("One client exit\n");
            break;
        }
        else{
            printf("client send: %s\n",rcv);
            //單純"Menu\n"
            if(strcmp(rcv,"Menu\n") == 0){
                printf("Server send: %s\n",menu);
                if((n = write(connfd,menu,BUFSIZE)) == -1){
                    errexit("Error: write menu signal");
                }
            }
            //單純"Search\n"
            else if(strcmp(rcv,"Search\n") == 0){
                P(sem);
                for(i = 0;i < 9;i++){
                    shm -> total[i] = shm -> child[i] + shm -> adult[i] + shm -> elder[i];
                }
                sprintf(snd,
                        "1. Baseball Stadium : %d\n"
                        "2. Big City : %d\n"
                        "3. Neiwan Old Street : %d\n"
                        "4. NYCU : %d\n"
                        "5. Smangus : %d\n"
                        "6. 17 km of Splendid Coastline : %d\n"
                        "7. 100 Tastes : %d\n"
                        "8. Science Park : %d\n"
                        "9. City God Temple : %d\n",
                        shm -> total[0],shm -> total[1],shm -> total[2],shm -> total[3],shm -> total[4],shm -> total[5],shm -> total[6],shm -> total[7],shm -> total[8]);
                V(sem);
                printf("Server send: %s\n",snd);
                if(write(connfd, snd, BUFSIZE) == -1){
                    errexit("Error: write all region");
                }
                memset(snd,'\0',BUFSIZE);
            }
            else if(strcmp(rcv,"Exit\n") == 0){
                shmdt(shm);
                close(connfd);
                printf("One client exit\n");
                break;
            }
            //Search單獨地區或是Report
            else{
                mode = strtok(rcv,delim);
                mode = trim(mode);
                //Search單獨地區
                if(strcmp(mode,"Search") == 0){
                    free(mode);
                    P(sem);
                    region = strtok(NULL,delim);
                    region = trim(region);
                    if(strcmp(region,"Baseball Stadium\n") == 0){
                        sprintf(snd,"Baseball Stadium - Child : %d | Adult : %d | Elder : %d\n",shm -> child[0],shm -> adult[0],shm -> elder[0]);
                    }
                    else if(strcmp(region,"Big City\n") == 0){
                        sprintf(snd,"Big City - Child : %d | Adult : %d | Elder : %d\n",shm -> child[1],shm -> adult[1],shm -> elder[1]);
                    }
                    else if(strcmp(region,"Neiwan Old Street\n") == 0){
                        sprintf(snd,"Neiwan Old Street - Child : %d | Adult : %d | Elder : %d\n",shm -> child[2],shm -> adult[2],shm -> elder[2]);
                    }
                    else if(strcmp(region,"NYCU\n") == 0){
                        sprintf(snd,"NYCU - Child : %d | Adult : %d | Elder : %d\n",shm -> child[3],shm -> adult[3],shm -> elder[3]);
                    }
                    else if(strcmp(region,"Smangus\n") == 0){
                        sprintf(snd,"Smangus - Child : %d | Adult : %d | Elder : %d\n",shm -> child[4],shm -> adult[4],shm -> elder[4]);
                    }
                    else if(strcmp(region,"17 km of Splendid Coastline\n") == 0){
                        sprintf(snd,"17 km of Splendid Coastline - Child : %d | Adult : %d | Elder : %d\n",shm -> child[5],shm -> adult[5],shm -> elder[5]);
                    }
                    else if(strcmp(region,"100 Tastes\n") == 0){
                        sprintf(snd,"100 Tastes - Child : %d | Adult : %d | Elder : %d\n",shm -> child[6],shm -> adult[6],shm -> elder[6]);
                    }
                    else if(strcmp(region,"Science Park\n") == 0){
                        sprintf(snd,"Science Park - Child : %d | Adult : %d | Elder : %d\n",shm -> child[7],shm -> adult[7],shm -> elder[7]);
                    }
                    else if(strcmp(region,"City God Temple\n") == 0){
                        sprintf(snd,"City God Temple - Child : %d | Adult : %d | Elder : %d\n",shm -> child[8],shm -> adult[8],shm -> elder[8]);
                    }
                    V(sem);
                    printf("Server send: %s\n",snd);
                    if(write(connfd, snd, BUFSIZE) == -1){
                        errexit("Error: write single region");
                    }
                    memset(snd,'\0',BUFSIZE);
                    free(region);
                }
                //Report
                else if(strcmp(mode,"Report") == 0){
                    free(mode);  
                    i = 0; 
                    // str_offset = 0;
                    delaytime = 0;
                    tem_delaytime = 0;
                    if(write(connfd,wait_sig,BUFSIZE) == -1){
                        errexit("Error: write waiting signal");
                    }
                    memset(temp_str,'\0',50);
                    memset(cut_arr, 0, sizeof(cut_arr));
                    memset(register_table,0,sizeof(register_table));
                    /*先將Report ｜之後全部切下來儲存到cut_arr*/
                    while((checker = strtok(NULL,delim)) != NULL){
                        strcpy(cut_arr[i],checker);
                        i++;
                    }
                    /*判斷cut_arr裡面有哪些地區和人種人數將它紀錄在register_table*/ 
                    for(index = 0; index < i; index++){
                        if(strcmp(cut_arr[index]," Baseball Stadium ") == 0){
                            region_index = 0;
                        }
                        else if(strcmp(cut_arr[index]," Big City ") == 0){
                            region_index = 1;
                        }
                        else if(strcmp(cut_arr[index]," Neiwan Old Street ") == 0){
                            region_index = 2;
                        }
                        else if(strcmp(cut_arr[index]," NYCU ") == 0){
                            region_index = 3;
                        }
                        else if(strcmp(cut_arr[index]," Smangus ") == 0){
                            region_index = 4;
                        }
                        else if(strcmp(cut_arr[index]," 17 km of Splendid Coastline ") == 0){
                            region_index = 5;
                        }
                        else if(strcmp(cut_arr[index]," 100 Tastes ") == 0){
                            region_index = 6;
                        }
                        else if(strcmp(cut_arr[index]," Science Park ") == 0){
                            region_index = 7;
                        }
                        else if(strcmp(cut_arr[index]," City God Temple ") == 0){
                            region_index = 8;
                        }
                        else{
                            people = strtok(cut_arr[index]," ");
                            number = strtok(NULL," ");
                            if(strcmp(people,"Child") == 0){
                                register_table[region_index][0] += atoi(number);
                            }
                            if(strcmp(people,"Adult") == 0){
                                register_table[region_index][1] += atoi(number);
                            }
                            if(strcmp(people,"Elder") == 0){
                                register_table[region_index][2] += atoi(number);
                            }
                        }
                    }

                    P(sem);
                    for(index = 0; index < 9; index++){
                        if(register_table[index][0] != 0 || register_table[index][1] != 0 || register_table[index][2] != 0){
                            if(index == 0){
                                // printf("Baseball Stadium\n");
                                /*region_flag represents real region index*/
                                tem_delaytime = Car_scheduling(shm,region_flag = 1);
                                if (tem_delaytime > delaytime){
                                    delaytime = tem_delaytime;
                                }
                                strcpy(temp_str,"Baseball Stadium ");
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);
                                // sprintf(snd+str_offset,"Baseball Stadium ");
                                // str_offset = strlen(snd); 
                            }
                            else if(index == 1){
                                // printf("Big City\n");
                                tem_delaytime = Car_scheduling(shm,region_flag = 2);
                                if (tem_delaytime > delaytime){
                                    delaytime = tem_delaytime;
                                }
                                strcpy(temp_str,"Big City ");
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);
                                // sprintf(snd+str_offset,"Big City ");
                                // str_offset = strlen(snd); 
                            }
                            else if(index == 2){
                                // printf("Neiwan Old Street\n");
                                tem_delaytime = Car_scheduling(shm,region_flag = 3);
                                if (tem_delaytime > delaytime){
                                    delaytime = tem_delaytime;
                                }
                                strcpy(temp_str,"Neiwan Old Street ");
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);
                                // sprintf(snd+str_offset,"Neiwan Old Street ");
                                // str_offset = strlen(snd); 
                            }
                            else if(index == 3){
                                // printf("NYCU\n");
                                tem_delaytime = Car_scheduling(shm,region_flag = 4);
                                if (tem_delaytime > delaytime){
                                    delaytime = tem_delaytime;
                                }
                                strcpy(temp_str,"NYCU ");
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);

                                // sprintf(snd+str_offset,"NYCU ");
                                // str_offset = strlen(snd); 
                            }
                            else if(index == 4){
                                // printf("Smangus\n");
                                tem_delaytime = Car_scheduling(shm,region_flag = 5);
                                if (tem_delaytime > delaytime){
                                    delaytime = tem_delaytime;
                                }
                                strcpy(temp_str,"Smangus ");
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);
                                // sprintf(snd+str_offset,"Smangus ");
                                // str_offset = strlen(snd); 
                            }
                            else if(index == 5){
                                // printf("17 km of Splendid Coastline\n");
                                tem_delaytime = Car_scheduling(shm,region_flag = 6);
                                if (tem_delaytime > delaytime){
                                    delaytime = tem_delaytime;
                                }
                                strcpy(temp_str,"17 km of Splendid Coastline ");
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);
                                // sprintf(snd+str_offset,"17 km of Splendid Coastline ");
                                // str_offset = strlen(snd); 
                            }
                            else if(index == 6){
                                // printf("100 Tastes\n");
                                tem_delaytime = Car_scheduling(shm,region_flag = 7);
                                if (tem_delaytime > delaytime){
                                    delaytime = tem_delaytime;
                                }
                                strcpy(temp_str,"100 Tastes ");
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);
                                // sprintf(snd+str_offset,"100 Tastes ");
                                // str_offset = strlen(snd); 
                            }
                            else if(index == 7){
                                // printf("Science Park\n");
                                tem_delaytime = Car_scheduling(shm,region_flag = 8);
                                if (tem_delaytime > delaytime){
                                    delaytime = tem_delaytime;
                                }
                                strcpy(temp_str,"Science Park ");
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);
                                // sprintf(snd+str_offset,"Science Park ");
                                // str_offset = strlen(snd); 
                            }
                            else if(index == 8){
                                // printf("City God Temple\n");
                                tem_delaytime = Car_scheduling(shm,region_flag = 9);
                                if (tem_delaytime > delaytime){
                                    delaytime = tem_delaytime;
                                }
                                strcpy(temp_str,"City God Temple ");
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);
                                // sprintf(snd+str_offset,"City God Temple ");
                                // str_offset = strlen(snd); 
                            }
                            if(register_table[index][0] != 0){
                                shm -> child[index] = shm -> child[index] + register_table[index][0];
                                sprintf(temp_str,"| Child %d ",register_table[index][0]);
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);
                                // sprintf(snd+str_offset,"| Child %d ",register_table[index][0]);
                                // str_offset = strlen(snd); 
                            }
                            if(register_table[index][1] != 0){
                                shm -> adult[index] = shm -> adult[index] + register_table[index][1];
                                sprintf(temp_str,"| Adult %d ",register_table[index][1]);
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);        
                                // sprintf(snd+str_offset,"| Adult %d ",register_table[index][1]);
                                // str_offset = strlen(snd); 
                            }
                            if(register_table[index][2] != 0){
                                shm -> elder[index] = shm -> elder[index] + register_table[index][2];
                                sprintf(temp_str,"| Elder %d ",register_table[index][2]);
                                strcat(snd, temp_str);
                                memset(temp_str,'\0',50);   
                                // sprintf(snd+str_offset,"| Elder %d ",register_table[index][2]);
                                // str_offset = strlen(snd); 
                            }
                            snd[strlen(snd)-1] = '\n';
                            // sprintf(snd+str_offset-1,"\n");
                            // str_offset = strlen(snd); 
                        }
                    }
                    snd[strlen(snd)] = '\0';
                        // printf("\ntotal number in semaphore:\n");
                        // printf("%d %d %d\n",shm -> child[0],shm -> adult[0],shm -> elder[0]);
                        // printf("%d %d %d\n",shm -> child[1],shm -> adult[1],shm -> elder[1]);
                        // printf("%d %d %d\n",shm -> child[2],shm -> adult[2],shm -> elder[2]);
                        // printf("%d %d %d\n",shm -> child[3],shm -> adult[3],shm -> elder[3]);
                        // printf("%d %d %d\n",shm -> child[4],shm -> adult[4],shm -> elder[4]);
                        // printf("%d %d %d\n",shm -> child[5],shm -> adult[5],shm -> elder[5]);
                        // printf("%d %d %d\n",shm -> child[6],shm -> adult[6],shm -> elder[6]);
                        // printf("%d %d %d\n",shm -> child[7],shm -> adult[7],shm -> elder[7]);
                        // printf("%d %d %d\n",shm -> child[8],shm -> adult[8],shm -> elder[8]);
                    V(sem);
                    printf("delaytime %d\n",delaytime);
                    sleep(delaytime);
                    printf("Server send: %s\n",snd);
                    if(write(connfd, snd, BUFSIZE) == -1){
                        errexit("Error: write Report signal\n");
                    }
                    memset(snd, '\0', BUFSIZE);
                }
            }
        }    
    }
}

void timer(void){
    int i;
    int timer_shmid;
    struct shmdata_struct *shm;
    clock_t start_time, end_time;

    while(1){
        // start_time = clock();
        // while(1){
        //     end_time = clock();
        //     if(((double)(end_time-start_time)/CLOCKS_PER_SEC) > 1){
        //         break;
        //     }
        // }
        sleep(1);
        if((timer_shmid = shmget(key, sizeof(struct shmdata_struct), 0666)) < 0){
            perror("timer process shmget() failed\n");
            exit(-1);
        }
        if((shm = shmat(timer_shmid, NULL, 0)) == (struct shmdata_struct *) -1){
            perror("child process shmat() failed\n");
            exit(-1);
        }
        P(sem);
        // printf("A car time_checker:");
        // for(i = 0;i < 9 ;i++){
        //     printf(" %d",shm -> Acar_timechecker[i]);
        // }
        // printf("\n");
        // printf("B car time_checker:");
        // for(i = 0;i < 9 ;i++){
        //     printf(" %d",shm -> Bcar_timechecker[i]);
        // }
        // printf("\n"); 
        if((shm -> Acar_timechecker[0]) != 0){
            shm -> Acar_timechecker[0] -= 1;
            if(shm -> Acar_timechecker[0] == 0){
                for(i = 0;i < 8 ;i++){
                    shm -> Acar_scheduling[i] = shm -> Acar_scheduling[i+1];
                    shm -> Acar_timechecker[i] = shm -> Acar_timechecker[i+1];
                }
                shm -> Acar_scheduling[8] = 0;
                shm -> Acar_timechecker[8] = 0;
            }
        }
        if((shm -> Bcar_timechecker[0]) != 0){
            shm -> Bcar_timechecker[0] -= 1;
            if(shm -> Bcar_timechecker[0] == 0){
                for(i = 0;i < 8 ;i++){
                    shm -> Bcar_scheduling[i] = shm -> Bcar_scheduling[i+1];
                    shm -> Bcar_timechecker[i] = shm -> Bcar_timechecker[i+1];
                }
                shm -> Bcar_scheduling[8] = 0;
                shm -> Bcar_timechecker[8] = 0;
            }
        }
        shmdt(shm);
        V(sem);
    }
}
int main(int argc, char *argv []){
    int main_loop = 0;
    int connfd; /* socket descriptor */
    pid_t child_pid;
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);
    struct shmdata_struct initial_data,*shm;
    memset(initial_data.child,0,sizeof(initial_data.child));
    memset(initial_data.adult,0,sizeof(initial_data.adult));
    memset(initial_data.elder,0,sizeof(initial_data.elder));
    memset(initial_data.total,0,sizeof(initial_data.total));
    memset(initial_data.Acar_scheduling,0,sizeof(initial_data.Acar_scheduling));
    memset(initial_data.Bcar_scheduling,0,sizeof(initial_data.Bcar_scheduling));
    memset(initial_data.Acar_timechecker,0,sizeof(initial_data.Acar_timechecker));
    memset(initial_data.Bcar_timechecker,0,sizeof(initial_data.Bcar_timechecker));

    if (argc != 2)
        errexit ("Usage: %s <port>\n",argv [0]);
    /* create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tcp", 1000);
    /* create semaphore*/
    sem = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if(sem < 0){
        fprintf(stderr, "Semaphore %d creation failed: %s\n", SEM_KEY,strerror(errno));
        exit(-1);
    }
    if(semctl(sem, 0, SETVAL, 1) < 0){
        fprintf(stderr, "Unable to initialize Semaphore: %s\n",strerror(errno));
        exit(-1);
    }
    if((shmid = shmget(key, sizeof(struct shmdata_struct), IPC_CREAT | 0666)) < 0){
        perror("shmget() failed\n");
        exit(-1);
    }
    if((shm = shmat(shmid, NULL, 0)) == (struct shmdata_struct*) -1){
        perror("shmat() failed\n");
        exit(-1);
    }
    memcpy(shm, &initial_data,sizeof(initial_data));
    if(shmdt(shm) < 0){
        perror("shmdt() failed\n");
        exit(-1);
    }
    signal(SIGINT,stop_handler);
    signal(SIGCHLD, process_handler);
    while(1){
        /*create a timer in first loop*/
        if(main_loop == 0){
            child_pid = fork();
            if(child_pid >= 0){
                if(child_pid == 0){
                    timer();
                }
                // else{
                //     signal(SIGINT,stop_handler);
                //     signal(SIGCHLD, process_handler);
                // }
            }
            else{
                perror("timer fork() failed\n");
                exit(-1);
            }
        }
        else if(main_loop != 0){
            /* waiting for connection */
            connfd = accept(sockfd , (struct sockaddr *) &addr_cln , &sLen);
            if (connfd == -1){
                errexit ("Error : accept ()\n" );
            }
            // printf("%d client connect!\n",connfd);
            child_pid = fork();
            if(child_pid >= 0){
                if(child_pid == 0){
                    child_process(connfd);
                }
                else{
                    close(connfd);
                }
            }
            else{
                perror("Error : child_process fork() failed\n");
                exit(-1);
            }
        }
        main_loop++;
    }
    return 0;
}