#include <iostream>
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
	cout << "Read File Done!" << endl;
	
	
	/*
	bucket sort
	create 128 buckets for S type characters and another 128 buckets for L type characters
	assign the index of each character to different buckets according ascii value
	*/
	FILE** buckets = (FILE**)malloc(sizeof(FILE*) * 256);
	for(int i=0; i<256; i++){
		char bucket_id[4] = {0};
		sprintf(bucket_id, "%d", i);
		char bucket_file_path[MAX_PATH_LENGTH] = {0}; 
		strcpy(bucket_file_path, temp_folder_name);
		strcat(bucket_file_path, "/");
		strcat(bucket_file_path, bucket_id);
		buckets[i] = fopen(bucket_file_path, "a+b");
	}
	for(int i=0; i<file_size; i++){
		char current_char = buffer[i];
		char next_char = buffer[i+1];
		FILE* bucket;
		if (next_char != '\0' && (int)current_char != delimiter){
			if( current_char > next_char ){
				// L type
				continue;
			} else {
				// S type
				bucket = buckets[current_char];
			}
		} else {
			// assume that all the delimiters are S type
			bucket = buckets[current_char];
		}
		fwrite(&i, sizeof(int), 1, bucket);
	}
	cout << "Bucket Done!" << endl;
	
	
	/*
	quick sort
	sort each S type bucket(0-127)
	then overwrite each bucket by the indexes of sorted S type characters
	*/
	bool* exist_table = (bool*)malloc(sizeof(bool) * file_size);
	for(int i=0; i<128; i++){
		// if this bucket stores delimiters, it does not need to be sorted
		if(i != delimiter){
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
					cout << suffixes[j].str << endl;
				}
				
				cout << "+++++++++++++++++++++++++++++++++++++++++++" << endl;
				
				// sort these suffixes by quick sort
				QuickSort(suffixes, number_of_indexes, delimiter);
				
				// overwrite buckets
				for (int j=0; j<number_of_indexes; j++) {
					cout << suffixes[j].str << endl;
					index_array[i] = suffixes[i].postion;
					exist_table[suffixes[i].postion] = true;
				}
				fseek(bucket, 0, SEEK_SET);
				fwrite(index_array, sizeof(int), number_of_indexes, bucket);
				
				cout << "------------------------------------------" << endl;
				free(index_array);
				free(suffixes);
			}
		}
	}
	cout << "Quick Sort Done!" << endl;

	
	
	
	
	for(int i=0; i<256; i++){
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