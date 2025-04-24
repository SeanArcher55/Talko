

#include "server.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define NUM_THREADS 25

struct Server server;
pthread_t threads[NUM_THREADS];
int tid;
int server_addrs[30];

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *ReceiveMessage(void* args) {
    int sockfd = (int)(__intptr_t)args;
    printf("%s%d%s","Receiving messages from address ", sockfd,".\n");
    while(1) {
        char *buf = malloc(sizeof(char) * 10000);
        int recvVal = recv(sockfd, (char *)buf, 1000, 0);
        if(recvVal > 0) {
            printf(buf);
            for(int i = 0; i < sizeof(server_addrs)/sizeof(int); i++) {
                if(server_addrs[i]!=0&&server_addrs[i]!=sockfd) {
                    int sendSucc = send(server_addrs[i], (char *)buf, strlen(buf), 0);
                    if(sendSucc == -1) {
                        printf("%s%d\n", "Failed send to ", server_addrs[i]);
                    }
                }
            }
        }
        if(recvVal==0) {
            for(int i = 0; i < sizeof(server_addrs)/sizeof(int); i++) {
                if(server_addrs[i]==sockfd) {
                    server_addrs[i] = 0;
                }
            }
            tid--;
            break;
        }
        if(recvVal == -1) {
            printf("%s%d%s","Error recieving message from ", sockfd, ".\n");
        } 
    }
    printf("%s%d%s","Lost connection with address ", sockfd, ".\n");
    return NULL;
}

void *AcceptConnections() {
    int tid = 1;
    struct sockaddr_in their_addr; // connector's address information
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    printf("%s%d%s","Accepting connections on thread ", tid,".\n");
    while(1) {
        int *new_fd = malloc(sizeof(int));
        sin_size = sizeof(their_addr);
        *new_fd = accept(server.socket, (struct sockaddr *)&their_addr, &sin_size);
        if(*new_fd < 0 || tid >= 12) {
            perror("Couldn't accept incoming connection...\n");
            continue;
        }
        
        inet_ntop(their_addr.sin_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("Connection established from %s\n", s);
        if(tid < 20) {
            pthread_create(&threads[tid], NULL, ReceiveMessage, (void *)(__intptr_t)new_fd);
            printf("Check 4\n");
            server_addrs[tid-1] = *new_fd;
            tid++;
        }
    }
    return NULL;
}

void launch(struct Server* server)
{
    pthread_t threads[NUM_THREADS];
    pthread_create(&threads[0], NULL, AcceptConnections, (void *)0);
    pthread_exit(NULL);
}

int main() {
    int min = 22220;
    int max = 22230;
    int *port = malloc(sizeof(int)); //Select desired port in range
    printf("%s%d%s%d%s", "Select desired port (", min, "-", max, ")\n");
    scanf("%d", port);
    if(testport(port, 22220, 22230) < 0) {
        perror("Port out of bounds...\n");
        exit(1);
    }

    tid = 1;
    for(int i = 0; i < sizeof(server_addrs)/sizeof(int); i++) {
        if(i > 30) {
            exit(31);
        }
        server_addrs[i] = 0;
    }

    server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, *port, 10, launch);
    server.launch(&server);
}