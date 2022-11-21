#include <stdint.h>
#include <stdio.h>
#include <netinet/in.h>

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
    return 0;
}