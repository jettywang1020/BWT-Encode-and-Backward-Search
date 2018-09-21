#include <iostream>
using namespace std;

#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 128
#define BUFFER_SIZE 128

int FristSort(const char*, const char*, bool []);
void NewRanks(const char*, bool [], int);
int SecondSort(const char*, int);
void BWTCode(const char*, const char*, const char*, int, int);

int main(int argc, char *argv[]) {
	// read arguments
	const char* delimiter = argv[1];
	const char* temp_folder_name = argv[2];
	const char* orginal_file_name = argv[3];
	const char* bwt_file_name = argv[4];
	
	// build an int array to remember the occurence of a char, if a character appears, it is true, otherwise, false.
	// the first one is 0(EOF), the second one \n, the third one is \s, the forth one is !, and so on
	bool alphabets[97] = {0};
	// do first sort
	int num_of_chars = FristSort(temp_folder_name, orginal_file_name, alphabets);
	// do second sort
	NewRanks(temp_folder_name, alphabets, num_of_chars);
	// do third sort
	int max_rank = SecondSort(temp_folder_name, num_of_chars);
	// get BWT code
	BWTCode(temp_folder_name, orginal_file_name, bwt_file_name, max_rank, num_of_chars);
}


// sort according to first two characters
// store information in the format: <rank next _ank; index> (<filename, content>)
int FristSort(const char *temp_folder_name, const char *orginal_file_name, bool alphabets[]){
	FILE* orginal_file = fopen(orginal_file_name, "r");

	int index = 0;
	// read first character
	char current_char = fgetc(orginal_file);
	int current_rank = current_char;
	if (current_rank != 10){
		// char from 32 to 126
		if ( 32 <= current_rank && current_rank <= 126){
			current_rank = current_rank -30;
		} else {
			cout << "Invalid char: " << current_char << endl;
			exit(0);
		}
	} else {
		// \n
		current_rank = 1;
	}
	alphabets[current_rank] = true;
	char next_char;
	int next_rank;
	while (!feof(orginal_file)) {
		// read second character
		next_char = fgetc(orginal_file);
		next_rank = next_char;
		if (next_rank != -1){
			if (next_rank != 10){
				// char from 32 to 126
				if ( 32 <= next_rank && next_rank <= 126){
					next_rank = next_rank -30;
				} else {
					cout << "Invalid char: " << next_char << endl;
					exit(0);
				}
			} else {
				// \s
				next_rank = 1;
			}
		} else{
			next_rank = 0;
		}
		alphabets[next_rank] = true;
		
		// construct the rank file path to store ranks while the filename is combineb by ranks, eg. tempfolder/2_1, and in this file is index
		char rank_file_path[MAX_PATH_LENGTH] = {0};
		char current_rank_string[MAX_FILENAME_LENGTH] = {0}; 
		char next_rank_string[MAX_FILENAME_LENGTH] = {0}; 
		// convert rank(int) to char array
		sprintf(current_rank_string, "%d", current_rank); 
		sprintf(next_rank_string, "%d", next_rank); 
		strcpy(rank_file_path, temp_folder_name);
		strcat(rank_file_path, "/");
		strcat(rank_file_path, current_rank_string);
		strcat(rank_file_path, "_");
		strcat(rank_file_path, next_rank_string);
		// use option a+ to append new lines.
		FILE* rank_file = fopen(rank_file_path, "a+");
		// write index into bucket files
		fprintf(rank_file, "%d\n", index);
		fclose(rank_file);
		
		// this is for next iteration
		current_char = next_char;
		current_rank = next_rank;
		index++;
	}
	fclose(orginal_file);
	return index;
}

// sort according to first four character
// store information in the format: <index; rank> (<filename, content>)
void NewRanks(const char *temp_folder_name, bool alphabets[], int num_of_chars){
	char current_rank_file_path[MAX_PATH_LENGTH] = {0};
	strcpy(current_rank_file_path, temp_folder_name);
	strcat(current_rank_file_path, "/");
	strcat(current_rank_file_path, "current_rank.txt");
	FILE* current_rank_file = fopen(current_rank_file_path, "wb");
	
	int rank = 0;
	int line_number = 0;
	for (int i = 0; i < 97; i++) {
		if(alphabets[i]){
			for (int j = 0; j < 97; j++) {
				if(alphabets[j]){
					// construct the rank file path to read ranks while the filename is combineb by ranks, eg. tempfolder/2_1, and in this file is index
					char rank_file_path[MAX_PATH_LENGTH] = {0};
					char current_rank_string[MAX_FILENAME_LENGTH] = {0}; 
					char next_rank_string[MAX_FILENAME_LENGTH] = {0}; 
					// convert rank(int) to char array
					sprintf(current_rank_string, "%d", i); 
					sprintf(next_rank_string, "%d", j); 
					strcpy(rank_file_path, temp_folder_name);
					strcat(rank_file_path, "/");
					strcat(rank_file_path, current_rank_string);
					strcat(rank_file_path, "_");
					strcat(rank_file_path, next_rank_string);
					FILE* rank_file = fopen(rank_file_path, "r");
					
					// read the index from the rank file
					if(rank_file != NULL){
						char index_string[BUFFER_SIZE] = {0};
						while (fgets(index_string, BUFFER_SIZE, rank_file) != NULL)
						{
							line_number++;
							if(line_number == 1){
								rank ++ ;
							}
							// write rank into files
							index_string[strlen(index_string) - 1] = '\0'; // replace \n that fgets stores with \0
							int index= atol(index_string);
							fseek(current_rank_file, index * 4, SEEK_SET);
							fwrite(&rank, sizeof(int), 1, current_rank_file);
						}
						line_number = 0;
					}
					fclose(rank_file);
					remove(rank_file_path);
				}
			}
		}
	}
	
	fclose(current_rank_file);
}

