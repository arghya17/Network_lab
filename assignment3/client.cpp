#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
using namespace std;
#define PORT "4000" // port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

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

int main()
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    string k;
    cout << "Enter the ip address of server ";
    cin >> k;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(k.c_str(), PORT, &hints, &servinfo)) != 0)
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
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect \n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_address((struct sockaddr *)p->ai_addr), s, sizeof s);
    cout << " Client connecting to " << s << "\n";

    freeaddrinfo(servinfo);
    string message;
    while (1)
    {
        cout << "Enter message to be sent : \n";
        cin >> message;
        if (send(sockfd, message.c_str(), message.length(), 0) == -1)
        {
            perror("send :");
        }
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        { // buf receives the message
            perror("recv : ");
            exit(1);
        }

        buf[numbytes] = '\0';
        printf("client : received '%s' \n", buf);
    }
    close(sockfd);
    return 0;
}