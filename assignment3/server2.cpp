#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include <event2/event.h>

using namespace std;
#define PORT "4000" // port users will be connecting to
#define MAXDATASIZE 1024
#define backlog 10 // maximum number of connections to be queued up by the server at a time

int num = -1;
typedef struct args
{
    int fd;
    string buf;
} args;
int sockfd;
int clientfd[backlog];
pthread_t thread1[backlog * 2];
void sigint_handler(int signum, siginfo_t *info, void *context)
{
    close(sockfd);
    int i;
    for (i = 0; i < num; i++)
    {
        close(clientfd[i]);
    }
    // for (i = 0; i < 2 * backlog; i++)
    // {
    //     pthread_cancel(thread1[i]);
    // }
    printf("socket closed");
    exit(0);
}
// getting sockaddr for IPv4 or IPv6
void *get_in_address(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
    // note that here we are type casting the socket address to the IPV6 or IPV4 before obtaining the desired address
}
void *chat1(void *var);
// prototype
void strip_leading_whitespaces(char *str)
{
    int i = 0, j = 0;

    while (isspace(str[i]))
    {
        i++;
    }

    while (str[i] != '\0')
    {
        str[j++] = str[i++];
    }

    str[j] = '\0';
}
void *chat(void *var)
{
    int newfd = *((int *)var);
    int numbytes;
    char buf[MAXDATASIZE];
    int i;
    for (i = 0; i < backlog; i++)
    {
        if (clientfd[i] == newfd)
        {
            break;
        }
    }
    pthread_t thread;

    while (1)
    {
        if ((numbytes = recv(newfd, buf, MAXDATASIZE - 1, 0)) == -1)
        { // buf receives the message
            perror("recv");
            exit(1);
        }
        if (numbytes == 0)
        {
            close(sockfd);
            break;
        }

        buf[numbytes] = '\0';
        printf("server : received '%s' \n", buf);
        strip_leading_whitespaces(buf);
        int p = buf[0] - '0';
        args m;
        m.fd = clientfd[p];
        if (p < 0 && p > 9 && p > num)
        {
            char s[] = "Client does not exist ";
            strcpy(buf, s);
            m.fd = newfd;
            continue;
        }
        buf[0] = (char)(i + 48);

        m.buf = buf;
        pthread_create(&thread, NULL, chat1, (void *)&m);
    }
    return NULL;
}
void *chat1(void *var)
{
    int sockfd = ((args *)var)->fd;

    if (send(sockfd, (((args *)var)->buf).c_str(), (((args *)var)->buf).length(), 0) == -1)
    {
        perror("send :");
    }
    return NULL;
}
int main(void)
{
    int newfd; // listen on sock_fd, newfd file descriptor returned by accept after new connection
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // client address information
    socklen_t sin_size;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    // memset sets all bits of hints to 0

    struct sigaction action;
    action.sa_sigaction = sigint_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;

    if (sigaction(SIGINT, &action, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }
    hints.ai_family = AF_UNSPEC;     // can be either IPV4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // specifies tcp ip connection
    hints.ai_flags = AI_PASSIVE;
    // last flag is interesting AI_Passive fill the address in hints with the local address

    rv = getaddrinfo(NULL, PORT, &hints, &servinfo);
    if (rv != 0)
    {
        fprintf(stderr, "getaddrinfo : %s \n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
        {
            perror(" server : socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);
    if (p == NULL)
    {
        fprintf(stderr, "server : failed to bind \n");
        exit(1);
    }
    if (listen(sockfd, backlog) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections ... \n");

    int i = 0;
    num = 0;
    while (1)
    { // acept loop
        sin_size = sizeof their_addr;
        newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        clientfd[i % backlog] = newfd;
        inet_ntop(their_addr.ss_family, get_in_address((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s \n", s);
        pthread_create(&thread1[i % 10], NULL, chat, (void *)&newfd);
        string buf = "you are client " + to_string(i);
        send(newfd, buf.c_str(), buf.length(), 0);
        if (num < i)
        {
            num = i;
        }
        i += 1;
        // close(newfd);
    }
    return 0;
}