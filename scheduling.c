/*=====================================================================================
	Assignment 3 - Question 1: Implement Disk Scheduling Algorithms [60 Marks]

	Name: Assignment3.c

	Written by: Brett Challice => March 12, 2024

	Purpose: To test and implement various disk scheduling algorithms given a series
		 of requests provided by the binary file "data/request.bin". The following
		 program outputs the order of requests in which they are serviced by each
		 of the algorithms; as well as the total amount of head movement incurred
		 servicing all the requests. The program requires two arguments, an initial
		 position (0-299 cylinders) and an initial direction. The position is needed
		 by all algorithms, but the direction is only required by SCAN and C-SCAN.

	Usage: gcc -o Assignment3 Assignment3.c
	       ./Assignment3

	Description of Parameters:

	-> 1) int    => Initial position of the disk head 
	   2) string => Direction of the head (LEFT or RIGHT)

	Subroutines/Libraries required:

	-> See include statements

---------------------------------------------------------------------------------------*/

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

// Prototypes
void fcfs_implementation(int* requests);
void scan_implementation(int* requests);
void c_scan_implementation(int* requests);
int cmpfunc (const void * a, const void * b);

// Globals
int initial_position;
char* initial_direction;
int num_requests;

//------------------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv){

	// Check for correct number of parameters
	if(argc != 3){printf("\nError: This program requires two arguments. Disk position (int) & Head Direction (LEFT or RIGHT)\n\n"); exit(EXIT_FAILURE);}

	// Retrieve command line args
	initial_position = atoi(argv[1]);
	initial_direction = argv[2];

	// Check for invalid arguments
	// Check if position is between 0 and 299
	if(initial_position < 0 || initial_position > 299){
		printf("Error: Invalid initial position (1st parameter) \n");
		exit(EXIT_FAILURE);
	}
	
	// Check if direction is either LEFT or RIGHT in either case
	if(strcasecmp(initial_direction, "LEFT") != 0 && strcasecmp(initial_direction, "RIGHT") != 0){
		printf("\nInvalid initial direction (2nd parameter)\n");
		exit(EXIT_FAILURE);
	}

	// Convert direction to uppercase to aid in comparision later
	char* initial_direction_start = initial_direction;
	while(*initial_direction){
		*initial_direction = toupper((unsigned char)*initial_direction);
		initial_direction++;
	}
	// Reset pointer to start of the string for printing later
	initial_direction = initial_direction_start;

	// Open the request.bin

	int request_fd;
	request_fd = open("data/request.bin", O_RDONLY);
	if (request_fd == -1){
		printf("Error opening request.bin!\n");
		exit(EXIT_FAILURE);
	}

	// Populate the requests array with the requests in the binary file
	// Get file size of the binary
	int file_size = lseek(request_fd, 0, SEEK_END);

	// Reset the file position to the start of the binary
	lseek(request_fd, 0, SEEK_SET);
	
	// Create requests array to write into, with the size of the binary file size
	int requests[file_size/sizeof(int)];
	
	int bytes_read = read(request_fd, requests, file_size);
	if(bytes_read == -1){
		printf("Error in the reading! \n");
		exit(EXIT_FAILURE);
	}

	// Close descriptor
	close(request_fd);
	
	// Print header
	printf("\n--------------------------------------------------");
	printf("\nTotal Requests => %lu\n", file_size/sizeof(int));
	printf("Initial Head Position => %d\n", initial_position);
	printf("Initial Head Direction => %s\n", initial_direction);
	printf("--------------------------------------------------\n");

	// Set global variable to use in implementations
	num_requests = sizeof(requests)/sizeof(int);

	// Call each implementation
	fcfs_implementation(requests);
	scan_implementation(requests);
	c_scan_implementation(requests);

	return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------

void fcfs_implementation(int* requests){

	printf("FCFS DISK SCHEDULING ALGORITHM: \n\n");

	// No need for sortedArray => Requests are processed in the order they are received

	// Print FSFS Numbers
	for(int i = 0; i < num_requests; i++){
		if (i == num_requests - 1)
			printf("%d\n", requests[i]);
		
		else
			printf("%d, ", requests[i]);
	}

	// Compute head movements
	int total_head_movements = 0;

	// Create a new variable tracking the head position, to preserve the global var
	int this_position = initial_position;

	// Loop through array, summing the absolute value of the difference between target location and the current position
	for(int i = 0; i < num_requests; i++){
		total_head_movements += abs(this_position - requests[i]);
		this_position = requests[i];
	}

	printf("\nFCFS - Total Head Movements = %d\n", total_head_movements);
	printf("--------------------------------------------------\n");
	
}

