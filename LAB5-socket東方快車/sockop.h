#ifndef _SOCKOP_H_
#define _SOCKOP_H_

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h> // Network related functions, e.g. gethostbyname()
#include <netinet/in.h> // struct sockaddr_in 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // system socket define
#include <sys/types.h> // system types
#include <sys/wait.h>

#define errexit(format,arg...) exit(printf(format,##arg))

/*Create server*/
int passivesock(const char *service, const char *transport, int qlen);

/*Connect to server*/
int connectsock(const char *host, const char *service, const char *transport);

#endif /*_SOCKOP_H_*/

