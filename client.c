#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

//Defining Colors for output
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
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


void printPackage()
{
    printf(RED "\n************ Package %d ************\n" RESET,
           header.segmentNo);
    printf("Packet ID =================\"%#X\"\n", header.startID);
    printf("Client ID =================\"%#X\"\n", header.clientID);
    printf("DATA ======================\"%#X\"\n", header.data);
    printf("Sequence Number ===========\"%d\"\n", header.segmentNo);
    printf("Length of payload =========\"%#X\"\n", header.length);
    printf("End of Packet ID ==========\"%#X\"\n", ender.endID);
    printf(RED "************ End of Package ************\n\n" RESET);
}

void printAck()
{
    printf(RED "\n************ Ack %d ************\n" RESET, ack.segmentNo);
    printf("Packet ID =====================\"%#X\"\n", ack.startID);
    printf("Client ID =====================\"%#X\"\n", ack.clientID);
    printf("ACK ===========================\"%#X\"\n", ack.ack);
    printf("Received Sequence Number ======\"%d\"\n", ack.segmentNo);
    printf("End of Packet ID ==============\"%#X\"\n", ack.endID);
    printf(RED "************ End of Ack ************\n\n" RESET);
}

void printReject()
{
    printf(RED "\n************ Reject Package %d ************\n" RESET,
           reject.segmentNo);
    printf("Packet ID =====================\"%#X\"\n", reject.startID);
    printf("Client ID =====================\"%#X\"\n", reject.clientID);
    printf("REJECT ========================\"%#X\"\n", reject.reject);
    printf("Reject sub code ===============\"%#X\"\n", reject.rejectCode);
    printf("Received Sequence Number ======\"%d\"\n", reject.segmentNo);
    printf("End of Packet ID ==============\"%#X\"\n", reject.endID);
    printf(RED "************ End of Reject Package ************\n\n" RESET);
}

int sock;
struct sockaddr_in server;  //<netinet/in.h>
struct sockaddr_in from; 
struct hostent *hp;   //  <netdb.h>
socklen_t length; // <socket.h>
struct timeval timeout; //Inbuilt
int packageNo;
char payload[256];
char package[264];
int counter;
int n;
char reply[16];

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void send_package()
{
    //try 3 times or quit
    counter = 3;
    while (counter >= 0)
    {
        //sending package to server
        if (counter == 3)
        {
            printf("\nSending message: %s\n", payload);
        }
        else
        {
            printf("\nSending message failed. Trying to resend");
            printf("\nSending message: %s\n", payload);
        }
        // sendto is defined in socket.h, 
        n = sendto(sock, package, sizeof(header) + strlen(payload) + 2, 0, (struct sockaddr *)&server, length);
        if (n < 0)
        {
            error("Client: Failed to send the package.\n");
        }
        //waiting for ack
        bzero(reply, 16);
        n = recvfrom(sock, reply, 16, 0, (struct sockaddr *)&from, &length);
        if (n >= 0)
        {
            memcpy(&ack, reply, sizeof(ack));
            if (ack.ack == 0xFFF2)
            {
                printf(GRN "Received an ack.\n " RESET);
                printAck();
                break;
            }
            else
            {
                memcpy(&reject, reply, sizeof(reject));
                printf(GRN "Received a reject package.\n " RESET);
                printf(YEL "Error Message: " RESET);
                if (reject.rejectCode == 0xFFF4)
                {
                    printf(YEL "Out of sequence.\n");
                }
                else if (reject.rejectCode == 0xFFF5)
                {
                    printf(YEL "Length mismatch.\n");
                }
                else if (reject.rejectCode == 0xFFF6)
                {
                    printf(YEL "End of packet missing.\n");
                }
                else if (reject.rejectCode == 0xFFF7)
                {
                    printf(YEL "Duplicate packet.\n");
                }
                else
                {
                    printf(YEL "Unknown error.\n");
                }
                printReject();
                counter++;
                break;
            }
        }
        counter--;
    }
    if (counter < 0)
    {
        error(RED "Error: Server does not respond\n" RESET);
    }
}

void package_builder(int seg, char *payload)
{
    header.segmentNo = seg;
    header.length = strlen(payload);
    memset(package, 0, 264);
    memcpy(package, &header, sizeof(header));
    memcpy(package + sizeof(header), payload, strlen(payload));
    memcpy(package + sizeof(header) + strlen(payload), &ender, 2);
    printf("Package created:");
    printPackage();
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
    //To set options at the socket level, specify the level argument as SOL_SOCKET
    //Sets the timeout value that specifies the maximum amount of time an input function waits until it completes. 
    //It accepts a timeval structure with the number of seconds and microseconds specifying the limit on how long 
    //to wait for an input operation to complete
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    //sending 5 error-free packages
    printf(CYN "\n\nTest1: Send 5 error-free packages in sequence\n\n" RESET);
    packageNo = 1;
    while(packageNo <= 5){
        //creating packages
        bzero(payload, 256);
        memcpy(payload, "Payload of a package.\n", 34);  // void *memcpy(void *dest, const void * src, size_t n)
        package_builder(packageNo, payload); 
        send_package();
        packageNo++;
    }
    printf(GRN "\n\n\n\nNow reseting the package counter for Test2\n\n\n\n" RESET);

    printf(CYN "\n\nTest2: Sending 5 packages to test errors(1 Correct and 4 error packages)\n\n" RESET);

    //package 1 : correct package
    printf(BLU "Package 1: Correct package\n" RESET);
    bzero(payload, 256);
    memcpy(payload, "Payload of a package.\n", 34);
    package_builder(1, payload);
    send_package();

    //package 2:test duplicate error
    printf(BLU "Package 2: Simulating duplicate packet error\n" RESET);
    bzero(payload, 256);
    memcpy(payload, "Payload of a package.\n", 34);
    package_builder(1, payload);
    send_package();

    //package 3:test out of sequence error
    printf(BLU "Package 3: Simulating out of sequence error\n" RESET);
    bzero(payload, 256);
    memcpy(payload, "Payload of a package.\n", 34);
    package_builder(3, payload);
    send_package();

    //package 4: test length mismatch error
    printf(BLU "Package 4: Simulating length mismatch error\n" RESET);

    // Building an error package
    bzero(payload, 256);
    memcpy(payload, "Payload of a package.\n", 34);
    header.segmentNo = 2;
    header.length = 0x44;
    memset(package, 0, 264);
    memcpy(package, &header, sizeof(header));
    memcpy(package + sizeof(header), payload, strlen(payload));
    memcpy(package + sizeof(header) + strlen(payload), &ender, 2);
    printf("Package created:");
    printPackage();
    send_package();

    //package 5:test end of packet missing error
    printf(BLU "Package 5: Simulating end of packet missing error\n" RESET);
    bzero(payload, 256);
    memcpy(payload, "Payload of a package.\n", 34);
    package_builder(2, payload);
    ender.endID = 0xFFF0;
    memcpy(package + sizeof(header) + strlen(payload), &ender, 2);
    printf(GRN "Package altered: end of packet ID changed " RESET);
    printPackage();
    send_package();

    return 0;
}
