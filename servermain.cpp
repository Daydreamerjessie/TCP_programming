#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <iostream>

#define PORT "33859"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold


#define MAXDATASIZE 100 // max number of bytes we can get at once 


void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    std::ifstream file("list.txt");
    std::string data;
    int flag=0;
    std::string id;
    std::unordered_map<std::string,std::string> map;
    std::string department;

    int num_backend =0;
    int num_depar = 0;
    std::unordered_map<int,int> num;

    while(file>>data){
        if(flag==0){
            if(num_backend>0){
                num.insert(std::make_pair(std::stoi(id),num_depar));
            }
            id=data;
            flag=1;
            num_backend+=1;
            num_depar=0;
            // std::cout<<id;
        }
        else{
            flag=0;
            std::stringstream is(data);
            while(getline(is,department,',')){
                // std::cout<<department<<std::endl;
                if(map.find(department)==map.end()){
                    map.insert(std::make_pair(department,id));
                    num_depar+=1;
                }
            }
        }
    }
    if(num_backend>0){
                num.insert(std::make_pair(std::stoi(id),num_depar));
            }
    // if (map.find("Art") != map.end()) {
    //     std::cout << map.find("Art") ->second << std::endl;
    // }
    // std::cout<<map.find("Accounting")->second;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    //  1. getaddrinfo(): set up structures
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        //2. socket(): return socket descriptor
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        //3. bind the port

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    //4. connect()
    //5. listen()

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("Main server is up and running.\n");

    printf("Main server has read the department list from list.txt.\n");

    printf("Total num of Backend Servers: %d\n",num_backend);

    for(auto &pair: num){
        printf("Backend Servers %d contains %d distinct departments\n",pair.first, pair.second);
    }
    int client = 0;

    while(1) {  // main accept() loop
        client+=1;
        sin_size = sizeof their_addr;
        // 6. accept()
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        // printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener

            //7. send() and recv()
            // if (send(new_fd, "Hello, world!", 13, 0) == -1)
            //     perror("send");
            
            int numbytes;
            char buf[MAXDATASIZE];
            while(1)
                {if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                    break;
                }
                buf[numbytes] = '\0';
                department=buf;
                int fd;
                if(new_fd>4){
                    fd=new_fd-4;
                }
                else{
                    fd=new_fd;
                }
                printf("Main server has received the request on Department %s from client %d using TCP over port 33859\n", (char*)department.data(),client);
                if(map.find(department)==map.end()){
                    printf("%s does not show up in backend server", (char*)department.data());
                    for(auto &pair: num){
                        printf(" %d,",pair.first);
                    }
                    printf("\nThe Main Server has sent “Department Name: Not found” to client %d using TCP over port 33859\n",client);
                    id="Department Name: Not found";
                }
                
                else{
                    id=map.find(department) ->second;
                    // printf("client: received '%s'\n",(char*)(map.find(department) ->second).data());
                    printf("%s shows up in backend server %s\n",(char*)department.data(),(char*)id.data());
                    printf("Main Server has sent searching result to client %d using TCP over port 33859\n\n",client);
                    }
                if (send(new_fd, (char*)id.data(), id.size(), 0) == -1){
                    perror("send");
                    break;
                    }
                }
                

            close(new_fd);
            exit(0);
        }
        //8. close()
        // close(new_fd);  // parent doesn't need this
    }

    return 0;
}