#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 128
#define BUFFER_SIZE 128

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
	
	
	/********************************************************************************************/
	/* 
	read original file into a dynamic size buffer
	this buffer costs 50MB memory at most and 50MB is much smaller than 2^32-1
	so we could convert the long type file size to int size safely
	and this buffer won't be free until the end of this program
	*/
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
	
	/********************************************************************************************/
	/*
	bucket sort
	create 128 buckets(0-127) for S type characters
	create 128 buckets(128-255) for sorted S type characters
	create 128 buckets(256-383) for sorted L type characters
	for instance: the bucket of character A is 65 for S type, and 193(65+128) for L type
	assign the index of each S type character (ignore L type) to different buckets according ascii value
	*/
	FILE** buckets = (FILE**)malloc(sizeof(FILE*) * 384);
	for(int i=0; i<384; i++){
		char bucket_id[4] = {0};
		sprintf(bucket_id, "%d", i);
		char bucket_file_path[MAX_PATH_LENGTH] = {0}; 
		strcpy(bucket_file_path, temp_folder_name);
		strcat(bucket_file_path, "/");
		strcat(bucket_file_path, bucket_id);
		buckets[i] = fopen(bucket_file_path, "a+b");
	}
	// true: S type; false: L type
	bool type;
	// count how many L type characters for each letter
	int l_type_occ[128] = {0};
	for(int i=file_size-1; i>=0 ; i--){
		char current_char = buffer[i];
		// we assue that all the delimiters are S type and they need to be sorted
		if((int)current_char == delimiter){
			type = true;
		} else {
			char next_char = buffer[i+1];
			if( current_char > next_char ){
				type = false;
			} 
			if (current_char < next_char){
				type = true;
			}
		}
		// write S type into buckets and ignore L type because we only need to sort S type
		if(type){
			FILE* bucket = buckets[current_char];
			fwrite(&i, sizeof(int), 1, bucket);
		}else{
			l_type_occ[current_char]++;
		}
	}
	cout << "Bucket Done!" << endl;
	
	
	/********************************************************************************************/
	/*
	quick sort
	sort each S type bucket(0-127)
	then overwrite each bucket by the indexes of sorted S type characters
	*/
	bool* exist_table = (bool*)malloc(sizeof(bool) * file_size);
	for(int i=0; i<128; i++){
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
			
			// sort these suffixes by quick sort
			QuickSort(suffixes, number_of_indexes, delimiter);
			
			// overwrite S type buckets
			for (int j=0; j<number_of_indexes; j++) {
				index_array[j] = suffixes[j].postion;
				exist_table[suffixes[j].postion] = true;
			}
			// free struct arrays
			free(suffixes);
			// write new order
			FILE* temp = buckets[i+128];
			fwrite(index_array, sizeof(int), number_of_indexes, temp);
			free(index_array);
		}
	}
	cout << "Quick Sort Done!" << endl;
	
	
	
	/********************************************************************************************/
	/*
	putting L type characters at the right postions in L type buckets(255-2383)
	write bwt file at the same time
	*/
	FILE* bwt_file = fopen(bwt_file_name, "w");
	for(int i=0; i<128; i++){
		// check L type bucket firstly
		if(l_type_occ[i] != 0){
			FILE* l_bucket = buckets[i+256];
			// set the file pointer to the end
			fseek(l_bucket , 0 , SEEK_END);
			// get the file size
			long l_bucket_file_size = ftell(l_bucket);
			// reset file pointer
			rewind(l_bucket);
			int l_number_of_indexes = l_bucket_file_size / sizeof(int);
			int* l_index_array = (int*)malloc(sizeof(int) * l_number_of_indexes);
			fread(l_index_array, sizeof(int), l_number_of_indexes, l_bucket);
			

			int rest_number = l_type_occ[i] - l_number_of_indexes;
			int* rest_index_array = (int*)malloc(sizeof(int) * rest_number);
			
			int offset = 0;
			for (int j=0; j<l_number_of_indexes; j++) {
				// if it is not the first character
				if(l_index_array[j] != 0){
					char this_char = buffer[l_index_array[j]];
					cout << this_char << endl;
					// if the position of previous character is not setted
					int prev_index = l_index_array[j] - 1;
					if(exist_table[prev_index] == false){
						int prev_char = buffer[prev_index];
						if(prev_char == i){
							// append the index into current bucket
							rest_index_array[offset] = prev_index;
							offset++;
						}
						// append the index into L type buctet
						FILE* temp_bucket = buckets[prev_char+256];
						fwrite(&prev_index, sizeof(int), 1, temp_bucket);
						exist_table[prev_index] = true;
					}
				}else{
					cout << last_char << endl;
				}
			}
			
			
			//change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//change here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//			while(offset != 0){
//				new_offset = 0;
//				for (int j=0; j<offset; j++) {
//					char this_char = buffer[rest_index_array[j]];
//					
//					cout << this_char << endl;
//				}
//				offset = new_offset;
//			}
			
			free(rest_index_array);
			free(l_index_array);
		}

		

		// check S type bucket secondly
		FILE* s_bucket = buckets[i+128];
		// set the file pointer to the end
		fseek(s_bucket , 0 , SEEK_END);
		// get the file size
		long s_bucket_file_size = ftell(s_bucket);
		// reset file pointer
		rewind(s_bucket);
		// malloc an array to read the index in buckets
		int s_number_of_indexes = s_bucket_file_size / sizeof(int);
		int* s_index_array = (int*)malloc(sizeof(int) * s_number_of_indexes);
		fread(s_index_array, sizeof(int), s_number_of_indexes, s_bucket);
		
		for (int j=0; j<s_number_of_indexes; j++) {
			// if it is not the first character
			if(s_index_array[j] != 0){
				char this_char = buffer[s_index_array[j]];
				cout << this_char << endl;
				// if the position of previous character is not setted
				int prev_index = s_index_array[j] - 1;
				if(exist_table[prev_index] == false){
					int prev_char = buffer[prev_index];
					// append the index into L type buctet
					FILE* temp_bucket = buckets[prev_char+256];
					fwrite(&prev_index, sizeof(int), 1, temp_bucket);
					exist_table[prev_index] = true;
				}
			}else{
				char this_char = buffer[s_index_array[j]];
				cout << this_char << endl;
			}
		}
		free(s_index_array);
	}
	fclose(bwt_file);
	cout << "BWT File Done! " << endl;

	
	
	/********************************************************************************************/
	/*
	free memory and delete files
	*/
	for(int i=0; i<384; i++){
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