/* Name: Sean Solberg
 * Date: 3/17/2018
 * Title: CS344 Project 4
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFFSIZE 140000
void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

void sendFile(char* filename, int socketFD, int fileLength)
{ 
	FILE* f = fopen(filename, "r");         //Set up a file to read 
	char buffer[BUFFSIZE]; 
	memset(buffer, '\0', sizeof(buffer)) ; 
	int numBytes; 
	int length; 
	int totLength = 0;

	length = fread(buffer, sizeof(char), fileLength, f); 
	totLength = length;
	//Loop to send data 
	while(length > 0 && totLength <= fileLength)
     	{	 
        	numBytes = send(socketFD, buffer, length, 0);
		if(numBytes < 0) 
		{	 
             		break; 
        	} 
        	memset(buffer, '\0', sizeof(buffer)); 
		length = fread(buffer, sizeof(char), fileLength, f);
		totLength = totLength + length;
	} 
       
	fclose(f);              //Clean up 
	return; 
} 


int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[BUFFSIZE];
	char authKey[] = "h8heUI";  
	char decodedMessage[BUFFSIZE];  	

	if (argc != 4) { fprintf(stderr,"USAGE: %s file keyfile port\n", argv[0]); exit(0); } // Check usage & args
	//arg1 is file, arg2 is keyfile, arg3 is port
	
	//Determine key file size
	int key = open(argv[2], O_RDONLY);
	int keyFileSize = lseek(key, 0, SEEK_END);

	//Determine message file size
	int message = open(argv[1], O_RDONLY);
	int messageFileSize = lseek(message, 0, SEEK_END); 

	//Check that key is equal of larger than message
	if(keyFileSize < messageFileSize)
	{	
		fprintf(stderr, "Key is smaller than message\n");
		exit(1);
	}
	if(keyFileSize > BUFFSIZE)
	{
		fprintf(stderr, "Key is too long\n");
		exit(1);
	}
	
	//Loop through message to check for only alphanumeric values.  Accept @ as terminating char.	
	char buff;
        int fd = open(argv[1], O_RDONLY);
        while ( (read(fd , &buff, sizeof(char))) > 0 ){
		if(buff == 64)
		{
			continue;
		}
		else if(!isalpha(buff) && !isspace(buff))
		{
			fprintf(stderr, "Error: File must be only characters and spaces\n");
			exit(1);
		}
        }

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) {
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(2); 
	}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) {
		fprintf(stderr, "CLIENT: ERROR opening socket");
		exit(2);
	}
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
	{
		fprintf(stderr, "CLIENT: ERROR connecting");
		exit(2);
	}

	// Authenticate 
	write(socketFD, authKey, sizeof(authKey));
	memset(buffer, '\0', sizeof(buffer));
	read(socketFD, buffer, sizeof(buffer));
	if (strcmp(buffer, authKey) !=0)
	{
		fprintf(stderr, "Cannot validate connection");
		exit(2);
	}
	
	//Send files
	sendFile(argv[1], socketFD, messageFileSize);		//Message
	sendFile(argv[2], socketFD, keyFileSize);		//Key

	
	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	char buf[256];	
	size_t bufLen = 0;
	
	do{
		memset(buf, '\0', sizeof(buf));
		int r = recv(socketFD, buf, sizeof(buf) , 0);
		if(r < 0) {
			 error("error connecting\n"); 
			exit(2); 
		}
		strcat(buffer, buf);
		bufLen += r;
	}while(bufLen < sizeof(buffer));


	//Parse message to get rid of excess and store in final array
	memset(decodedMessage, '\0', sizeof(decodedMessage));
	int i = 0;
	do{
		decodedMessage[i] = buffer[i];
		i +=1;
	}while(buffer[i] != '\0');
	decodedMessage[i] = '\0';

	//Output and close socket
	fprintf(stdout, "%s\n", decodedMessage);
	
	close(socketFD); // Close the socket
	return 0;
}
