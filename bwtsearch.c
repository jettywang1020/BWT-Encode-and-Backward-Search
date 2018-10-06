#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_PATH_LENGTH 256
#define BUFFER_SIZE 1024 * 3

// occ function
int occ(int current_char, int pre, int **occurance, const char* bwt_file_name){
	int bsize = BUFFER_SIZE;
	double temp = pre / (double)(bsize);
	int rows = floor(temp);
	
	// get result from occ table
	int result = 0;
	if(rows != 0){
		result = occurance[rows-1][current_char];
	}
	int start = bsize * rows;
	int offset = pre - bsize * rows;
	
	// conut rest number of this from bwt file
	FILE* bwt_file = fopen(bwt_file_name, "r");
	fseek(bwt_file , start , SEEK_SET);
	int i = 0;
	while (i < offset) {
		int this_char = fgetc(bwt_file);
		if(this_char == current_char){
			result++;
		}
		i++;
	}
	fclose(bwt_file);
	
	return result;
}

int compare (const void * a, const void * b) {
	return ( *(int*)a - *(int*)b );
}

int main(int argc, char *argv[]) {
	int delimiter;
	if(strcmp("\\n", argv[1]) == 0){
		delimiter = 10;
	} else{
		delimiter = argv[1][0];
	}
	const char* bwt_file_name = argv[2];
	const char* temp_folder_name = argv[3];
	const char* model = argv[4];
	const char* pattern = argv[5];
	
	int i, j, k;
	
	FILE* bwt_file = fopen(bwt_file_name, "r");
	fseek(bwt_file , 0 , SEEK_END);
	// get the file size
	long file_size = ftell(bwt_file);
	// reset file pointer
	rewind(bwt_file);

	int bsize = BUFFER_SIZE;
	double temp = file_size / (double)(bsize);
	int row_size = ceil(temp);
	// occ table
	int** occurance = (int**)malloc(row_size * sizeof(int*)); 
	
	for( i=0; i<row_size; i++ ) {
		occurance[i] = (int*)malloc(127 * sizeof(int)); 
	}
	// c table
	int c_table[127] = {0};
	
	
	// occ file path
	char occ_file_path[MAX_PATH_LENGTH] = {0};
	strcpy(occ_file_path, bwt_file_name);
	strcat(occ_file_path, ".occ");
	// c table file path
	char ctable_file_path[MAX_PATH_LENGTH] = {0};
	strcpy(ctable_file_path, bwt_file_name);
	strcat(ctable_file_path, ".ctable");
	
	// if occ file and c table file exist
	if( access( occ_file_path, F_OK ) != -1  && access( ctable_file_path, F_OK ) != -1) {
		FILE* occ_file = fopen(occ_file_path, "rb");
		FILE* ctable_file = fopen(ctable_file_path, "rb");
		
		// read occ file
		for (i=0; i<row_size; i++) {
			fread(occurance[i], sizeof(int), 127, occ_file);
		}
		
		// read c table file
		fread(c_table, sizeof(int), 127, ctable_file);
		
		fclose(occ_file);
		fclose(ctable_file);
	} else {
		FILE* occ_file = fopen(occ_file_path, "wb");
		FILE* ctable_file = fopen(ctable_file_path, "wb");
		
		// read bwt file and construct occ table
		char bwt_text[BUFFER_SIZE] = {0};
		int size = 0;
		int itertaion = 0;
		while((size = fread(bwt_text, sizeof(char), BUFFER_SIZE, bwt_file)) > 0){
			for (i = 0 ; i < size; i++) {
				int ascii = bwt_text[i];
				occurance[itertaion][ascii]++;
			}
			itertaion ++ ;
		}
		
		// write occ file
		for (i=0; i<row_size; i++) {
			for (j=0; j<127; j++) {
				if(i != 0){
					occurance[i][j] =  occurance[i][j] + occurance[i-1][j];
				}
			}
			fwrite(occurance[i], sizeof(int), 127, occ_file);
		}
		
		// compute c table
		for (i=1; i<127; i++) {
			c_table[i] = occurance[row_size-1][i-1] + c_table[i-1];
		}
		// write c table file
		fwrite(c_table, sizeof(int), 127, ctable_file);
		
		fclose(occ_file);
		fclose(ctable_file);
	}
	

	/*********************************************************************************************
	-m backword search
	*********************************************************************************************/
	if(strcmp("-m", model) == 0){
		int index = strlen(pattern) - 1;
		int current_char = pattern[index];
		int first = c_table[current_char] + 1;
		int last = c_table[current_char + 1];

		while (first <= last && index >0 ) {
			index--;
			current_char = pattern[index];
			first = c_table[current_char] + occ(current_char, first - 1, occurance, bwt_file_name) + 1;
			last = c_table[current_char] + occ(current_char, last, occurance, bwt_file_name);
		}
		printf("%d\n", last - first + 1);
	}
	
	/*********************************************************************************************
	-n backword search
	*********************************************************************************************/
	if(strcmp("-n", model) == 0){		
		int index = strlen(pattern) - 1;
		int current_char = pattern[index];
		int first = c_table[current_char] + 1;
		int last = c_table[current_char + 1];

		while (first <= last && index >0 ) {
			index--;
			current_char = pattern[index];
			first = c_table[current_char] + occ(current_char, first - 1, occurance, bwt_file_name) + 1;
			last = c_table[current_char] + occ(current_char, last, occurance, bwt_file_name);
		}		
		if( last - first + 1 == 0 ){
			printf("0\n");
		} else if( last - first == 0 ){
			printf("1\n");
		} else {
			int* delimiters = (int*)malloc((last - first + 1) * sizeof(int));
			for(i=first; i<=last; i++){
				int position = i;
				j = 0;
				// do backwark decode and the max length of each record is 5000
				while(j < 5000){
					fseek(bwt_file , position - 1, SEEK_SET);
					char temp = fgetc(bwt_file);
					int this_char = temp;
					// if this character is delimiter, stop, otherwise, find next character
					if(this_char == delimiter){
						delimiters[i-first] = position + 1; 
						break;
					} else {
						position = c_table[this_char] + occ(this_char, position - 1, occurance, bwt_file_name) + 1;
					}
					j++;
				}
			}			

			// sort the index of demiliters and remove reduance
			qsort(delimiters, last - first + 1, sizeof(int), compare);
			int previous = 0;
			int counter = 0;
			for(i=0; i<last - first + 1; i++){
				if(delimiters[i] != previous){
					previous = delimiters[i];
					counter++;
				}
			}
			printf("%d\n", counter);
			
			free(delimiters);
		}
	}
	
	/*********************************************************************************************
	-a backword search
	*********************************************************************************************/
	if(strcmp("-a", model) == 0){
		int index = strlen(pattern) - 1;
		int current_char = pattern[index];
		int first = c_table[current_char] + 1;
		int last = c_table[current_char + 1];

		while (first <= last && index > 0 ) {
			index--;
			current_char = pattern[index];
			first = c_table[current_char] + occ(current_char, first - 1, occurance, bwt_file_name) + 1;
			last = c_table[current_char] + occ(current_char, last, occurance, bwt_file_name);
		}
		if( last - first + 1 == 0 ){
			exit(0);
		} else {
			// aux position file path
			char aux_file_path[MAX_PATH_LENGTH] = {0};
			strcpy(aux_file_path, bwt_file_name);
			strcat(aux_file_path, ".aux");
			FILE* aux_file = fopen(aux_file_path, "rb");
			// get the file size
			fseek(aux_file , 0 , SEEK_END);
			long aux_file_size = ftell(aux_file);
			// reset file pointer
			rewind(aux_file);
			// malloc an array to store the index of delimters
			int no_of_delimiter = aux_file_size / sizeof(int);
			
			int* delimiters = (int*)malloc((last - first + 1) * sizeof(int));
			for(i=first; i<=last; i++){
				int position = i;
				j = 0;
				// do backwark decode and the max length of each record is 5000
				while(j < 5000){
					fseek(bwt_file , position - 1, SEEK_SET);
					char temp = fgetc(bwt_file);
					int this_char = temp;
					// if this character is delimiter, stop, otherwise, find next character
					if(this_char == delimiter){
						position = occ(this_char, position - 1, occurance, bwt_file_name) + 1;
						// get the index of this delimiter and insert this postion into set
						int index_of_delimiter;
						fseek(aux_file, sizeof(int) * (position - 1), SEEK_SET);
						fread(&index_of_delimiter, sizeof(int), 1, aux_file);
						index_of_delimiter ++;
						if ( index_of_delimiter == no_of_delimiter ){
							delimiters[i-first] =  1; 
						} else {
							delimiters[i-first] =  index_of_delimiter + 1; 
						}
						break;
					} else {
						position = c_table[this_char] + occ(this_char, position - 1, occurance, bwt_file_name) + 1;
					}
					j++;
				}
			}
			fclose(aux_file);
						
			// sort the index of demiliters and remove reduance
			qsort(delimiters, last - first + 1, sizeof(int), compare);
			int previous = 0;
			for(i=0; i<last - first + 1; i++){
				if(delimiters[i] != previous){
					previous = delimiters[i];
					printf("%d\n", delimiters[i]);
				}
			}
			free(delimiters);
		}
	}
	
	
	/*********************************************************************************************
	-i backword search
	*********************************************************************************************/
	if(strcmp("-i", model) == 0){
		// extract start postion and end postion
		int start, end = 4;
		char input[128] = {0}; 
		strcpy(input, pattern);
		char *split;
		split = strtok(input, " ");
		if(split){
			sscanf(split, "%d", &start);
		}
		split = strtok(NULL, " ");
		if(split){
			sscanf(split, "%d", &end);
		}
		
		for (i = start; i <= end; i++) {
			char* record = (char*)malloc(5000 * sizeof(char));
			int length = 0;
			int position = c_table[delimiter] + i;
			j = 0;
			// do backwark decode
			while(j < 5000){
				fseek(bwt_file , position - 1, SEEK_SET);
				char temp = fgetc(bwt_file);
				int this_char = temp;
				// if this character is delimiter, stop, otherwise, find next character
				if(this_char == delimiter){
					for (k=length-1; k>=0; k--) {
						printf("%c", record[k]);
					}
					printf("\n");
					break;
				} else {
					record[length] = temp;
					length ++ ;
					position = c_table[this_char] + occ(this_char, position, occurance, bwt_file_name);
				}
				j++;
			}
			free(record);
		}
	}
	
	// close bwt file
	fclose(bwt_file);
	// free memory
	for(i=0; i<row_size; i++) {
		free(occurance[i]);
	}
	free(occurance);
}