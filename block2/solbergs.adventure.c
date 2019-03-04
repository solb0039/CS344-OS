// solbergs.buildroom.c
// Name: Sean Solberg
// ID: solbergs
// Date: 2/14/2018
// Description: Program generates a folder names 'solbergs.<processID>" with 7 file.  Each file
// 	is a room with links to other rooms

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

struct room
{
	char name[20];
	char ROOM_TYPE[20];
	int room_connections[6];
	int numConnects;
};

char *room_names[10] = { "luna", "mars", "jupiter", "saturn" , "mercury" , "venus", "pluto","neptune","earth", "uranus"};

//Function prototypes
void  readStartRoomFile(struct room* roomPtr, char roomToFind[50], char* homeDir );
char* getHomeDir();
void playAdventure(struct room *currRoom, char* homeDir);
void getRoomInfo(struct room *rm, char* homeDirectory);
void createThread();
void* timeFunction();
void printTime();

//Function to find newest folder that matches pattern.  Code adopted from CS344 lectures.
char* getHomeDir()
{
  int newestDirTime = -1; // Modified timestamp of newest subdir examined
  char targetDirPrefix[32] = "solbergs.rooms."; // Prefix we're looking for
  char newestDirName[256]; // Holds the name of the newest dir that contains prefix  
  memset(newestDirName, '\0', sizeof(newestDirName));
  char* newDir = malloc(sizeof(char)*64);

  DIR* dirToCheck; // Holds the directory we're starting in
  struct dirent *fileInDir; // Holds the current subdir of the starting dir
  struct stat dirAttributes; // Holds information we've gained about subdir

  dirToCheck = opendir("."); // Open up the directory this program was run in

  if (dirToCheck > 0) // Make sure the current directory could be opened
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
    {
      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
      {
        stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

        if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
        {
          newestDirTime = (int)dirAttributes.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          strcpy(newestDirName, fileInDir->d_name);
          
	  strcpy(newDir, newestDirName);
        }
      }
    }
  }

  closedir(dirToCheck); // Close the directory we opened
  
  return newDir;
}


/*Function searches through each file in the folder to identify key START_ROOM */

void  readStartRoomFile(struct room *roomPtr, char roomToFind[50], char* homeDir )
{
	//If parameter passed is 'start', then search for type = START_ROOM
	char filePath[100];	
	if(strcmp("START_ROOM" , roomToFind)==0)
	{
		//Open current directory
		DIR* theDir;
		struct dirent *fileInCurDir;
		FILE *open_file;
		theDir = opendir(homeDir);
		char aLine[50];
		char field1[24];
		char field2[24];
		char field3[24];
		if(theDir >0) 
		{		
			while ((fileInCurDir = readdir(theDir)) != NULL)
			{		
				if(!strcmp(fileInCurDir->d_name, "."))
					continue;
				if(!strcmp(fileInCurDir->d_name, ".."))
					continue;			
				sprintf(filePath, "%s/%s", homeDir, fileInCurDir->d_name); 
				
				open_file = fopen(filePath, "r");
				if (open_file == NULL) 
				{perror("Error opening file\n");}
				
				while(fgets(aLine, 50, open_file) != NULL)
				{
					sscanf(aLine, "%s%s%s", field1, field2, field3);
					if(strcmp(field3, "START_ROOM")==0)
					{
						//Start room found so put name of room into struct 
						strcpy(roomPtr->name, fileInCurDir->d_name);
					}			
				}
				fclose(open_file);
			}
		}
		closedir(theDir);
	}
	//Else, find room that matches parameter
	else
	{
		printf("Error, no start room found \n");
	}

}


/*Helper function to search through room_names array and return index of room. Added because 
 * easier to work with ints than strings in C */

int getRoomIndexFromName(char roomName[20]){			//Getter function to find room name from array
	int i;
	for(i = 0; i<10; i++)
	{
		if (strcmp(room_names[i], roomName) == 0){
			return i;
		}
	}
	return 0;
}


/*Function opens room file and parses information to set the struct room properties */

void getRoomInfo(struct room *rm, char* homeDirectory)
{
	FILE *theFile;
	char filePath[100];
	char line[60];
	char field1[20], field2[20], field3[20];
	int roomCounter;

	sprintf(filePath, "./%s/%s", homeDirectory, rm->name);
	//printf("fo;e [path in room info is %s\n", filePath);
	theFile = fopen(filePath, "r");	//Open file
	if (theFile ==NULL)
	{	
		perror("Error opening file\n");
	}
	fgets(line, 50, theFile); //Read first line : ROOM NAME: roomname
	sscanf(line, "%s%s%s", field1, field2, field3);

	memset(field1, '\0', sizeof(field1));
	memset(field2, '\0', sizeof(field2));
	memset(field3, '\0', sizeof(field3));
	roomCounter = 0;	
	fgets(line, 50, theFile);
 	sscanf(line, "%s%s%s", field1, field2, field3);
	while(strcmp(field1, "CONNECTION") ==0)		//Read room connections and set to fieldX vars
	{
		rm->room_connections[roomCounter] = getRoomIndexFromName(field3); 
	 	roomCounter++;
		memset(field1, '\0', sizeof(field1));
		memset(field2, '\0', sizeof(field2));
		memset(field3, '\0', sizeof(field3));
		fgets(line, 50, theFile);
		sscanf(line, "%s %s %s", field1, field2, field3);		
	}
	rm->numConnects = roomCounter;			//Set total number of rooms

	strcpy(rm->ROOM_TYPE, field3);		//Last line of data is the room type so set that var too
	fclose(theFile);
}

