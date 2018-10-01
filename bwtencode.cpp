#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

#define MAX_PATH_LENGTH 256

struct Suffix {
	char *str;
	int postion;
};

void Swap(struct Suffix *a, struct Suffix *b)
{
	struct Suffix temp = *a;
	*a = *b;
	*b = temp;
}

bool compare(struct Suffix *a, struct Suffix *b, int delimter){
	int i = 0;
	while (a->str[i] == b->str[i]) {
		int ascii = a->str[i];
		if(ascii == delimter){
			return (a->postion < b->postion);
		}
		i++;
	}
	return (a->str[i] < b->str[i]);
}

void QuickSort(struct Suffix *suffixes, int len, int delimter)
{
	int pvt=0;

	if (len <= 1)
		return;

	// swap a randomly selected value to the last node
	Swap(&suffixes[(int)rand() % len], &suffixes[len - 1]);

	// reset the pivot index to zero, then scan
	for (int i=0; i<len-1; i++)
	{
		if (compare(&suffixes[i], &suffixes[len-1], delimter)){
			Swap(&suffixes[i], &suffixes[pvt++]);
		}
	}

	// move the pivot value into its place
	Swap(&suffixes[pvt], &suffixes[len-1]);

	// and invoke on the subsequences. does NOT include the pivot-slot
	QuickSort(suffixes, pvt++, delimter);
	QuickSort(suffixes + pvt, len - pvt, delimter);
}


int BinarySearch(int arr[], int l, int r, int x) 
{ 
	if (r >= l) 
	{ 
		int mid = l + (r - l)/2; 

		// If the element is present at the middle 
		// itself 
		if (arr[mid] == x) 
			return mid; 

		// If element is smaller than mid, then 
		// it can only be present in left subarray 
		if (arr[mid] > x) 
			return BinarySearch(arr, l, mid-1, x); 

		// Else the element can only be present 
		// in right subarray 
		return BinarySearch(arr, mid+1, r, x); 
	} 

	// We reach here when element is not 
	// present in array 
	return -1; 
} 


