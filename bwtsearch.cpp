#include <iostream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <set>
#include <sstream>
using namespace std;

#define MAX_PATH_LENGTH 256
#define BUFFER_SIZE 1024*4

// occ function
int occ(int current_char, int pre, int *occurance[127], const char* bwt_file_name){
	int result = 0;
	int bsize = BUFFER_SIZE;
	double temp = pre / (double)(bsize);
	int rows = floor(temp);
	
	// get result from occ table
	for(int i=0; i<rows; i++){
		result += occurance[i][current_char];
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
	
	// sum of occurances of every char
	int sum_of_occurance[127] = {0};
	for (int i=0; i<row_size; i++) {
		for (int j=0; j<127; j++) {
			sum_of_occurance[j] = sum_of_occurance[j] + occurance[i][j];
		}
	}
	
	// construct c table
	int c_table[127] = {0};
	for (int i=1; i<127; i++) {
		c_table[i] = sum_of_occurance[i-1] + c_table[i-1];
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
		cout << last - first + 1 << endl;
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
			cout << 0 << endl;
		} else if( last - first == 0 ){
			cout << 1 << endl;
		} else {
			set<int> delimiters; 
			for(int i=first; i<=last; i++){
				int position = i;
				int j = 0;
				// do backwark decode and the max length of each record is 5000
				while(j < 5000){
					fseek(bwt_file , position - 1, SEEK_SET);
					char temp = fgetc(bwt_file);
					int this_char = temp;
					// if this character is delimiter, stop, otherwise, find next character
					if(this_char == delimiter){
						delimiters.insert(position); 
						break;
					} else {
						position = c_table[this_char] + occ(this_char, position - 1, occurance, bwt_file_name) + 1;
					}
					j++;
				}
			}			
			cout << delimiters.size() << endl;
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
			cout << 0 << endl;
		} else if( last - first == 0 ){
			cout << 1 << endl;
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
			int* delimiter_array = new int [no_of_delimiter];
			fread(delimiter_array, sizeof(int), no_of_delimiter, aux_file);
			fclose(aux_file);
			
			set<int> delimiters; 
			for(int i=first; i<=last; i++){
				int position = i;
				int j = 0;
				// do backwark decode and the max length of each record is 5000
				while(j < 5000){
					fseek(bwt_file , position - 1, SEEK_SET);
					char temp = fgetc(bwt_file);
					int this_char = temp;
					// if this character is delimiter, stop, otherwise, find next character
					if(this_char == delimiter){
						position = occ(this_char, position - 1, occurance, bwt_file_name) + 1;
						// get the index of this delimiter and insert this postion into set
						int index_of_delimiter = delimiter_array[position - 1];						
						index_of_delimiter ++;
						if ( index_of_delimiter == no_of_delimiter ){
							delimiters.insert(1); 
						} else {
							delimiters.insert(index_of_delimiter + 1); 
						}
						break;
					} else {
						position = c_table[this_char] + occ(this_char, position - 1, occurance, bwt_file_name) + 1;
					}
					j++;
				}
			}
			delete[] delimiter_array;
						
			set <int> :: iterator itr; 
			for (itr = delimiters.begin(); itr != delimiters.end(); ++itr) 
			{ 
				cout << *itr << endl;
			}
		}
	}
	
	
	/*********************************************************************************************
	-i backword search
	*********************************************************************************************/
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
		
		for (int i = start; i <= end; i++) {
			char* record = new char[5001];
			int length = 0;
			int position = c_table[delimiter] + i;
			int j = 0;
			// do backwark decode
			while(j < 5000){
				fseek(bwt_file , position - 1, SEEK_SET);
				char temp = fgetc(bwt_file);
				int this_char = temp;
				// if this character is delimiter, stop, otherwise, find next character
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
				j++;
			}
		}
	}
	
	// close bwt file
	fclose(bwt_file);
	// free memory
	for( int i=0; i<row_size; i++ ) {
		delete[] occurance[i];
	}
}