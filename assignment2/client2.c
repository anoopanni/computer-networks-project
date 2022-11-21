#include <stdint.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h> // exit()
#include <string.h>

// Defining the port Number and length of packet
#define PORT 8081
#define PAID 0XFFFB
#define NOTPAID 0XFFF9
#define NOTEXIST 0XFFFA
#define TECHNOLOGYMISMATCH 0XFFFC

#define RED	    "\x1B[31m"
#define CYN     "\x1B[32m"
#define MAG     "\x1B[33m"
#define BLU     "\x1B[34m"
#define YEL     "\x1B[35m"
#define GRN     "\x1B[36m"
#define WHITE   "\x1B[37m"
#define RESET   "\x1B[0m"

// Datatype to Response Packet Structure
struct ResponsePacket {
	uint16_t packetId;
	uint8_t clientId;
	uint16_t type;
	uint8_t segment_No;
	uint8_t LENGTH;
	uint8_t technology;
	unsigned long SourceSubscriberNo;
	uint16_t endPacket;
};

// Dataype to Request Packet Structure
struct RequestPacket {
	uint16_t packetId;
	uint8_t clientId;
	uint16_t Acc_Per;
	uint8_t segment_No;
	uint8_t LENGTH;
	uint8_t technology;
	unsigned long SourceSubscriberNo;
	uint16_t endPacket;
};

// Data for Requesting Packet
struct RequestPacket Initialize () {
	struct RequestPacket requestPacket;
	requestPacket.packetId = 0XFFFF;
	requestPacket.clientId = 0XFF;
	requestPacket.Acc_Per = 0XFFF8;
	requestPacket.endPacket = 0XFFFF;
	return requestPacket;

}

// To Print Packet Content
void DisplayPacket(struct RequestPacket requestPacket) {
	printf(YEL "Packet ID ============== %x\n",requestPacket.packetId);
	printf(YEL "Client ID ============== %hhx\n",requestPacket.clientId);
	printf("Access Permission ====== %x\n",requestPacket.Acc_Per);
	printf("Segment no ============= %d \n",requestPacket.segment_No);
	printf("Length of the packet === %d\n",requestPacket.LENGTH);
	printf("Technology ============= %d \n", requestPacket.technology);
	printf("Subscriber no ========== %lu \n",requestPacket.SourceSubscriberNo);
	printf("End of DataPacket ID === %x \n",requestPacket.endPacket);
}

int main(int argc, char**argv)
{
    struct RequestPacket requestPacket;
	struct ResponsePacket responsePacket;
	char line[30];
	int i = 1;
	FILE *filePointer;  // <stdio.h>
	int sockfd,n = 0;
	struct sockaddr_in clientAddress; // <netinet/in.h>
	socklen_t addr_size;
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	struct timeval timeValue;
	timeValue.tv_sec = 3;              // Timeout
	timeValue.tv_usec = 0;

    // Checking for the connection
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeValue,sizeof(struct timeval));
	int count = 0;
	if(sockfd < 0) {
		printf(RED "Connection has Failed\n" RESET);
	}
	bzero(&clientAddress,sizeof(clientAddress)); //<string.h>
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	clientAddress.sin_port=htons(PORT);
	addr_size = sizeof clientAddress ;

    // To Load data into Packet
	requestPacket = Initialize();

    // Reading input file from root location
	filePointer = fopen("Verification_Database.txt", "rt");

	if(filePointer == NULL)
	{
		printf("File could not be opened\n");
	}

    while(fgets(line, sizeof(line), filePointer) != NULL) 
    {
		count = 0;
		n = 0;
		printf(RED " \n ************ Start of a New Packet ************* \n" RESET);
		char * words;
		// To Split the line
		words = strtok(line," ");  //string token
		requestPacket.LENGTH = strlen(words);
		
		requestPacket.SourceSubscriberNo = (unsigned long) strtol(words, (char **)NULL, 10);
		
		words = strtok(NULL," ");
		requestPacket.LENGTH += strlen(words);
		requestPacket.technology = atoi(words);
		words = strtok(NULL," ");
		requestPacket.segment_No = i;
		// To Print Contents of the Packet
		DisplayPacket(requestPacket);
		while(n <= 0 && count < 3) { // Check if packet sent, if not tries again.
			sendto(sockfd,&requestPacket,sizeof(struct RequestPacket),0,(struct sockaddr *)&clientAddress,addr_size);
			// To Receive response from the Server
			n = recvfrom(sockfd,&responsePacket,sizeof(struct ResponsePacket),0,NULL,NULL);
			if(n <= 0 ) {
				// If there is no response
				printf(WHITE "Out of Given Time\n" RESET);
				count ++;
			}
			else if(n > 0) {
				// When the Response is recieved
				printf(GRN "Status = " RESET);
				if(responsePacket.type == NOTPAID) {
					printf(GRN "Subscriber not paid for the subscription.\n" RESET);
				}
				else if(responsePacket.type == TECHNOLOGYMISMATCH) {
					printf(GRN "Subscriber found but technology doesn't match.\n" RESET);

				}
				else if(responsePacket.type == NOTEXIST ) {
					printf(GRN "Subscriber does not exist.\n" RESET);
				}
				else if(responsePacket.type == PAID) {
					printf(GRN "Subscriber permitted to access the network.\n" RESET);

				}
				
			}
		}
		// Make upto 5 attempts for server to respond
		if(count >= 5 ) {
			printf(RED "Server is not responding." RESET);
			exit(0);
		}
		i++;
		
	}

    return 0;
}