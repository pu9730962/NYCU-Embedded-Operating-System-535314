#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#include "sockop.h"


#define BUFSIZE 1024

int sockfd, connfd; /* socket descriptor */

void fd_handler(int signum) {
    close(sockfd);
}


// Allow test multi-service on the machine using different port
// unsigned int portbase = 1024;    
int passivesock(const char* service, const char* transport, int qlen)
{   
    int yes = 1;
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
    memset(strOut,'\0',len);
    strncpy(strOut,strIn + i, len);
    strOut[len-1]='\0';
    // printf("trim output:%s\n",strOut);
    return strOut;
}


int main(int argc, char *argv []){
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);
    int n;
    int temp = 1;
    int i;
    char snd [BUFSIZE] = "\0";
    char rcv [BUFSIZE] = "\0";
    char *menu = "1. Search\n2. Report\n3. Exit\n";
    char *wait_sig = "Please wait a few seconds...\n";
    char *delim = "|" ;
    char *disconnect;
    char *checkmenu;
    char *mode;
    char *region;
    char *people_number;
    char *people;
    char *number;
    int child[9];
    int adult[9];
    int elder[9]; 
    int total[9];
    int str_offset = 0;
    double delaytime = 0;
    clock_t start_time, end_time;
    // signal(SIGINT,fd_handler);
    if (argc != 2)
        errexit ("Usage: %s <port>\n",argv [0]);
    /* create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tep", 10);
    while(1){
        memset(child,0,sizeof(child));
        memset(adult,0,sizeof(adult));
        memset(elder,0,sizeof(elder));
        memset(total,0,sizeof(total));
        // str_offset = 0;
        delaytime = 0;

        /* waiting for connection */
        connfd = accept(sockfd , (struct sockaddr *) &addr_cln , &sLen);
        printf("This is client:%d\n\n",temp);
        if (connfd == -1){
            errexit (" Error : accept () \n" );
        }
        // read(connfd, rcv, BUFSIZE);
        // checkmenu = strtok(rcv,"\0");
        // if(strcmp(checkmenu,"Menu") == 0){
        //     if(n = write(connfd,menu,strlen(menu)+1) == -1){
        //         errexit("Error: write menu");
        //     }

        // }
        while(1){
            memset(rcv,'\0',BUFSIZE);
            n = read(connfd, rcv, BUFSIZE);
            printf("client send: %s\n",rcv);
            //單純menu
            if(strlen(rcv) == 4){
                printf("Server send: %s\n",menu);
                if(n = write(connfd,menu,strlen(menu)+1) == -1){
                    errexit("Error: write menu");
                }
            }
            //單純Search
            if(strlen(rcv) == 6){
                for(i = 0;i < 9;i++){
                    total[i]=child[i]+adult[i]+elder[i];
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
                        total[0],total[1],total[2],total[3],total[4],total[5],total[6],total[7],total[8]);
                printf("Server send: %s\n",snd);
                if(write(connfd, snd,(strlen(snd)+1)) == -1){
                    errexit("Error: write all region");
                }
                memset(snd,'\0',BUFSIZE);
            }
            //Search單獨地區或是Report或是exit
            else{
                mode = strtok(rcv,delim);
                mode = trim(mode);
                //離開
                if(strcmp(mode,"Exit") == 0){
                    free(mode);
                    printf("client %d disconnect!\n",temp);
                    close(connfd);
                    break;
                }
                //Search單獨地區
                else if(strcmp(mode,"Search") == 0){
                    free(mode);
                    region = strtok(NULL,delim);
                    region = trim(region);
                    if(strcmp(region,"Baseball Stadium") == 0){
                        sprintf(snd,"Baseball Stadium - Child : %d Adult : %d Elder : %d\n",child[0],adult[0],elder[0]);
                    }
                    else if(strcmp(region,"Big City") == 0){
                        sprintf(snd,"Big City - Child : %d Adult : %d Elder : %d\n",child[1],adult[1],elder[1]);
                    }
                    else if(strcmp(region,"Neiwan Old Street") == 0){
                        sprintf(snd,"Neiwan Old Street - Child : %d Adult : %d Elder : %d\n",child[2],adult[2],elder[2]);
                    }
                    else if(strcmp(region,"NYCU") == 0){
                        sprintf(snd,"NYCU - Child : %d Adult : %d Elder : %d\n",child[3],adult[3],elder[3]);
                    }
                    else if(strcmp(region,"Smangus") == 0){
                        sprintf(snd,"Smangus - Child : %d Adult : %d Elder : %d\n",child[4],adult[4],elder[4]);
                    }
                    else if(strcmp(region,"17 km of Splendid Coastline") == 0){
                        sprintf(snd,"17 km of Splendid Coastline - Child : %d Adult : %d Elder : %d\n",child[5],adult[5],elder[5]);
                    }
                    else if(strcmp(region,"100 Tastes") == 0){
                        sprintf(snd,"100 Tastes - Child : %d Adult : %d Elder : %d\n",child[6],adult[6],elder[6]);
                    }
                    else if(strcmp(region,"Science Park") == 0){
                        sprintf(snd,"Science Park - Child : %d Adult : %d Elder : %d\n",child[7],adult[7],elder[7]);
                    }
                    else if(strcmp(region,"City God Temple") == 0){
                        sprintf(snd,"City God Temple - Child : %d Adult : %d Elder : %d\n",child[8],adult[8],elder[8]);
                    }
                    printf("Server send: %s\n",snd);
                    if(write(connfd, snd,(strlen(snd)+1)) == -1){
                        errexit("Error: write single region");
                    }
                    memset(snd,'\0',BUFSIZE);
                    free(region);
                }
                //Report
                else if(strcmp(mode,"Report") == 0){
                    str_offset = 0;
                    free(mode);
                    if(write(connfd,wait_sig,strlen(wait_sig)+1) == -1){
                        errexit("Error: write waiting signal");
                    }
                    // start_time = clock();
                    while(1){
                        region = strtok(NULL,delim);
                        if(region != NULL){
                            region = trim(region);
                        }
                        if(region == NULL){
                            start_time = clock();
                            // printf("HELLO\n");
                            while(1){
                                end_time = clock();
                                if(((double)(end_time-start_time)/CLOCKS_PER_SEC)>delaytime){
                                    break;
                                }
                            }
                            printf("Server send: %s\n",snd);
                            if(write(connfd, snd,(strlen(snd)+1)) == -1){
                                errexit("Error: write Report signal");
                            }
                            memset(snd,'\0',BUFSIZE);
                            break;
                        }
                        else if(strcmp(region,"Baseball Stadium") == 0){
                            if(delaytime < 1){
                                delaytime = 1;
                            }
                            people = strtok(NULL," ");
                            number = strtok(NULL," ");
                            if(strcmp(people,"Child") == 0){
                                child[0] = child[0] + atoi(number);
                            }
                            else if(strcmp(people,"Adult") == 0){
                                adult[0] = adult[0] + atoi(number);
                            }
                            else if(strcmp(people,"Elder") == 0){
                                elder[0] = elder[0] + atoi(number);
                            }
                            sprintf(snd+str_offset,"Baseball Stadium | %s %s\n",people,number);
                            str_offset = strlen(snd);
                        }
                        else if(strcmp(region,"Big City") == 0){
                            if(delaytime < 2){
                                delaytime = 2;
                            }
                            people = strtok(NULL," ");
                            number = strtok(NULL," ");
                            if(strcmp(people,"Child") == 0){
                                child[1] = child[1] + atoi(number);
                            }
                            else if(strcmp(people,"Adult") == 0){
                                adult[1] = adult[1] + atoi(number);
                            }
                            else if(strcmp(people,"Elder") == 0){
                                elder[1] = elder[1] + atoi(number);
                            }
                            // printf("%d",snd);
                            sprintf(snd+str_offset,"Big City | %s %s\n",people,number);
                            // printf("%s",snd);
                            str_offset = strlen(snd);
                            // printf("test:%d\n",str_offset);
                            
                        }
                        else if(strcmp(region,"Neiwan Old Street") == 0){
                            if(delaytime < 3){
                                delaytime = 3;
                            }
                            people = strtok(NULL," ");
                            number = strtok(NULL," ");
                            if(strcmp(people,"Child") == 0){
                                child[2] = child[2] + atoi(number);
                            }
                            else if(strcmp(people,"Adult") == 0){
                                adult[2] = adult[2] + atoi(number);
                            }
                            else if(strcmp(people,"Elder") == 0){
                                elder[2] = elder[2] + atoi(number);
                            }
                            sprintf(snd+str_offset,"Neiwan Old Street | %s %s\n",people,number);
                            str_offset = strlen(snd);
                        }
                        else if(strcmp(region,"NYCU") == 0){
                            if(delaytime < 4){
                                delaytime = 4;
                            }
                            people = strtok(NULL," ");
                            number = strtok(NULL," ");
                            if(strcmp(people,"Child") == 0){
                                child[3] = child[3] + atoi(number);
                            }
                            else if(strcmp(people,"Adult") == 0){
                                adult[3] = adult[3] + atoi(number);
                            }
                            else if(strcmp(people,"Elder") == 0){
                                elder[3] = elder[3] + atoi(number);
                            }
                            sprintf(snd+str_offset,"NYCU | %s %s\n",people,number);
                            str_offset = strlen(snd);
                        }
                        else if(strcmp(region,"Smangus") == 0){
                            if(delaytime < 5){
                                delaytime = 5;
                            }
                            people = strtok(NULL," ");
                            number = strtok(NULL," ");
                            if(strcmp(people,"Child") == 0){
                                child[4] = child[4] + atoi(number);
                            }
                            else if(strcmp(people,"Adult") == 0){
                                adult[4] = adult[4] + atoi(number);
                            }
                            else if(strcmp(people,"Elder") == 0){
                                elder[4] = elder[4] + atoi(number);
                            }
                            sprintf(snd+str_offset,"Smangus | %s %s\n",people,number);
                            str_offset = strlen(snd);
                        }
                        else if(strcmp(region,"17 km of Splendid Coastline") == 0){
                            if(delaytime < 6){
                                delaytime = 6;
                            }
                            people = strtok(NULL," ");
                            number = strtok(NULL," ");
                            if(strcmp(people,"Child") == 0){
                                child[5] = child[5] + atoi(number);
                            }
                            else if(strcmp(people,"Adult") == 0){
                                adult[5] = adult[5] + atoi(number);
                            }
                            else if(strcmp(people,"Elder") == 0){
                                elder[5] = elder[5] + atoi(number);
                            }
                            sprintf(snd+str_offset,"17 km of Splendid Coastline | %s %s\n",people,number);
                            str_offset = strlen(snd);
                        }
                        else if(strcmp(region,"100 Tastes") == 0){
                            if(delaytime < 7){
                                delaytime = 7;
                            }
                            people = strtok(NULL," ");
                            number = strtok(NULL," ");
                            if(strcmp(people,"Child") == 0){
                                child[6] = child[6] + atoi(number);
                            }
                            else if(strcmp(people,"Adult") == 0){
                                adult[6] = adult[6] + atoi(number);
                            }
                            else if(strcmp(people,"Elder") == 0){
                                elder[6] = elder[6] + atoi(number);
                            }
                            sprintf(snd+str_offset,"100 Tastes | %s %s\n",people,number);
                            str_offset = strlen(snd);
                        }
                        else if(strcmp(region,"Science Park") == 0){
                            if(delaytime < 8){
                                delaytime = 8;
                            }
                            people = strtok(NULL," ");
                            number = strtok(NULL," ");
                            if(strcmp(people,"Child") == 0){
                                child[7] = child[7] + atoi(number);
                            }
                            else if(strcmp(people,"Adult") == 0){
                                adult[7] = adult[7] + atoi(number);
                            }
                            else if(strcmp(people,"Elder") == 0){
                                elder[7] = elder[7] + atoi(number);
                            }
                            sprintf(snd+str_offset,"Science Park | %s %s\n",people,number);
                            str_offset = strlen(snd);
                        }
                        else if(strcmp(region,"City God Temple") == 0){
                            if(delaytime < 9){
                                delaytime = 9;
                            }
                            people = strtok(NULL," ");
                            number = strtok(NULL," ");
                            if(strcmp(people,"Child") == 0){
                                child[8] = child[8] + atoi(number);
                            }
                            else if(strcmp(people,"Adult") == 0){
                                adult[8] = adult[8] + atoi(number);
                            }
                            else if(strcmp(people,"Elder") == 0){
                                elder[8] = elder[8] + atoi(number);
                            }
                            sprintf(snd+str_offset,"City God Temple | %s %s\n",people,number);
                            str_offset = strlen(snd);
                        }
                    }
                    free(region);
                }
            }
        }
        temp++;
    }
    /* close server socket */
    return 0;
}