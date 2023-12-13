#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include "sockop.h"

#define BUFSIZE 1024

int sockfd, connfd; /* socket descriptor */
pid_t child_pid;

void process_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);  //-1等待任何childprocess結束 
}
void fd_handler(int signum) {
    close(sockfd);
}
void childfunc(void){
    dup2(connfd,STDOUT_FILENO);
    execlp("sl","sl","-l",NULL);
    close(connfd);
    exit(0);
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

int connectsock(const char *host, const char *service, const char *transport)
{
    struct hostent *phe;    // 主機條目
    struct servent *pse;    // 服務條目
    struct sockaddr_in sin; // Internet端點地址
    int s, type;            // 套接字描述符和套接字類型

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    // 查找服務條目
    if ((pse = getservbyname(service, transport)))
        sin.sin_port = pse->s_port;
    else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0)
        return -1;

    // 使用協議創建套接字
    if (strcmp(transport, "udp") == 0)
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;

    if ((s = socket(PF_INET, type, 0)) < 0)
        return -1;

    // 連接到服務器
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        return -1;

    return s;
}

int main(int argc, char *argv []){
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);
    int n;
    char snd [BUFSIZE], rcv [BUFSIZE];
    signal(SIGCHLD, process_handler);
    signal(SIGINT,fd_handler);
    if (argc != 2)
        errexit ("Usage: %s <port>\n",argv [0]);
    /* create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tep", 10);
    while(1){
        /* waiting for connection */
        connfd = accept(sockfd , (struct sockaddr *) &addr_cln , &sLen);
        if (connfd == -1)
            errexit (" Error : accept () \n" );
        child_pid = fork();
        if (child_pid >= 0){
            if (child_pid == 0){
                childfunc();
            }
            else{
                printf("Train ID: %d\n",child_pid);
            }
        }
        else{
            perror("fork()");
            exit(-1);
        }
    }
    /* close server socket */
    return 0;
}