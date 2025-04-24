

#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>

#define MAXDATASIZE 1000

void *get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    int min = 22220;
    int max = 22230;
    int *port = malloc(sizeof(int)); //Select desired port in range
    printf("%s%d%s%d%s", "Select desired port (", min, "-", max, ")\n");
    scanf("%d", port);
    if(testport(port, 22220, 22230) < 0) {
        perror("Port out of bounds...\n");
        exit(1);
    }
    char *portchar = malloc(sizeof(port));
    snprintf(portchar, sizeof(port), "%d", *port);

    int sockfd;
    //char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    printf(portchar);

    if((rv = getaddrinfo(argv[1], portchar, &hints, &servinfo))!=0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p!=NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))==-1) {
            perror("client: socket");
            continue;
        }

        if(connect(sockfd, p->ai_addr, p->ai_addrlen)==-1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }
    printf("%d",sockfd);
    freeaddrinfo(servinfo);

    if(p==NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting %s\n", s);

    if(send(sockfd, "Test", 5, 0)==-1) {
        perror("send");
    }

    return 0;
}