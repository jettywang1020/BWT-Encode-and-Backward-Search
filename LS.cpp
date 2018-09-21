#include <iostream>
using namespace std;

#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 128
#define BUFFER_SIZE 128

int FristIteration(const char*, const char*, bool []);

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
	int num_of_chars = FristIteration(temp_folder_name, orginal_file_name, alphabets);
}


// sort according to first two characters
// store information in the format: <rank next _ank; index> (<filename, content>)
int FristIteration(const char *temp_folder_name, const char *orginal_file_name, bool alphabets[]){
	// Large Small file path
	char LS_file_path[MAX_PATH_LENGTH] = {0}; 
	strcpy(LS_file_path, temp_folder_name);
	strcat(LS_file_path, "/");
	strcat(LS_file_path, "LS.txt");
	// distance file path
	char distance_file_path[MAX_PATH_LENGTH] = {0}; 
	strcpy(distance_file_path, temp_folder_name);
	strcat(distance_file_path, "/");
	strcat(distance_file_path, "distance.txt");
	
	FILE* LS_file = fopen(LS_file_path, "w");
	FILE* distance_file = fopen(distance_file_path, "w");
	FILE* orginal_file = fopen(orginal_file_name, "r");

	int index = 0;
	int current_S = -1;
	int distance = 0;
	
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
			// end of file
			next_rank = 0;
		}
		alphabets[next_rank] = true;
		
		// write LS file
		char LS;
		if( current_rank > next_rank ){
			LS = 'L';
		} else {
			LS = 'S';
			current_S = index;
		}
		fwrite(&LS, sizeof(char), 1, LS_file);
		// for last character
		if(next_rank == 0){
			LS = 'S';
			fwrite(&LS, sizeof(char), 1, LS_file);
		}
		
		// write distance file
		
		
		// this is for next iteration
		current_char = next_char;
		current_rank = next_rank;
		index++;
	}
	
	fclose(LS_file);
	fclose(distance_file);
	fclose(orginal_file);
	return index;
}