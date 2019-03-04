/* Name: Sean Solberg
 * Date: 3/5/2018
 * Description: CS344 Assignment 3, smallsh shell
 * */

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int backGroundIgnore = 0;			//Global var to control foreground only mode

/*Function handler for  SIGTSTP */

void catchSIGTSTP(int signo)
{
	
	if(backGroundIgnore)			//Set signal and variable for background ignore state 
	{
		char* message = "Exiting foreground-only mode\n";
		write(STDOUT_FILENO, message, 29);
		backGroundIgnore = 0;
	}
	else 					//Toggle back to foreground only mode
	{
		char* message = "Entering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, 49);
		backGroundIgnore = 1;
	}
	fflush(stdout);

}


int main(){

	struct sigaction SIGINT_action = {0};
	struct sigaction SIGTSTP_action = {0};

	SIGINT_action.sa_handler = SIG_IGN;				//Ignore signal for main program (shell)
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;
	sigaction(SIGINT, &SIGINT_action, NULL);

	SIGTSTP_action.sa_handler = catchSIGTSTP;			//Sigaction for SIGTSTP signal set to a function
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	char *InpLine[512];
	char Line[2048];	
	char *stringtok;
	char *tempCmd;
	char *outFile = NULL;
	char *inFile = NULL;
	int numCmds = 0;
	int exitFlag = 0;
	int fileCheck = 0;
	int isBackGroundProcess = 0;	
	pid_t spawnpid = -5;
	int childExitMethod = -5;
	char proId[10];

	/*Start of main loop during run-time*/
	while(exitFlag == 0){
		isBackGroundProcess = 0;
		printf(": ");							//Print command prompt
		fflush(stdout);
		fgets(Line, 2048  , stdin);					//Get input
	
		stringtok = strtok(Line, " \n");				//Parse input by using strtok calls to step through
 		while (stringtok != NULL){	
			if ((strcmp(stringtok, "#") == 0) || ('#' == stringtok[0]) ) //Ignore input case
			{
				fflush(stdout);
				break;
			}
			else if (strcmp(stringtok, ">") == 0)			//Output redireciton case
			{
				/*Output redirection*/
				stringtok = strtok(NULL, " \n");
				outFile = stringtok;
			}		
			else if (strcmp(stringtok, "<") == 0)			//Input redirection case
			{		
				/*Input redirection*/
				stringtok = strtok(NULL, " \n");
				inFile = stringtok;
			}
			else if (strcmp(stringtok, "&") == 0)			//Background mode case
			{ 	
				/*Found background command, check if & is last element*/
				stringtok = strtok(NULL, " \n");
				if (stringtok == NULL)				//& is last element
				{
					if(!backGroundIgnore)
					{
						isBackGroundProcess = 1;	//Set back ground flag
					}
				}
				else 
				{
					continue; 				// ignore &
				}
			}	
			else
			{
				int len = strlen(stringtok);			//Temp vars to handle string processing
				char str2[3];
				char str3[100];
				memset(str2, '\0', sizeof(str2));
				memset(str3, '\0', sizeof(str3));
				strncpy(str2, stringtok+len-2,2);		//Get last two chars in string token
				if ((strcmp(str2, "$$") == 0) && (len > 2))  	//Case where last 2 letters are $$
				{
					sprintf(proId, "%d", getpid());	
					strncpy(str3, stringtok, len-2);	//Cut off $$	
					strcat(str3, proId);			//Add process id to string 
					InpLine[numCmds] = str3;
					numCmds++;
					stringtok = strtok(NULL, " \n");
				}
				else if((strcmp(stringtok, "$$") == 0) ) 	//Handle $$ character
				{
					sprintf(proId, "%d", getpid());
					InpLine[numCmds] = proId;
					numCmds++;
					stringtok = strtok(NULL, " \n"); 
				}
				else
				{
					InpLine[numCmds] =  stringtok;		//copy command into array
					numCmds++;
					stringtok = strtok(NULL, " \n");
				}
			}
		}
		InpLine[numCmds] = NULL;					//Add null terminator for execvp command
		if(numCmds == 0) 
		{
		}
		/*Carry out commands received*/
		else if (strcmp(InpLine[0], "cd") == 0 )			//cd option redirect
		{
			if (InpLine[1] != NULL){
				chdir(InpLine[1]);
			}
			else 
			{ 
				chdir(getenv("HOME"));				//Get PATH variable
			}
		}
		else if (strcmp(InpLine[0], "status") == 0)			//Status command option
		{
			if(WIFEXITED(childExitMethod)){          		//Taken from lecture
        			printf("exit value %i\n", WEXITSTATUS(childExitMethod));
				fflush(stdout);
    			}	 
    			else{
       				 printf("terminated by signal %i\n", childExitMethod);
				fflush(stdout);
    			}	
		}
		else if (strcmp(InpLine[0], "exit") == 0)			//Exit shell case
		{
			exitFlag = 1;
			exit(0);
		}
		else								//Different command sent-->Fork
		{
			spawnpid = fork();
			switch(spawnpid)					//Go over possible cases
			{
				case -1:
					perror("Error in fork\n");
					exit(1);
					break;
				case 0:						//Run command in child process
					if(!isBackGroundProcess)		//Start signal handler for interrupt
					{	
						SIGINT_action.sa_handler = SIG_DFL;	//Kill process in foregd
						sigaction(SIGINT, &SIGINT_action, NULL);			
					}	
					/*Set up input and output files*/
					if (inFile != NULL)
					{
						fileCheck = open(inFile, O_RDONLY);
						if (fileCheck == -1)
						{
							printf("cannot open %s for input\n", inFile);
							fflush(stdout);
							_exit(1);
						}
						else {dup2(fileCheck, 0);}  		//Set input file to stdin 
						close(fileCheck);
					}
					else if (isBackGroundProcess)		//For background processes with no sigint
					{
						fileCheck = open("/dev/null", O_RDONLY);
						if (fileCheck == -1)
						{
							perror("Error opening file");
							_exit(1);	
						}
						else {dup2(fileCheck, 0);}
				
					}
					if (outFile != NULL)				//Set output if file specified
					{
						fileCheck = open(outFile, O_WRONLY | O_TRUNC | O_CREAT, 0744);
						if(fileCheck == -1)
						{
							printf("cannot open %s for output\n", outFile);
							fflush(stdout);
							_exit(1);
						}
						else {dup2(fileCheck, 1);}
						close(fileCheck);
					}
					if( execvp(InpLine[0], InpLine) < 0)		//Execute unix command
					{
						printf("Command: %s is not found\n", InpLine[0]);
						fflush(stdout);
						_exit(1); 				//abort child process	
					} 
					break;
				default:	
					if((isBackGroundProcess)) 			//Print process id of background
					{
						printf("background pid is %i\n", spawnpid);	
						fflush(stdout);
						break;
					}
					else 	//Parent process waits for child is in foreground
					{
						waitpid(spawnpid, &childExitMethod, 0); 
					} 
			}
		}
		/*Clean up and reset variables for next loop iteration*/
		numCmds = 0;
		inFile = NULL;
		outFile = NULL;
		int i;
		for (i = 0; i<numCmds; i++){
			free(InpLine[i]);
		}

		/*Check if any process has completed, return immediately with 0 if none have*/
		/*From lecture notes. Checking the exit status*/
		spawnpid = waitpid(-1, &childExitMethod, WNOHANG);
		/*if(! WIFEXITED( childExitMethod ) )		//Catch SIGINT to get exit status
		{
			if( WIFSIGNALED( childExitMethod ) )
            		{
				printf( "terminated by signal %d\n", WTERMSIG( childExitMethod ));
				fflush(stdout);
			}
		}*/
		while(spawnpid > 0)
		{
			if (WIFEXITED(childExitMethod))
			{	
				// Exited normally
				int exitStatus = WEXITSTATUS(childExitMethod);
				printf("background pid %i is done: exit value %d\n", spawnpid, exitStatus);
				fflush(stdout);
			} 
			else
			{	//Terminated by signal, background
				if(isBackGroundProcess){
					printf("background pid %i is done: terminated by a signal %i\n", spawnpid, childExitMethod);
					fflush(stdout);
				}
				else	//Termindated by signal, foreground
				{
					printf("terminated by signal %i\n", childExitMethod);
					fflush(stdout);
 				}
			}
			spawnpid = waitpid(-1, &childExitMethod, WNOHANG);
		}	
	}
	return 0;
}

