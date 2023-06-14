#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <string>

#include <arpa/inet.h>

#define PORT "33859" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("Client is up and running.\n");

    freeaddrinfo(servinfo); // all done with this structure

    // struct sockaddr * my_addr;
    // socklen_t * addrlen;

    // int getsock_check=getsockname(sockfd, (struct sockaddr *) &addr, &addrlen);
    // printf("Local port: %d\n", ntohs(addr.sin_port));

    // getsockname(sockfd,my_addr, addrlen);
    // std::cout<<my_addr->

    // if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    //     perror("recv");
    //     exit(1);
    // }

    std::string department;
    while(1)
        {printf("Enter Department Name: \n");
        std::cin>>department;
        // department="Art"; 
        // std::cout << "department size: " << department.size() << std::endl;
        if (send(sockfd, (char*)department.data(),department.size(), 0) < 0) {
            perror("send");
            std::cout << "send" << std::endl;
        }
        printf("Client has sent Department %s to Main Server using TCP. \n",(char*)department.data());

        // buf[numbytes] = '\0';

        // printf("client: received '%s'\n",buf);

        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) < 0) {
            perror("recv");
            std::cout << "recv" << std::endl;
            exit(1);
        }
        buf[numbytes] = '\0';
        if((std::string)(buf)=="Department Name: Not found"){
            printf("%s Not found\n",(char*)department.data());
        }
        else{
            printf("Client has received results from Main Server: \n");
            printf("%s is associated with backend server %s.\n",(char*)department.data(),buf);
            }
        printf("-----Start a new query-----\n");
        }

    close(sockfd);

    return 0;
}