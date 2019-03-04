// Program: solbergs.buildrooms
// Name: Sean Solberg
// ID: solbergs
// Date: 2/14/2018
// Description: Program generates 7 rooms in a folder nameed solbergs.<processID>.  Each roomm file contains
// a room name and connectionz to other room files 


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>


typedef room; 

struct room
{
	char* name; 
	char* ROOM_TYPE; 
	int room_connections[6]; 
	int numConnects; 
};

char *room_names[10] = { "luna", "mars", "jupiter", "saturn" , "mercury" , "venus", "pluto","neptune","earth", "uranus"};
char *room_types[3] = { "START_ROOM", "MID_ROOM", "END_ROOM" };

/*Function to null out room connections.  Added to ensure all values were known after creation */

void initRoomConnections(struct room aRoom[], int number)
{
	int i;
	for (i = 0; i < 6; i++) 
	{
		aRoom[number].room_connections[i] = -1;
	}
	aRoom[number].numConnects = 0;
}

/*Function that uses randomness to create room names and properties  */

void createRooms(struct room inRooms[]) 
{
	int startRm = 0;
	int endRm = 0;
	//loop over all rooms to initialize 
	//create rooms by randomly selecting names and types.  
	//if type selected is start or end, then check local var and set if needed
	srand(time(0)); 
	int i;
	for (i = 0; i < 7; i++) 
	{
		int setFlag = 0;
		//While room is not assigned correctly
		while (setFlag == 0)
		{
			setFlag = 1;
			int roomNum = (rand() % 10);	//Generate random number for random room
			//printf("num is %d\n", roomNum);
			int j;
			for (j = 0; j < 10; j++)	//scan through rooms to see if name already exists
			{
				if (room_names[roomNum] == inRooms[j].name)
					setFlag = 0;
			}
			inRooms[i].name = room_names[roomNum];	//Assign room name
		}

		//Assign room type
		int roomSetFlag = 0;
		while (roomSetFlag == 0)
		{
			int roomTy = (rand() % 3);	//Generate random number for random room type
			if (roomTy == 0 && startRm == 0)
			{
				inRooms[i].ROOM_TYPE = room_types[roomTy];
				startRm = 1;
				roomSetFlag = 1;
			}
			else if (roomTy == 2 && endRm == 0)
			{
				inRooms[i].ROOM_TYPE = room_types[roomTy];
				endRm = 1;
				roomSetFlag = 1;
			}
			else if (roomTy ==1)
			{
				inRooms[i].ROOM_TYPE = room_types[roomTy];
				roomSetFlag = 1;
			}
		}
		initRoomConnections(inRooms, i);	//Zero out room connection array
	}
	//check condition to ensure that start room and end room were assigned.
	int found_start=0;
	int found_end=0;
	int counter;
	for (counter=0; counter< 7; counter++)
	{
		if (strcmp(inRooms[counter].ROOM_TYPE, "START_ROOM") == 0)
			found_start = 1; 
		if (strcmp(inRooms[counter].ROOM_TYPE, "END_ROOM") == 0)
			found_end = 1;
	};
	//If no start or end found, then assign rooms
	if (!found_start) 			//Assign Start Room to first open position
	{	
		counter = 0;
		while(strcmp(inRooms[counter].ROOM_TYPE, "END_ROOM") == 0)
		{
			counter++;
		}
		inRooms[counter].ROOM_TYPE = "START_ROOM";
	}
	if (!found_end)
	{
		counter = 0;			//Assign end room to first open position
		while(strcmp(inRooms[counter].ROOM_TYPE, "START_ROOM") == 0)
		{
			counter++;
		}
		inRooms[counter].ROOM_TYPE = "END_ROOM"; 
	}
}

/*Helper funciton to generate random numbers in a range*/

int generateNumber(int start, int end)		//Helper function to generate random number in a range
{
	int numFlag = 0;
	int number = 0;
	while (numFlag == 0)
	{ 
		numFlag = 1;
		number = (rand() % end) + 1;  //Number 1 to 6
			if (number <= (start-1))
				numFlag = 0;
	}
	return number;
}

/*Function that randomly selects 2 rooms and connects them together if possible*/

