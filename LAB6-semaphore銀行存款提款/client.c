#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sockop.h"
#define BUFSIZE 1024
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

int main(int argc, char *argv[]){
    int connfd; /* socket descriptor */
    int n;
    int times;
    char buf[BUFSIZE];

    if (argc != 6){
        errexit("Usage: %s <ip> <port> <depoit/withdraw> <amount> <times>\n",argv[0]);
    }
    times = atoi(argv[5]);
    for(int i = 0;i < times;i++){
        /* create socket and connect to server */
        connfd = connectsock(argv[1], argv[2], "tcp");
        sprintf(buf,"%s %s",argv[3],argv[4]);
        printf("%s\n",buf);
        /* write message to server */
        if ((n = write(connfd, buf ,strlen(buf)+1)) == -1){
            errexit (" Error: write()" );
        } 
        // memset (buf, 0, BUFSIZE);
        /* close client socket */
        close (connfd) ;
    }
    return 0;   
}
