#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>


//Defining Colors 
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

// Declaring the data structures required
struct DataPackage_HEADER
{
    unsigned short int startID;
    unsigned char clientID;
    unsigned short int data;
    unsigned char segmentNo;
    unsigned char length;
} header = {0xFFFF, 0xFF, 0xFFF1, 0x0, 0xFF};
struct DataPackage_ENDER
{
    unsigned short int endID;
} ender = {0xFFFF};
struct ACK_PACKAGE
{
    unsigned short int startID;
    unsigned char clientID;
    unsigned short int ack;
    unsigned char segmentNo;
    unsigned short int endID;
} ack = {0xFFFF, 0xFF, 0xFFF2, 0x0, 0xFFFF};
struct Reject_PACKAGE
{
    unsigned short int startID;
    unsigned char clientID;
    unsigned short int reject;
    unsigned short int rejectCode;
    unsigned char segmentNo;
    unsigned short int endID;
} reject = {0xFFFF, 0xFF, 0xFFF3, 0xFFF4, 0x0, 0xFFFF};

// Displaying Packet Content
void printPackage()
{
    printf(RED "\n************ Package %d ************\n" RESET,
           header.segmentNo);
    printf("Packet ID =============\"%#X\"\n", header.startID);
    printf("Client ID =============\"%#X\"\n", header.clientID);
    printf("DATA ==================\"%#X\"\n", header.data);
    printf("Sequence Number =======\"%d\"\n", header.segmentNo);
    printf("Length of payload =====\"%#X\"\n", header.length);
    printf("End of Packet ID ======\"%#X\"\n", ender.endID);
    printf(RED "************ End of Package ************\n\n" RESET);
}
void printAck()
{
    printf(RED "\n************ Ack %d ************\n" RESET, ack.segmentNo);
    printf("Packet ID ==================\"%#X\"\n", ack.startID);
    printf("Client ID ==================\"%#X\"\n", ack.clientID);
    printf("ACK ========================\"%#X\"\n", ack.ack);
    printf("Received Sequence Number ===\"%d\"\n", ack.segmentNo);
    printf("End of Packet ID ===========\"%#X\"\n", ack.endID);
    printf(RED "************ End of Ack ************\n\n" RESET);
}
void printReject()
{
    printf(RED "\n************ Reject Package %d ************\n" RESET,
           reject.segmentNo);
    printf("Packet ID ===================\"%#X\"\n", reject.startID);
    printf("Client ID ===================\"%#X\"\n", reject.clientID);
    printf("REJECT ======================\"%#X\"\n", reject.reject);
    printf("Reject sub code =============\"%#X\"\n", reject.rejectCode);
    printf("Received Sequence Number ====\"%d\"\n", reject.segmentNo);
    printf("End of Packet ID ============\"%#X\"\n", reject.endID);
    printf(RED "************ End of Reject Package ************\n\n" RESET);
}

// Package variables and info
void reject_builder(unsigned short int code)
{
    reject.startID = 0xFFFF;
    reject.clientID = header.clientID;
    reject.reject = 0xFFF3;
    reject.rejectCode = code;
    reject.segmentNo = header.segmentNo;
    reject.endID = 0xFFFF;
}
void error(char *msg)
{
    perror(msg);
    exit(1);
}

char buffer[255];
char package[1024];
char reply[16];

