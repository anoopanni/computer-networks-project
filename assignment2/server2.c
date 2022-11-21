// COEN 233 assignment2
// Name : Anoop Shivayogi
// Student ID : W1648523

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Defining the port Number and length of packet
#define PORT 8081
#define LENGTH 10

// Reject Codes for the 4 Case  
#define PAID                     0XFFFB
#define NOTPAID                  0XFFF9
#define DOESNOTEXIST             0XFFFA
#define TECHNOLOGYMISMATCH       0XFFFC

#define RED    "\x1B[31m"
#define CYN    "\x1B[32m"
#define MAG    "\x1B[33m"
#define BLU    "\x1B[34m"
#define YEL    "\x1B[35m"
#define GRN    "\x1B[36m"
#define WHITE  "\x1B[37m"
#define RESET  "\x1B[0m"

// Storing the Subscriber 
struct SubscriberDatabase {
	unsigned long subscriberNumber;
	uint8_t technology;
	int status;
};

// Datatypes Response Packet Structure
struct ResponsePacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned long SourceSubscriberNo;
	uint16_t endpacketID;
};

// Dataype to Request Packet Structure
struct RequestPacket{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t Acc_Per;
	uint8_t segment_No;
	uint8_t length;
	uint8_t technology;
	unsigned long long SourceSubscriberNo;
	uint16_t endpacketID;
};

// Dataypes to Create Response Packet
struct ResponsePacket generateResponsePacket(struct RequestPacket requestPacket) {
	struct ResponsePacket responsePacket;
	responsePacket.packetID = requestPacket.packetID;
	responsePacket.clientID = requestPacket.clientID;
	responsePacket.segment_No = requestPacket.segment_No;
	responsePacket.length = requestPacket.length;
	responsePacket.technology = requestPacket.technology;
	responsePacket.SourceSubscriberNo = requestPacket.SourceSubscriberNo;
	responsePacket.endpacketID = requestPacket.endpacketID;
	return responsePacket;
}

// Display the Packet Content
void displayPacket(struct RequestPacket requestPacket ) {
	printf(YEL "Packet ID ================ %x\n",requestPacket.packetID);
	printf(YEL "Client ID ================ %hhx\n",requestPacket.clientID);
	printf("Access Permission ======== %x\n",requestPacket.Acc_Per);
	printf("Segment no =============== %d \n",requestPacket.segment_No);
	printf("Length of the Packet ===== %d\n",requestPacket.length);
	printf("Technology =============== %d \n", requestPacket.technology);
	printf("Subscriber no ============ %llu \n",requestPacket.SourceSubscriberNo);
	printf("End of RequestPacket ID == %x \n",requestPacket.endpacketID);
}

//  Checking whether Subscriber's data exists in Database or not
int check(struct SubscriberDatabase subscriberDatabase[],unsigned int subscriberNumber,uint8_t technology) {
	int value = -1;
	for(int j = 0; j < LENGTH;j++) {
		if(subscriberDatabase[j].subscriberNumber == subscriberNumber && subscriberDatabase[j].technology == technology) {
			return subscriberDatabase[j].status;
		}
                else if (subscriberDatabase[j].subscriberNumber == subscriberNumber && subscriberDatabase[j].technology != technology)
                        return 2;
	}
	return value;
}

// Reading file from the Subscriber Database
void readFile(struct SubscriberDatabase subscriberDatabase[]) {

	// Read file and store the contents
	char line[30];
	int i = 0;
	FILE *filePointer;

	filePointer = fopen("Verification_Database.txt", "rt");

	if(filePointer == NULL)
	{
		printf("File could not be opened!\n");
		return;
	}
	while(fgets(line, sizeof(line), filePointer) != NULL)
	{
		char * words=NULL;
		words = strtok(line," ");
		subscriberDatabase[i].subscriberNumber =(unsigned) atol(words);     // long int
		words = strtok(NULL," ");
		subscriberDatabase[i].technology = atoi(words);						// int
		words = strtok(NULL," ");
		subscriberDatabase[i].status = atoi(words);
		i++;
	}
	fclose(filePointer);
}


int main(int argc, char**argv)
{
    struct RequestPacket requestPacket;
	struct ResponsePacket responsePacket;
	struct SubscriberDatabase subscriberDatabase[LENGTH];
	readFile(subscriberDatabase);

    int sockfd,n;
	struct sockaddr_in serverAddress;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	sockfd=socket(AF_INET,SOCK_DGRAM,0);

    bzero(&serverAddress,sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr=htonl(INADDR_ANY);
	serverAddress.sin_port=htons(PORT);
	
    bind(sockfd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));
	addr_size = sizeof serverAddress;
	printf(RED "Server has been deployed and is running successfully\n" RESET);

    for (;;) 
    {
        // Receieve the packet
		n = recvfrom(sockfd,&requestPacket,sizeof(struct RequestPacket),0,(struct sockaddr *)&serverStorage, &addr_size);
		displayPacket(requestPacket);
		if(requestPacket.segment_No == 11) 
        {
			exit(0);
		}

		if(n > 0 && requestPacket.Acc_Per == 0XFFF8) 
        {
			// Generating the response packet
			responsePacket = generateResponsePacket(requestPacket);

			int value = check(subscriberDatabase,requestPacket.SourceSubscriberNo,requestPacket.technology);
			if(value == 0) 
            {
				
				responsePacket.type = NOTPAID;
				printf(GRN "Subscriber has not paid for the subscription.\n" RESET);
			}
			else if(value == 1) 
            {
				
				printf(GRN "Subscriber has paid for the subscription.\n" RESET);
				responsePacket.type = PAID;
			}

			else if(value == -1) 
            {
               
				printf(GRN "Subscriber does not exist.\n" RESET);
				responsePacket.type = DOESNOTEXIST;
			}
                        
            else
            {                  
                printf(GRN "Subscriber's technology does not match.\n" RESET);
            	responsePacket.type = TECHNOLOGYMISMATCH;
            }                        
			// To send the packet for response
			sendto(sockfd,&responsePacket,sizeof(struct ResponsePacket),0,(struct sockaddr *)&serverStorage,addr_size);
		}
		n = 0;
		printf(RED" \n *********** Start of a New Packet *********** \n" RESET);
	}

    return 0;
}