void connectRooms(struct room inRooms[]) 
{
	int room1, room2;
	//Connect start room to random room
	//First find the start room number
	int counter = 0;
	while (inRooms[counter].ROOM_TYPE != "START_ROOM") 
	{
		counter++;
	}
	//Find a random room to connect to
	room2 = generateNumber(0, 7)-1;
	while (room2 == counter)
	{
		room2 = generateNumber(0, 7)-1;
	}
	//Make connections and increment counters
	inRooms[counter].room_connections[0] = room2;
	inRooms[counter].numConnects += 1;
	inRooms[room2].room_connections[0] = counter;
	inRooms[room2].numConnects += 1;

	//Connect end room to random room
	//Find the end room number
	counter = 0;
	while (inRooms[counter].ROOM_TYPE != "END_ROOM")
	{
		counter++;
	}
	//Find a random room to connect to
	room1 = generateNumber(0, 7)-1;
	while (room1 == counter)
	{
		room1 = generateNumber(0, 7)-1;
	}
	//Make connection and increment room counters
	inRooms[counter].room_connections[inRooms[counter].numConnects] = room1;
	inRooms[counter].numConnects += 1;
	inRooms[room2].room_connections[inRooms[room2].numConnects ] = counter;
	inRooms[room2].numConnects += 1;

	//Loop to make connections until exit condition met 
	int exitFlag = 0;
	while (exitFlag == 0)
	{
		do {
			room1 = generateNumber(0, 7)-1;
			room2 = generateNumber(0, 7)-1;
		} while (room1 == room2);

		int connectionFlag = 0;
		
		//Check if selected rooms have free connections
		if ((inRooms[room1].numConnects < 6) && (inRooms[room2].numConnects < 6))
		{

			while (connectionFlag == 0)
			{
				//Check if rooms are already connected and if either room has free conncetions
				int currRom;
				for (currRom = 0; currRom < 6; currRom++)
				{
					if ((inRooms[room1].room_connections[currRom] == room2) || (inRooms[room2].room_connections[currRom] == room1))
						connectionFlag = 1;
				}
				//If make if through loop without flag ==1 then connect rooms
				if (!connectionFlag) 
				{
					inRooms[room1].room_connections[inRooms[room1].numConnects] = room2;
					inRooms[room1].numConnects++;
					//printf("Connected %d to room %d\n", room1, room2);
					inRooms[room2].room_connections[inRooms[room2].numConnects] = room1;
					inRooms[room2].numConnects++;
					connectionFlag = 1;
				}
			}
		}

		//Final check conditions: All rooms have at least 3 connections and no more than 6
		if ((inRooms[0].numConnects >= 3) && (inRooms[1].numConnects >= 3) && (inRooms[2].numConnects >= 3) && (inRooms[3].numConnects >= 3) && (inRooms[4].numConnects >= 3) && (inRooms[5].numConnects >= 3) && (inRooms[6].numConnects >= 3))
			exitFlag = 1;
	}
}

/*Function to write all room data to files in the created folder*/

void writeFile(struct room roomSet[]) 
{
	//Create folder to store files
	int id = getpid();
	char dirname[20]; //need to add PID
	sprintf(dirname, "solbergs.rooms.%d", id);
	
	mkdir(dirname, 0755);  //creates directory
	chdir(dirname);
	
	FILE *fp;
	
	//Add all folders to the directory
	int i;
	for (i = 0; i < 7; i++)
	{	
		char* filename;
		filename = roomSet[i].name;
		fp = fopen(filename, "w");
		
		fprintf(fp, "ROOM NAME: %s\n", roomSet[i].name);
		int j;
		for (j = 0; j < roomSet[i].numConnects; j++)
		{
			 //Get integer of room connection and then get name of that room	
			fprintf(fp, "CONNECTION %d: %s\n", (j+1), roomSet[  roomSet[i].room_connections[j] ].name );
		}
		fprintf(fp, "ROOM TYPE: %s\n", roomSet[i].ROOM_TYPE);
		fclose(fp);
	}
}

int main()
{
	//Declare and initialize struct
	struct room theRooms[7];
	
	//Initialize rooms
	createRooms(theRooms);
	connectRooms(theRooms);
	
	//Write files
	writeFile(theRooms);

    return 0;
}

