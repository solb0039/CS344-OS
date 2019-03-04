#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[])
{
	int keylength;
	int counter;
	int randKey;
	char randVal[1];
	srand(time(NULL));	

	if (argc < 2)						//Check for two arguments
	{	
		printf("Error: key ‘myshortkey’ is too short");
		exit(1);
	}
	keylength = atoi(argv[1]);				//keylength is the  argument
	
	counter = 0;
	for(counter = 0; counter<keylength; counter++)		//Loop to generate key
	{
		randKey = rand()%27 + 65;			//Generate random number and convert to ASCII
		if (randKey == 91)				//Handle case of blank space
		{
			randKey = 32;
			fprintf(stdout, "%c", randKey);
		}
		else						//Regular case
		{
			//Output 
			fprintf(stdout, "%c", randKey);
		}
	}
	
	fprintf(stdout, "\n");					//Add newline

	return 0;
}
