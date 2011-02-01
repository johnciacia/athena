#include "network_utils.h"

/*
    Establish a tcp connection with `host` on port `service`
    Return a socket file discriptor on success and -1 on failure
*/
int tcp_connect(const char *host, const char *service)
{
    int sock_fd, n;
    struct addrinfo hints, *res, *p;

    memset((void*) &hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(host, service, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(n));
        return -1;
    }

    for(p = res; p != NULL; p = p->ai_next) {
        if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("tcp_connect: socket\n");
            continue;
        }

        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock_fd);
            perror("tcp_connect: connect\n");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "tcp_connect error for %s, %s\n", host, service);
        return -1;
    }

    freeaddrinfo(res);
    return sock_fd;
}

/*
    Create a passive socket on the specified port
    Return -1 on failure 
*/
int create_tcp_passive_socket(const char *port) {
    int listen_fd, n;
    const int on = 1;
    struct addrinfo hints, *res, *p;

    memset((void *) &hints, 0, sizeof(struct addrinfo));
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(n));
        return -1;
    }

    for(p = res; p != NULL; p = p->ai_next) {
        if ((listen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("create_tcp_passive_socket: socket\n");
            continue;
        }

        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
            perror("create_tcp_passive_socket: setsockopt\n");
            return -1;
        }

        if (bind(listen_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(listen_fd);
            perror("create_tcp_passive_socket: bind\n");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "tcp_listen(): cannot bind the socket to port %s\n", port);
        freeaddrinfo(res);
        return -1;
    }

    if (listen(listen_fd, BACKLOG) < 0) {
        freeaddrinfo(res);
        fprintf(stderr, "error starting listening\n");
        return -1;
    }

    freeaddrinfo(res);
    return listen_fd;
}


ssize_t readn(int fd, void* vptr, size_t n)
{
  size_t  nleft;
  ssize_t nread;
  char*   ptr;
  
  ptr = vptr;
  nleft = n;
  while (nleft > 0) {           /* keep reading upto n bytes     */
    if ( (nread = read(fd, ptr, nleft)) < 0) {
      if (errno == EINTR) {     /* got interrupted by a signal ? */
        nread = 0;              /* try calling read() again      */
      } else {
        return(-1);
      }
    } else if (nread == 0) {
      break;                    /* EOF */
    }
    nleft -= nread;
    ptr   += nread;
  }

  return(n - nleft);            /* return >= 0 */
}


void print_packet(const char *buff, int size) {
    int count = 1;
    uint8_t chars[20];
    uint x;
    for(x = 0; x < size; x++) {
        if(buff[x]>=0 && buff[x]<= 15) {
            printf("0%X ", 0xFF & buff[x]);
        } else {
            printf("%X ", 0xFF & buff[x]);
        }

        if(buff[x] >= 32 && buff[x] <= 126) {
            chars[(count-1)%16] = buff[x];
        } else {
            chars[(count-1)%16] = '.';
        }

        if((count%16) == 0 && x != 0) {
            chars[16] = '\0';
            printf("%s\n", chars);
        }
        count++;
    }

    uint fillspaces = (16-((count-1)%16))*3;
    for ( x=0; x<fillspaces; x++) {
          printf(" ");
    }
    chars[(count-1)%16] = '\0';
    printf("%s\n",chars);
}

/*
char *get_local_ip()
{
    static char local_ip_addr[INET_ADDRSTRLEN];
    int n;
    char myhostname[100];
    struct sockaddr_in sa;
    struct addrinfo hints, *res;

    memset((void *) &hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;

    if (gethostname(myhostname, 100) < 0) {
        printf("gethostname() failed");
        return -1;
    }

    if ((n = getaddrinfo(myhostname, NULL, &hints, &res)) != 0) {
       printf("getaddrinfo() failed for host %s\n", myhostname);
       return -1;
    }

    memcpy(&sa, res->ai_addr, sizeof(sa));

    if (inet_ntop(AF_INET, &sa.sin_addr, local_ip_addr, INET_ADDRSTRLEN) == NULL) {
        printf("get_local_ip(): can't convert my ip to its presentation format");
        return -1;    
    }

    return local_ip_addr;	
}
*/
