#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>

using namespace std;
#define PORT "4000" //port users will be connecting to 

#define backlog 10 //maximum number of connections to be queued up by the server at a time

// getting sockaddr for IPv4 or IPv6
void * get_in_address(struct sockaddr * sa)
{
    if(sa->sa_family ==AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
    //note that here we are type casting the socket address to the IPV6 or IPV4 before obtaining the desired address

}

void* chat(void *var){
    int newfd=*((int *)var);
    if(send(newfd, "Hello world !", 14, 0)==-1){
        perror("send");
    }
    close(newfd);
    return NULL;
}
int main(void)
{
    int sockfd, newfd; //listen on sock_fd, newfd file descriptor returned by accept after new connection
    struct addrinfo hints , *servinfo, *p;
    struct sockaddr_storage their_addr; //client address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    //memset sets all bits of hints to 0

    hints.ai_family= AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM; //specifies tcp ip connection
    hints.ai_flags=AI_PASSIVE;
    //last flag is interesting AI_Passive fill the address in hints with the local address 

    rv= getaddrinfo(NULL, PORT, &hints, &servinfo);
    if(rv!=0)
    {
        fprintf(stderr, "getaddrinfo : %s \n",gai_strerror(rv));
        return 1;
    }

    for(p=servinfo; p!=NULL; p=p->ai_next)
    {
        sockfd= socket(p->ai_family,p->ai_socktype, p->ai_protocol);
        if(sockfd==-1)
        {
            perror(" server : socket");
            continue;
        }
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1){
            perror("setsockopt");
            exit(1);
        }
        if(bind(sockfd, p->ai_addr,p->ai_addrlen)==-1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);
    if(p==NULL)
    {
        fprintf(stderr, "server : failed to bind \n");
        exit(1);
    }
    if(listen(sockfd, backlog) ==-1)
    {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections ... \n");

    pthread_t thread1[backlog];
    int i=0;
    while(1){//acept loop
        sin_size=sizeof their_addr;
        newfd= accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        inet_ntop(their_addr.ss_family, get_in_address((struct sockaddr *)&their_addr), s , sizeof s);
        printf("server: got connection from %s \n", s);
        pthread_create(&thread1[i%10],NULL,chat,(void *)&newfd);
        // close(newfd);
    }
    return 0;
}