//------------------------------------------------------------------------------------------------------------------------------------------------------
void scan_implementation(int* requests){

	printf("SCAN DISK SCHEDULING ALGORITHM: \n\n");

	int sortedArray[num_requests];

	// SCAN ALGORITHM:
	// 1) Scan moves moves the disk arm in one direction, servicing requests along the way until reaching the end of the disk
	// 2) Once the end of the disk is reached, the arm switches direction and services requests on its way back to the start

	// Create new variables tracking the head position and direction, to preserve the global vars
	int this_position = initial_position;
	char* this_direction = initial_direction;

	// Sort the requests from least to greatest
	qsort(requests, num_requests, sizeof(int), cmpfunc);	
	
	// Get starting index
	int start_index;

	// Initial position less than minimum page request
	// Start index is the first element, with the direction flipped
	if((initial_position < requests[0]) && (strcmp(initial_direction, "LEFT") == 0)){
		start_index = 0;
	}
	
	// Initial position is more than maximum page request
	// Start index is last element, with direction flipped
	else if((initial_position > requests[num_requests-1]) && (strcmp(initial_direction, "RIGHT") == 0)){
		start_index = num_requests - 1;
	}

	// Iterate through array to find the starting index
	else{
		for(int i = 0; i < num_requests; i++){

			// If position is in between the two page requests
			if((requests[i] <= initial_position) && (requests[i+1] >= initial_position)){
				if(initial_position == requests[i+1]){
					start_index = i+1;
					break;
				}
				else{
					start_index = i;
					break;
				}		
			}
			// Initial position is greater than element and next element, continue to next comparison					
			else
				continue;
		}
	}


	// Starting at the starting index, iterate in the given direction storing the value in the sorted array
	// Once the bottom or top of the array has been reached,
	// Flip direction, and start from the right or left of the starting index

	int current_index = start_index;
	for(int i = 0; i < num_requests; i++){

		// LEFT DIRECTION
		if(strcmp(this_direction, "LEFT") == 0){

			// Edge => Min is reached
			if(current_index == 0){
			   	this_direction = "RIGHT";
				sortedArray[i] = requests[current_index];
				current_index = start_index + 1;
			}
			else{
				sortedArray[i] = requests[current_index];
				current_index--;
			}
		}
		
		// RIGHT DIRECTION
		else{
			// Edge => Max is reached
			if(current_index == num_requests - 1){
				this_direction = "LEFT";
				sortedArray[i] = requests[current_index];
				current_index = start_index - 1;
			}
			else{
				sortedArray[i] = requests[current_index];
				current_index++; 
			}
		}

	} 

	// Print sorted requests
	for(int i = 0; i < num_requests; i++){
		if (i == num_requests - 1)
			printf("%d\n", sortedArray[i]);
		
		else
			printf("%d, ", sortedArray[i]);
	}


	// Compute head movements
	int total_head_movements = 0;

	// Loop through array, summing the absolute value of the difference between target location and the current position
	for(int i = 0; i < num_requests; i++){
		total_head_movements += abs(this_position - sortedArray[i]);
		this_position = sortedArray[i];
	}


	printf("\nSCAN - Total Head Movements = %d\n", total_head_movements);
	printf("--------------------------------------------------\n");

}

//------------------------------------------------------------------------------------------------------------------------------------------------------
void c_scan_implementation(int* requests){

	printf("C-SCAN DISK SCHEDULING ALGORITHM: \n\n");

	int sortedArray[num_requests];

	// Create new variables tracking the head position and direction, to preserve the global vars
	int this_position = initial_position;
	char* this_direction = initial_direction;

	// C-SCAN ALGORITHM (Circular SCAN)
	// 1) Improvement over the SCAN Algorithm
	// 2) Arm moves in one direction only (LEFT or RIGHT)
	// 3) Once an edge is hit, it immediately jumps to the other end continuing in the same direction

	// Sort the requests from least to greatest
	qsort(requests, num_requests, sizeof(int), cmpfunc);

	// Get starting index
	int start_index;

	// Initial position less than minimum page request
	// Start index is the first element, with the direction flipped
	if((initial_position < requests[0]) && (strcmp(initial_direction, "LEFT") == 0)){
		start_index = 0;
	}
	
	// Initial position is more than maximum page request
	// Start index is last element, with direction flipped
	else if((initial_position > requests[num_requests-1]) && (strcmp(initial_direction, "RIGHT") == 0)){
		start_index = num_requests - 1;
	}

	// Iterate through array to find the starting index
	else{
		for(int i = 0; i < num_requests; i++){

			// If position is in between the two page requests
			if((requests[i] <= initial_position) && (requests[i+1] >= initial_position)){
				if(initial_position == requests[i+1]){
					start_index = i+1;
					break;
				}
				else{
					start_index = i;
					break;
				}		
			}
			// Initial position is greater than element and next element, continue to next comparison					
			else
				continue;
		}
	}

	// Starting at the starting index, iterate in the given direction storing the value in the sortedArray
	// When an edge is hit, loop around to opposite end
	int current_index = start_index;
	for(int i = 0; i < num_requests; i++){

		// LEFT DIRECTION
		if(strcmp(this_direction, "LEFT") == 0){

			// Edge => Min is reached
			if(current_index == 0){
				sortedArray[i] = requests[current_index];
				current_index = num_requests - 1;
			 }
			else{
				sortedArray[i] = requests[current_index];
				current_index--;
			}
		}
		
		// RIGHT DIRECTION
		else{
			// Edge => Max is reached
			if(current_index == num_requests - 1){
				sortedArray[i] = requests[current_index];
				current_index = 0;
			}
			else{
				sortedArray[i] = requests[current_index];
				current_index++; 
			}
		}

	} 

	// Print sorted requests
	for(int i = 0; i < num_requests; i++){
		if (i == num_requests - 1)
			printf("%d\n", sortedArray[i]);
		
		else
			printf("%d, ", sortedArray[i]);
	}

	// Compute head movements
	int total_head_movements = 0;

	// Loop through array, summing the absolute value of the difference between target location and the current position
	for(int i = 0; i < num_requests; i++){
		total_head_movements += abs(this_position - sortedArray[i]);
		this_position = sortedArray[i];
	}

	printf("\nC-SCAN - Total Head Movements = %d\n", total_head_movements);
	printf("--------------------------------------------------\n");

}
//------------------------------------------------------------------------------------------------------------------------------------------------------

// Used in qsort => Taken from https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm
int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}
//------------------------------------------------------------------------------------------------------------------------------------------------------

