#include <iostream>
#include <map>
#include <array>
#include <string>
#include <sstream>
using namespace std;

#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 128
#define BUFFER_SIZE 128

long FirstSort(int, const char*, const char*);
int SecondSort(const char*, long, map<array<int, 2>, string>*);
void BWTCode(const char*, const char*, const char*, int, long, map<array<int, 2>, string>*);

int main(int argc, char *argv[]) {
	int delimiter;
	if(strcmp("\n", argv[1]) == 0){
		delimiter = 10;
	} else{
		delimiter = argv[1][0];
	}
	
	// read arguments
	//const char* delimiter = argv[1];
	const char* temp_folder_name = argv[2];
	const char* orginal_file_name = argv[3];
	const char* bwt_file_name = argv[4];
	
	// do first sort
	long file_size = FirstSort(delimiter, temp_folder_name, orginal_file_name);	
	// do third sort
	map<array<int, 2>, string> rank_table;
	int max_rank = SecondSort(temp_folder_name, file_size, &rank_table);
	// get BWT code
	BWTCode(temp_folder_name, orginal_file_name, bwt_file_name, max_rank, file_size, &rank_table);
}

long FirstSort(int delimiter, const char* temp_folder_name, const char* orginal_file_name){
	
	FILE* orginal_file = fopen(orginal_file_name, "r");
	// file does not exist
	if (orginal_file == NULL) {
		cout << "File error" << endl; 
		exit(0);
	}
	// set the file pointer to the end
	fseek(orginal_file , 0 , SEEK_END);
	// get the file size
	long file_size = ftell(orginal_file);
	// reset file pointer
	rewind(orginal_file);
	// allocate memory to contain the whole file
	char* buffer = (char*) malloc(sizeof(char) * file_size);
	// allocate memory failed
	if (buffer == NULL) {
		cout << "Memory error" << endl;
		exit(0);
	}
	// read file and the reult is the number of chars
	size_t result = fread(buffer, 1, file_size, orginal_file); 
	// file size should be as same as the number of chars
	if (result != file_size) {
		cout << "Reading error" << endl;
		exit(0);
	}
	
	/* 
	build a map to store ranks and index: key:<1,0,1>, value:"1 2 3"
	first int is current rank, 
	second int is only for counting the index of delimiter, 
	third int is next rank.
	values is a string constructed by index
	*/
	map<array<int, 3>, string> rank_table;
	int delimiter_index = 0;
	
	for (int i = 0; i < file_size; i++) {
		int current_rank = buffer[i];
		int next_rank;
		if( i + 1 == file_size){
			next_rank = 0;
		} else {
			next_rank = buffer[i+1];
		}
		
		array<int, 3> key = {0, 0, 0};
		if(delimiter == current_rank){
			key[0] = current_rank;
			key[1] = delimiter_index;
			key[2] = next_rank;
			delimiter_index ++ ;
		} else {
			key[0] = current_rank;
			key[1] = 0;
			key[2] = next_rank;
		}
		
		char index[16] = {0};
		// convert index to index string
		sprintf(index, "%d", i);
		
		if ( rank_table.find(key) == rank_table.end() ) {
			rank_table[key] = index;
		} else {
			string temp = rank_table[key];
			int n = temp.length(); 
			char index_string[n+1]; 
			strcpy(index_string, temp.c_str());  
			strcat(index_string, " ");
			strcat(index_string, index);
			rank_table[key] = index_string;
		}
	}
	fclose(orginal_file);
	free(buffer);
	
	// build current rank file
	char current_rank_file_path[MAX_PATH_LENGTH] = {0};
	strcpy(current_rank_file_path, temp_folder_name);
	strcat(current_rank_file_path, "/");
	strcat(current_rank_file_path, "current_rank");
	FILE* current_rank_file = fopen(current_rank_file_path, "wb");
	
	int rank = 0;
	int line = 0;
	map<array<int, 3>, string>::iterator it;
	for ( it = rank_table.begin(); it != rank_table.end(); it++ ){
		stringstream ss(it->second);
		string index_string;
		while (getline(ss, index_string, ' ')) {
			line++;
			if(line == 1){
				rank ++ ;
			}
			stringstream temp(index_string);
			int index = 0;
			temp >> index;
			fseek(current_rank_file, index * 4, SEEK_SET);
			fwrite(&rank, sizeof(int), 1, current_rank_file);
		}
		line = 0;
	}
	
	fclose(current_rank_file);
	return file_size;
}

// store information in the format: <rank next_rank; index> (<filename, content>), where next_rank is the rank of current_index+2
int SecondSort(const char *temp_folder_name, long file_size, map<array<int, 2>, string>* rank_table){
	char current_rank_file_path[MAX_PATH_LENGTH] = {0};
	strcpy(current_rank_file_path, temp_folder_name);
	strcat(current_rank_file_path, "/");
	strcat(current_rank_file_path, "current_rank");
	FILE* current_rank_file = fopen(current_rank_file_path, "rb");
	
	int max_rank = 1;
	int rank;
	for (int i = 0; i < file_size; i++) {		
		int next_rank;
		int current_rank;
		
		fseek(current_rank_file, i * 4, SEEK_SET);
		fread(&current_rank, sizeof(int), 1, current_rank_file);
		
		if(current_rank > max_rank){
			max_rank = current_rank;
		}
		
		if( i + 2 >= file_size ){
			// if there is no next suffix at index + 2, we store next rank as 0
			next_rank = 0;
		} else {
			fseek(current_rank_file, ( i + 2 ) * 4, SEEK_SET);
			fread(&next_rank, sizeof(int), 1, current_rank_file);
		}
		
		array<int, 2> key = {current_rank, next_rank};
		
		char index[16] = {0};
		// convert index to index string
		sprintf(index, "%d", i);
		
		if ( (*rank_table).find(key) == (*rank_table).end() ) {
			(*rank_table)[key] = index;
		} else {
			string temp = (*rank_table)[key];
			int n = temp.length(); 
			char index_string[n+1]; 
			strcpy(index_string, temp.c_str());  
			strcat(index_string, " ");
			strcat(index_string, index);
			(*rank_table)[key] = index_string;
		}
	}
	
	fclose(current_rank_file);
	remove(current_rank_file_path);
	return max_rank;
}

// build BWT codes
// read files(geneated by last step) one by one, and we will get suffix array
// BWT is suffix array - 1
void BWTCode(const char* delimiter, const char*  orginal_file_name, const char* bwt_file_name, int max_rank, long file_size, map<array<int, 2>, string>* rank_table){
	FILE* orginal_file = fopen(orginal_file_name, "r");
	FILE* bwt_file = fopen(bwt_file_name, "w");
	
	map<array<int, 2>, string>::iterator it;
	for ( it = rank_table->begin(); it != rank_table->end(); it++ ){
		stringstream ss(it->second);
		string index_string;
		while (getline(ss, index_string, ' ')) {
			stringstream temp(index_string);
			int index = 0;
			temp >> index;
			// reading index in the order of suffix array
			// look for the character by index-1
			int postion;
			if( index == 0){
				postion = file_size - 1;
			} else {
				postion = index - 1;
			}
			fseek(orginal_file, postion, SEEK_SET);
			char current_char = fgetc(orginal_file);
			// write the last results into bwt file
			fputc(current_char, bwt_file);
		}
	}
	
	fclose(orginal_file);
	fclose(bwt_file);
}