int main(int argc, char *argv[]) {
	// read arguments
	int delimiter;
	if(strcmp("\n", argv[1]) == 0){
		delimiter = 10;
	} else{
		delimiter = argv[1][0];
	}
	const char* temp_folder_name = argv[2];
	const char* original_file_name = argv[3];
	const char* bwt_file_name = argv[4];
	
	
	/*********************************************************************************************
	read original file into a dynamic size buffer
	this buffer costs 50MB memory at most and 50MB is much smaller than 2^32-1
	so we could convert the long type file size to int size safely
	and this buffer won't be free until the end of this program
	*********************************************************************************************/
	FILE* original_file = fopen(original_file_name, "r");
	// set the file pointer to the end
	fseek(original_file , 0 , SEEK_END);
	// get the file size
	long file_size = ftell(original_file);
	// reset file pointer
	rewind(original_file);
	// allocate memory to contain the whole file
	char* buffer = (char*) malloc(sizeof(char) * (file_size + 1));
	buffer[file_size] = '\0';
	// allocate memory failed
	if (buffer == NULL) {
		cout << "Memory error" << endl;
		exit(0);
	}
	// read file and the reult is the number of chars
	fread(buffer, 1, file_size, original_file); 
	fclose(original_file);
	char last_char = buffer[file_size-1];
	cout << "Read File Done!" << endl;
	
	
	/*********************************************************************************************
	bucket sort
	create 128 buckets(0-127) for 128 characters
	for instance: the ascii value of character A is 65, then file 65 is the bucket for A.
	*********************************************************************************************/
	FILE** buckets = (FILE**)malloc(sizeof(FILE*) * 128);
	for(int i=0; i<128; i++){
		// construct bucket file path
		char bucket_id[4] = {0};
		sprintf(bucket_id, "%d", i);
		char bucket_file_path[MAX_PATH_LENGTH] = {0}; 
		strcpy(bucket_file_path, temp_folder_name);
		strcat(bucket_file_path, "/");
		strcat(bucket_file_path, bucket_id);
		buckets[i] = fopen(bucket_file_path, "a+b");
	}
	// assigne each index to related buckets
	for(int i=0; i<file_size; i++){
		int this_char = buffer[i];
		FILE* bucket = buckets[this_char];
		fwrite(&i, sizeof(int), 1, bucket);
	}
	cout << "Bucket Done!" << endl;
	
	
	
	/*********************************************************************************************
	sort each bucket by quick sort
	then write bwt file
	*********************************************************************************************/
	// open bwt file  
	FILE* bwt_file = fopen(bwt_file_name, "w");
	// position file path
	char aux_file_path[MAX_PATH_LENGTH] = {0};
	strcpy(aux_file_path, bwt_file_name);
	strcat(aux_file_path, ".aux");
	// open position file 
	FILE* aux_file = fopen(aux_file_path, "wb");
	
	FILE* delimiter_file = buckets[delimiter];
	// set the file pointer to the end
	fseek(delimiter_file , 0 , SEEK_END);
	// get the file size
	long delimiter_file_size = ftell(delimiter_file);
	// reset file pointer
	rewind(delimiter_file);
	// malloc an array to read the index in buckets
	int number_of_delimiters = delimiter_file_size / sizeof(int);
	int* delimiter_array = (int*)malloc(sizeof(int) * number_of_delimiters);
	fread(delimiter_array, sizeof(int), number_of_delimiters, delimiter_file);
	
	for(int i=0; i<128; i++){
		cout << "Sorting character " << (char)i << endl;
		FILE* bucket = buckets[i];
		// set the file pointer to the end
		fseek(bucket , 0 , SEEK_END);
		// get the file size
		long bucket_file_size = ftell(bucket);
		// reset file pointer
		rewind(bucket);
		
		// if this bucket is empty, it does not need to be sorted 
		if(bucket_file_size){
			// malloc an array to read the index in buckets
			int number_of_indexes = bucket_file_size / sizeof(int);
			int* index_array = (int*)malloc(sizeof(int) * number_of_indexes);
			fread(index_array, sizeof(int), number_of_indexes, bucket);
			
			// malloc an array to store suffix structs
			struct Suffix* suffixes = (struct Suffix*)malloc(sizeof(struct Suffix) * number_of_indexes);
			for (int j=0; j<number_of_indexes; j++) {
				suffixes[j].str = &buffer[index_array[j]];
				suffixes[j].postion = index_array[j];
			}
			
			// we dont need to sort delimiter
			if(i != delimiter){
				// sort these suffixes by quick sort
				QuickSort(suffixes, number_of_indexes, delimiter);
			}
			
			//write bwt file
			for (int j=0; j<number_of_indexes; j++) {
				int this_position;
				if(suffixes[j].postion == 0){
					this_position = file_size - 1;
				} else {
					this_position = suffixes[j].postion - 1;
				}
				char this_char = buffer[this_position];
				fwrite(&this_char, sizeof(char), 1, bwt_file);
				
				if((int)this_char == delimiter){
					int delimiter_position = BinarySearch(delimiter_array, 0, number_of_delimiters-1, this_position); 
					fwrite(&delimiter_position, sizeof(int), 1, aux_file);
				}
			}
			
			// free struct array and index array
			free(suffixes);
			free(index_array);
		}
	}
	cout << "Bwt File Done!" << endl;
	
	
	/*********************************************************************************************
	free memory and delete files
	*********************************************************************************************/
	free(delimiter_array);
	fclose(bwt_file);
	fclose(aux_file);
	for(int i=0; i<128; i++){
		// close file
		fclose(buckets[i]);
		// delete file
		char bucket_id[4] = {0};
		sprintf(bucket_id, "%d", i);
		char bucket_file_path[MAX_PATH_LENGTH] = {0}; 
		strcpy(bucket_file_path, temp_folder_name);
		strcat(bucket_file_path, "/");
		strcat(bucket_file_path, bucket_id);
		remove(bucket_file_path);
	}
	free(buckets);
	free(buffer);
}