/* Main game play loop. While loop until END_ROOM is found */

void playAdventure(struct room *currRoom, char* homeDir)
{
	int numSteps = 1;			//Var to keep track of number of steps to solve game
	char* roomMove;
	size_t nbytes = 50;
	size_t response;
	int validResponse = 0;
	char pathToFinish[50][10];			//Huge array to keep track of all rooms along path
	int i;

	//Given a name of a room, populate the structure with all the attributes
	getRoomInfo(currRoom, homeDir);		//Initial loop so room var is already set, just populate struct
	roomMove =(char *)malloc (nbytes+1);
	strcpy(pathToFinish[0], currRoom->name);

	do{	
		validResponse = 0;

		while (validResponse == 0)
		{
			//Menu options
			printf("\n");					//newline to space out
			printf("CURRENT LOCATION: %s\n", currRoom->name);
			printf("POSSIBLE CONNECTIONS: ");
		
			for(i = 0; i< currRoom->numConnects-1; i++){
				printf("%s, ", room_names[currRoom->room_connections[i]]);
			}
			printf("%s.\n", room_names[currRoom->room_connections[i]]);
			printf("WHERE TO? >");

			response = getline(&roomMove, &nbytes, stdin);	//Read user input

			char* pos;
			if ((pos=strchr(roomMove, '\n')) != NULL)
			{	
				*pos = '\0';
			}						//Find first instance of newline and remove
			
			if(strcmp(roomMove, "time") == 0)
			{
				createThread();	
				printTime();
			}
			else
			{
			//Scan room array to see if it is a valid room
			for (i=0; i < 10; i++)
			{
				if ( strcmp(roomMove, room_names[i]) == 0)
				{
					validResponse = 1;
				}
			}
			if ( validResponse == 0)	//Invalid response.  Print error
			{
				printf("HUH? I DONT'T UNDERSTAND THAT ROOM.  TRY AGAIN\n");
			}
			}
		}

		if(response == -1){
			puts("ERROR");
		}

		for (i=0; i<currRoom->numConnects; i++)	//If find match, Move pointer and read room data
		{
			if(strcmp(roomMove, room_names[currRoom->room_connections[i]])==0)
			{ 
				strcpy(currRoom->name,  room_names[currRoom->room_connections[i]]);
				getRoomInfo(currRoom, homeDir);
			}
		}
		
		strcpy(pathToFinish[numSteps], currRoom->name);
		numSteps++;					//Increment number of steps

	}while(strcmp(currRoom->ROOM_TYPE, "END_ROOM") != 0);
	
	//Found end room, so print game results
	printf("YOU HAVE FOUND THE END ROOM.  CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS\n", numSteps);
	for (i = 0; i < numSteps; i++)
	{
		printf("%s\n", pathToFinish[i]);
	}
	
	free(roomMove);
}

/*Function to get current time. Function is the entry point from mutex in createThread function  */

void* timeFunction(){
  char timeLine[80];
  FILE* myFile;                             
  myFile = fopen("currentTime.txt", "w+");  
  memset(timeLine, '\0', sizeof(timeLine));
         
  struct tm *theTm;
  time_t theTime;
  time(&theTime);
  theTm = localtime(&theTime);
  
  strftime (timeLine, sizeof(timeLine), "%I:%M%p %A, %B %d, %Y", theTm);	//Correct time formatting
  
  //Write to file
  fputs(timeLine, myFile);
  fclose(myFile);
}

/*Function to print the time from timeFunction */

void printTime()
{ 
  //Now open file again for reading and printing
  FILE *myFile;
  char timeLine[80];
  memset(timeLine, '\0', sizeof(timeLine)); 
  myFile = fopen("currentTime.txt", "r");   
 
  if(myFile == NULL){              
    perror("Error opening file\n");
  }
  else{
    fgets(timeLine, 80, myFile);     
    printf("\n%s\n", timeLine);
    fclose(myFile);
  }
}


/*Function to create a separate thread using mutex.  Calls timeFunction as entry point */

void createThread()
{
     //Code example from lecture notes
     pthread_t aThread;                          
     pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
     pthread_mutex_init(&myMutex, NULL);
     pthread_mutex_lock(&myMutex);
     
     int tid = pthread_create(&aThread, NULL, timeFunction, NULL);
     
     pthread_mutex_unlock(&myMutex);
     pthread_mutex_destroy(&myMutex);
     
     usleep(50);
}

int main(void)
{
	char* homeDir = getHomeDir();		//Get the newest directory with rooms
	
	struct room *curRoom = malloc(sizeof(struct room));	//Allocate memory 
	
	readStartRoomFile(curRoom, "START_ROOM", homeDir);	//Initiate by finding starting room file
		
	playAdventure(curRoom, homeDir);		//Game play loop

	free(curRoom);			//Free dynamnic memory
	free(homeDir);
	return 0;
}