int main(int argc, char *argv[])
{
    int sock;
    int length;
    int n;
    int counter = 0;
    struct sockaddr_in server; //<netinet/in.h>
    struct sockaddr_in from;
    socklen_t fromlen;
    
    if (argc != 2)
    {
        fprintf(stderr, "Type: ./server port_number\n"); //stderr is in stdio.h
        exit(1);
    }

    //creating socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        error("Opening socket");
    }
    length = sizeof(server);
    bzero(&server, length);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));
    //binding socket
    if (bind(sock, (struct sockaddr *)&server, length) < 0)
    {
        error("binding");
    }
    fromlen = sizeof(struct sockaddr_in);
    //listening to client && sending ACKs
    printf(YEL "Server has been deployed and is running successfully\n" RESET);

    while (1)
    {
        n = recvfrom(sock, package, 1024, 0, (struct sockaddr *)&from, &fromlen);
        if (n < 0)
        {
            error("recvfrom");
        }
        //get the first 7 bytes as header
        memcpy(&header, package, sizeof(header));

        //get payload
        memcpy(&buffer, package + sizeof(header), n - sizeof(header) - 2);
        buffer[strlen(buffer)] = '\0';
        //get the last 2 bytes as ender
        memcpy(&ender, package + n - 2, 2);
        //display the package information
        printf("\n\nReceived a package: \n");
        printf("Packet number: %d \n", header.segmentNo);
        printf("Package size: %d\n", n);
        printf("Content: %s", buffer);
        printPackage();
        printf("\n\n");
        //package analysis and error handling
        if (header.segmentNo == counter)
        {
            //duplicate package
            printf(YEL "Server: Package Rejected. \nError: Duplicated Packet\n" RESET);
            reject_builder(0xFFF7);
            printReject();
            bzero(reply, 16);
            memset(reply, 0, 16);
            memcpy(reply, &reject, sizeof(reject));
            n = sendto(sock, &reply, sizeof(reject), 0, (struct sockaddr *)&from, fromlen);
            if (n < 0)
            {
                error(RED "Server: Failed to send an ack.\n" RESET);
            }
        }
        else if (header.segmentNo != counter + 1)
        {
            //out of sequence
            printf(YEL "Server: Package Rejected. \nError: Out of sequence\n" RESET);
            reject_builder(0xFFF4);
            printReject();
            bzero(reply, 16);
            memset(reply, 0, 16);
            memcpy(reply, &reject, sizeof(reject));
            n = sendto(sock, &reply, sizeof(reject), 0, (struct sockaddr *)&from, fromlen);
            if (n < 0)
            {
                error(RED "Server: Failed to send an ack.\n" RESET);
            }
        }
        else if (header.length != strlen(buffer))
        {
            //length mismatch
            printf(YEL "Server: Package Rejected. \nError: Length mismatch\n" RESET);
            reject_builder(0xFFF5);

            printReject();
            bzero(reply, 16);
            memset(reply, 0, 16);
            memcpy(reply, &reject, sizeof(reject));
            n = sendto(sock, &reply, sizeof(reject), 0, (struct sockaddr *)&from, fromlen);
            if (n < 0)
            {
                error(RED "Server: Failed to send an ack.\n" RESET);
            }
        }
        else if (ender.endID != 0xFFFF)
        {
            //end if packet missing
            printf(YEL "Server: Package Rejected. \nError: End of packet missing\n" RESET);
            reject_builder(0xFFF6);
            printReject();
            bzero(reply, 16);
            memset(reply, 0, 16);
            memcpy(reply, &reject, sizeof(reject));
            n = sendto(sock,&reply,sizeof(reject),0,(struct sockaddr
            *)&from,fromlen);
            if (n < 0)
            {
                            error(RED "Server: Failed to send an ack.\n" RESET);
            }
        }
        else
        {
            printf(YEL "Server: Package accepted. Sending an ack.\n" RESET);
            counter++;
            ack.startID = 0xFFFF;
            ack.clientID = header.clientID;
            ack.ack = 0xFFF2;
            ack.segmentNo = header.segmentNo;
            ack.endID = 0xFFFF;
            printf("sizeof(ack) == %lu\n", sizeof(ack));
            printAck();
            bzero(reply, 16);
            memset(reply, 0, 16);
            memcpy(reply, &ack, sizeof(ack));
            n = sendto(sock, &reply, sizeof(ack), 0, (struct sockaddr *)&from, fromlen);
            if (n < 0)
            {
                error(RED "Server: Failed to send an ack.\n" RESET);
            }
            //Reset the server counter for testing purposes!
            if (counter == 5)
            {
                printf(YEL"\n\n\n\nNow reseting the packet counter for testing other cases\n\n\n\n");
                counter = 0;
            }
        }
    }


    return 0;
}