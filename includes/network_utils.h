#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <sys/socket.h>  /* basic socket definitions               */
#include <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include <netdb.h>       /* name and address conversion            */
#include <arpa/inet.h>   /* inet_pton/ntop                         */
#include <unistd.h>      /* read, write, close, exec, etc.         */
#include <errno.h>       /* error handlings                        */
#include <string.h>      /* strtok, mem*, and stuff                */
#include <stdio.h>       /* fgets, etc., perror,                   */
#include <signal.h>      /* signal handling                        */
#include <sys/wait.h>    /* wait, waitpid, etc.                    */

#define BACKLOG 20


int create_tcp_passive_socket(const char* port);
ssize_t readn(int fd, void* vptr, size_t n);
int tcp_connect(const char *host, const char *service);
char *get_local_ip();
void print_packet(const char *buff, int size);
#endif
