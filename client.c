#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

int sock;
struct sockaddr_in server;  //<netinet/in.h>
struct hostent *hp;
socklen_t length; // <socket.h>
struct timeval timeout;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){

    if (argc != 3)
    {
        printf("Use format: ./client server_name port_number(ex: localhost 3333)\n");
        exit(1); 
    }
    // Create a Socket
    // SOCK_DGRAM: UDP connectionless protocol | SOCK_STREAM is for TCP connection-oriented protocol
    sock = socket(AF_INET, SOCK_DGRAM, 0);  //AddressFamily = ipv4 for INET and AF_INET6 for ipv6  
    if (sock < 0)
    {
        error("Client: Failure while creating socket.\n");
    }
    server.sin_family = AF_INET;
    printf("%s",argv[1]);
    hp = gethostbyname(argv[1]);
    if (hp == 0)
    {
        error("Client: Unknown host.\n");
    }
    bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    //short integer from host byte order to network byte order
    server.sin_port = htons(atoi(argv[2]));   //atoi converts str to integer
    length = sizeof(struct sockaddr_in);
     //setting timer
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    

    return 0;
}