// store information in the format: <rank next_rank; index> (<filename, content>), where next_rank is the rank of current_index+2
int SecondSort(const char *temp_folder_name, int num_of_chars){
	char current_rank_file_path[MAX_PATH_LENGTH] = {0};
	strcpy(current_rank_file_path, temp_folder_name);
	strcat(current_rank_file_path, "/");
	strcat(current_rank_file_path, "current_rank.txt");
	FILE* current_rank_file = fopen(current_rank_file_path, "rb");
	
	int max_rank = 1;
	int rank;
	for (int i = 0; i < num_of_chars; i++) {		
		int next_rank;
		int current_rank;
		
		fseek(current_rank_file, i * 4, SEEK_SET);
		fread(&current_rank, sizeof(int), 1, current_rank_file);
		
		if(current_rank > max_rank){
			max_rank = current_rank;
		}
		
		if( i + 2 >= num_of_chars ){
			// if there is no next suffix at index + 2, we store next rank as 0
			next_rank = 0;
		} else {
			fseek(current_rank_file, ( i + 2 ) * 4, SEEK_SET);
			fread(&next_rank, sizeof(int), 1, current_rank_file);
		}
		
		// construct the rank file path to read ranks while the filename is combineb by ranks, eg. tempfolder/2_1, and in this file is index
		char rank_file_path[MAX_PATH_LENGTH] = {0}; 
		char current_rank_string[MAX_FILENAME_LENGTH] = {0}; 
		char next_rank_string[MAX_FILENAME_LENGTH] = {0}; 
		sprintf(current_rank_string, "%d", current_rank); 
		sprintf(next_rank_string, "%d", next_rank); 
		strcpy(rank_file_path, temp_folder_name);
		strcat(rank_file_path, "/");
		strcat(rank_file_path, current_rank_string);
		strcat(rank_file_path, "_");
		strcat(rank_file_path, next_rank_string);
		strcat(rank_file_path, ".txt");
		FILE* rank_file = fopen(rank_file_path, "a+");
		// write index into bucket files
		fprintf(rank_file, "%d\n", i);
		fclose(rank_file);
	}
	
	fclose(current_rank_file);
	remove(current_rank_file_path);
	return max_rank;
}

// build BWT codes
// read files(geneated by last step) one by one, and we will get suffix array
// BWT is suffix array - 1
void BWTCode(const char* temp_folder_name, const char*  orginal_file_name, const char* bwt_file_name, int max_rank, int num_of_chars){
	FILE* orginal_file = fopen(orginal_file_name, "r");
	FILE* bwt_file = fopen(bwt_file_name, "w");

	for (int i = 0; i <= max_rank; i++) {
		for (int j = 0; j <= max_rank; j++) {
			// construct the rank file path to read ranks while the filename is combineb by ranks, eg. tempfolder/2_1, and in this file is index
			char rank_file_path[MAX_PATH_LENGTH] = {0};
			char current_rank_string[MAX_FILENAME_LENGTH] = {0}; 
			char next_rank_string[MAX_FILENAME_LENGTH] = {0}; 
			sprintf(current_rank_string, "%d", i); 
			sprintf(next_rank_string, "%d", j); 
			strcpy(rank_file_path, temp_folder_name);
			strcat(rank_file_path, "/");
			strcat(rank_file_path, current_rank_string);
			strcat(rank_file_path, "_");
			strcat(rank_file_path, next_rank_string);
			strcat(rank_file_path, ".txt");
			FILE* rank_file = fopen(rank_file_path, "r");
			
			if(rank_file != NULL){
				char index_string[BUFFER_SIZE] = {0};
				while (fgets(index_string, BUFFER_SIZE, rank_file) != NULL)
				{
					index_string[strlen(index_string)-1] = '\0';
					int index= atol(index_string);
					// reading index in the order of suffix array
					// look for the character by index-1
					int postion;
					if( index == 0){
						postion = num_of_chars - 1;
					} else {
						postion = index - 1;
					}
					fseek(orginal_file, postion, SEEK_SET);
					char current_char = fgetc(orginal_file);
					// write the last results into bwt file
					fputc(current_char, bwt_file);
				}
			}
			fclose(rank_file);
			remove(rank_file_path);
		}
	}
	fclose(orginal_file);
	fclose(bwt_file);
}