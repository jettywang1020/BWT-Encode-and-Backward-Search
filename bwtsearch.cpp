#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <set>
#include <sstream>
using namespace std;

#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 128
#define BUFFER_SIZE 1024*1024

int occ(int current_char, int pre, int *occurance[127], const char* bwt_file_name){
	int result = 0;
	
	int bsize = BUFFER_SIZE;
	
	double temp = pre / (double)(bsize);
	int rows = floor(temp);
	
	for(int i=0; i<rows; i++){
		result += occurance[i][current_char];
	}
	int start = bsize * rows;
	int offset = pre - bsize * rows;
	
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

int main(int argc, char *argv[]) {
	int delimiter;
	if(strcmp("\n", argv[1]) == 0){
		delimiter = 10;
	} else{
		delimiter = argv[1][0];
	}
	const char* bwt_file_name = argv[2];
	const char* temp_folder_name = argv[3];
	const char* model = argv[4];
	const char* pattern = argv[5];
	
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
	int *occurance[row_size];
	for( int i=0; i<row_size; i++ ) {
		occurance[i] = new int [127];
	}
	
	char bwt_text[BUFFER_SIZE] = {0};
	int size = 0;
	int itertaion = 0;
	while((size = fread(bwt_text, sizeof(char), BUFFER_SIZE, bwt_file)) > 0){
		for (int i = 0 ; i < size; i++) {
			int ascii = bwt_text[i];
			occurance[itertaion][ascii]++;
		}
		itertaion ++ ;
	}
	fclose(bwt_file);
	
	// sum of occurances of every char
	int sum_of_occurance[127] = {0};
	for (int i=0; i<row_size; i++) {
		for (int j=0; j<127; j++) {
			sum_of_occurance[j] = sum_of_occurance[j] + occurance[i][j];
		}
	}
	
	int c_table[127] = {0};
	for (int i=1; i<127; i++) {
		c_table[i] = sum_of_occurance[i-1] + c_table[i-1];
	}
	

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
		cout << last - first + 1 << endl;
	}
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
			cout << 0 << endl;
		} else if( last - first == 0 ){
			cout << 1 << endl;
		} else {
			FILE* bwt_file = fopen(bwt_file_name, "r");
			set<int> delimiters; 
			
			for(int i=first; i<=last; i++){
				int position = i;
				// do backwark decode
				while(1){
					fseek(bwt_file , position - 1, SEEK_SET);
					char temp = fgetc(bwt_file);
					int this_char = temp;
					if(this_char == delimiter){
						delimiters.insert(position); 
						break;
					} else {
						position = c_table[this_char] + occ(this_char, position - 1, occurance, bwt_file_name) + 1;
					}
				}
			}			
			cout << delimiters.size() << endl;
			fclose(bwt_file);
		}
	}
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
			cout << 0 << endl;
		} else if( last - first == 0 ){
			cout << 1 << endl;
		} else {
			char postion_file_path[MAX_PATH_LENGTH] = {0};
			strcpy(postion_file_path, temp_folder_name);
			strcat(postion_file_path, "/");
			strcat(postion_file_path, "postion.aux ");
			FILE* postion_file = fopen(postion_file_path, "rb");
			fseek(postion_file , 0 , SEEK_END);
			// get the file size
			long file_size = ftell(postion_file);
			int no_of_delimiter = file_size / sizeof(int);
			// reset file pointer
			rewind(postion_file);
			FILE* bwt_file = fopen(bwt_file_name, "r");
			
			set<int> delimiters; 
			for(int i=first; i<=last; i++){
				int position = i;
				// do backwark decode
				while(1){
					fseek(bwt_file , position - 1, SEEK_SET);
					char temp = fgetc(bwt_file);
					int this_char = temp;
					if(this_char == delimiter){
						position = occ(this_char, position - 1, occurance, bwt_file_name) + 1;
						int index_of_delimiter;
						fseek(postion_file , (position - 1) * sizeof(int), SEEK_SET);
						fread(&index_of_delimiter, sizeof(int), 1, postion_file);
						
						if ( index_of_delimiter == no_of_delimiter ){
							delimiters.insert(1); 
						} else {
							delimiters.insert(index_of_delimiter + 1); 
						}
						break;
					} else {
						position = c_table[this_char] + occ(this_char, position - 1, occurance, bwt_file_name) + 1;
					}
				}
			}
						
			set <int> :: iterator itr; 
			for (itr = delimiters.begin(); itr != delimiters.end(); ++itr) 
			{ 
				cout << *itr << endl;
			}
			fclose(postion_file);
			fclose(bwt_file);
		}
	}
	if(strcmp("-i", model) == 0){
		stringstream ss(pattern);
		string start_string;
		string end_string;
		getline(ss, start_string, ' ');
		getline(ss, end_string, ' ');
		stringstream stemp(start_string);
		int start = 0;
		stemp >> start;
		stringstream send(end_string);
		int end = 0;
		send >> end;
		
		char postion_file_path[MAX_PATH_LENGTH] = {0};
		strcpy(postion_file_path, temp_folder_name);
		strcat(postion_file_path, "/");
		strcat(postion_file_path, "postion.aux ");
		FILE* postion_file = fopen(postion_file_path, "rb");
		fseek(postion_file , 0 , SEEK_END);
		// get the file size
		long file_size = ftell(postion_file);
		int no_of_delimiter = file_size / sizeof(int);
		// reset file pointer
		rewind(postion_file);
		FILE* bwt_file = fopen(bwt_file_name, "r");
		
		for (int i = start; i <= end; i++) {
			char* record = new char[5001];
			int length = 0;
			int position = c_table[delimiter] + i;
			while(1){
				fseek(bwt_file , position - 1, SEEK_SET);
				char temp = fgetc(bwt_file);
				int this_char = temp;
				if(this_char == delimiter){
					for (int j=length-1; j>=0; j--) {
						cout << record[j];
					}
					cout << endl;
					break;
				} else {
					record[length] = temp;
					length ++ ;
					position = c_table[this_char] + occ(this_char, position, occurance, bwt_file_name);
				}
			}
		}
		
		fclose(postion_file);
		fclose(bwt_file);
	}
	
	for( int i=0; i<row_size; i++ ) {
		delete[] occurance[i];
	}
}