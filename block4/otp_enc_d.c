/* Name: Sean Solberg
 * Date: 3/17/2018
 * Title: CS344 Project 4
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#define	BUFFSIZE 140000 

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[BUFFSIZE];
	struct sockaddr_in serverAddress, clientAddress;
	pid_t pid;	

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args
	//arg1 is port
	
	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0)
	{
		fprintf(stderr, "ERROR opening socket\n");
		exit(2);
	}

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
	{
		fprintf(stderr, "ERROR on binding");
		exit(2);
	}
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	
	while(1){
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) {fprintf(stderr, "ERROR on accept");}

		//Fork and run as child
		pid = fork();
		if(pid < 0)
		{
			fprintf(stderr, "Error forking\n");
			exit(1);
		}
		if(pid == 0) 
		{
			// Get the message from the client and display it
			memset(buffer, '\0', BUFFSIZE);
			char* buffLocation = buffer; //set pointer to start of buffer
			char* keyLocation ;
			char* endLocation;
			int indicator = 0;
			int remainBytes = BUFFSIZE;
			int totBytesRead = 0;
			int exitFlag = 1;
			int i;
			//Vars for sending response
			char message[BUFFSIZE];
			char keyMessage[BUFFSIZE];
			//Vars for encoding message
			int messCharAsInt;
			int keyCharAsInt;
			int encodedNum;
			char encodedChar;
			int messLength = 0;
			
			//Authenticate	
			read(establishedConnectionFD, buffer, 100);
			if( strcmp(buffer, "E9vk72n") !=0)
			{
				char response[] = "Authentication failed";
				write(establishedConnectionFD, response, sizeof(response)); // Send failure notice back
				exit(2);
			}
			else
			{
				char response[] = "E9vk72n";
				write(establishedConnectionFD, response, sizeof(response));
			}
	 		memset(buffer, '\0', 100);
	
			//Loop over stream data to get key and message
			while(exitFlag)
			{	
				charsRead = recv(establishedConnectionFD, buffLocation, remainBytes, 0); 
				// Read the client's message from the socket	
				if (charsRead < 0){fprintf(stderr, "ERROR reading from socket"); exit(2);}
				if (charsRead == 0){break;}
			
				//Step one by one through charsRead to find \n which separates key from message
				for (i = totBytesRead; i < totBytesRead + charsRead; i++)
				{	
					if (buffer[i] == '\n')
					{
						indicator++;
						if (indicator == 1)
						{
							keyLocation = buffer + i +1;   //Add 1 to move pointer to start of key
							messLength = (keyLocation - buffer);
						}
						if (indicator == 2)			//Found second newline
						{
							endLocation = buffer + i;	//Mark end location
							exitFlag = 0;
							break;
						}
					}
				}
				totBytesRead = totBytesRead + charsRead;
				buffLocation = buffLocation + charsRead;
				remainBytes = remainBytes - charsRead;
			}
			//Copy data into char arrays
			memset(keyMessage, '\0', sizeof(keyMessage));
			strncpy(keyMessage, keyLocation, (endLocation - keyLocation));	
			memset(message, '\0', sizeof(message));
			strncpy(message, buffer, (keyLocation - buffer));
			
			//Encode message
			//Step through each position and convet to ascii int
			for (i=0; i < messLength - 1; i++)
			{
				messCharAsInt = (int)message[i];
				if (messCharAsInt == 32)		//Handle case if a space
				{
					messCharAsInt = 26;
				}
				else					//Convert ot ASCII
				{
					messCharAsInt = messCharAsInt - 65;
				}
				keyCharAsInt = (int)keyMessage[i];
				if (keyCharAsInt == 32)		//Handle case if a space
				{
					keyCharAsInt = 26;
				}
				else					//Convert ot ASCII
				{
					keyCharAsInt = keyCharAsInt - 65;
				}
				encodedNum = (messCharAsInt + keyCharAsInt) % 27;
		
				if(encodedNum == 26)
				{
					encodedChar = (char)(32);
				}
				else {
					encodedChar = (char)(encodedNum + 65);
				} 
				message[i] = encodedChar;
			}	
			message[i]='\0';
			// Send encoded  message back to the client
			charsRead = send(establishedConnectionFD, message, sizeof(message), 0); // Send success back
			if (charsRead < 0) {
				fprintf(stderr, "ERROR writing to socket\n"); 
				exit(2);
			}
		}
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
	}
	close(listenSocketFD); // Close the listening socket
	return 0; 
}
