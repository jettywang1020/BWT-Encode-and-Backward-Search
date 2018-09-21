#include <iostream>
#include <map>
#include <array>
#include <sstream>
using namespace std;

#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 128
#define BUFFER_SIZE 200

int FirstSort(int, const char*, const char*, map<int, int>*);
long SecondSort(const char*, const char*, map<array<int, 2>, string>*);
void BWTCode(int, long, const char*, const char*, const char*, map<array<int, 2>, string>*, map<int, int>);

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
	map<int,int> delimiter_table;
	int no_of_delimiter = FirstSort(delimiter, temp_folder_name, orginal_file_name, &delimiter_table);
	cout << 1 << endl;
	// do third sort
	map<array<int, 2>, string> rank_table;
	long file_size = SecondSort(temp_folder_name, orginal_file_name, &rank_table);
	cout << 2 << endl;
	// get BWT code
	BWTCode(delimiter, file_size, temp_folder_name, orginal_file_name, bwt_file_name, &rank_table, delimiter_table);
	cout << 3 << endl;

}

int FirstSort(int delimiter, const char* temp_folder_name, const char* orginal_file_name, map<int, int>* delimiter_table){
	FILE* orginal_file = fopen(orginal_file_name, "r");
	// set the file pointer to the end
	fseek(orginal_file , 0 , SEEK_END);
	// get the file size
	long file_size = ftell(orginal_file);
	// reset file pointer
	rewind(orginal_file);
	// allocate memory to contain the whole file
	char* buffer = (char*) malloc(sizeof(char) * (file_size + 1));
	buffer[file_size] = '\0';
	// allocate memory failed
	if (buffer == NULL) {
		cout << "Memory error" << endl;
		exit(0);
	}
	// read file and the reult is the number of chars
	fread(buffer, 1, file_size, orginal_file); 
	
	cout << "Read File Done!" << endl;
	
	/* 
	build a map to store ranks and index: key:<1,0,1>, value:"1 2 3"
	first int is current rank, 
	second int is only for counting the index of delimiter, 
	third int is next rank.
	values is a string constructed by index
	*/
	map<array<int, 2>, string> rank_table;
	int delimiter_index = 0;
	
	for (int i = 0; i < file_size; i++) {
		int current_rank = buffer[i];
		int next_rank;
		if( i + 1 == file_size){
			next_rank = 0;
		} else {
			next_rank = buffer[i+1];
		}
		
		char index[16] = {0};
		// convert index to index string
		sprintf(index, "%d", i);
		
		array<int, 2> key = {0, 0};
		if(delimiter == current_rank){
			key[0] = current_rank;
			key[1] = next_rank;
			delimiter_index ++ ;
			(*delimiter_table).insert(map<int, int> :: value_type(i, delimiter_index));
		} else {
			key[0] = current_rank;
			key[1] = next_rank;
		}
		
		if ( rank_table.find(key) == rank_table.end() ) {
			rank_table.insert(map<array<int, 2>, string> :: value_type(key, index));
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
	cout << "Build Map Done!" << endl;
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
	map<array<int, 2>, string>::iterator it;
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
	cout << "Rank File Done!" << endl;
	fclose(current_rank_file);
	return delimiter_index;
}

// store information in the format: <rank next_rank; index> (<filename, content>), where next_rank is the rank of current_index+2
long SecondSort(const char *temp_folder_name, const char *orginal_file_name, map<array<int, 2>, string>* rank_table){
	FILE* orginal_file = fopen(orginal_file_name, "r");
	// file does not exist
	if (orginal_file == NULL) {
		cout << "File error" << endl; 
		exit(0);
	}
	fseek(orginal_file , 0 , SEEK_END);
	// get the file size
	long file_size = ftell(orginal_file);
	// reset file pointer
	rewind(orginal_file);
	fclose(orginal_file);
	
	char current_rank_file_path[MAX_PATH_LENGTH] = {0};
	strcpy(current_rank_file_path, temp_folder_name);
	strcat(current_rank_file_path, "/");
	strcat(current_rank_file_path, "current_rank");
	FILE* current_rank_file = fopen(current_rank_file_path, "rb");

	int rank;
	for (int i = 0; i < file_size; i++) {		
		int next_rank;
		int current_rank;
		
		fseek(current_rank_file, i * 4, SEEK_SET);
		fread(&current_rank, sizeof(int), 1, current_rank_file);
		
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
			(*rank_table).insert(map<array<int, 2>, string> :: value_type(key, index));
		} else {
			string temp = (*rank_table)[key];
			int n = temp.length(); 
			char index_string[n+10]; 

			strcpy(index_string, temp.c_str());
			strcat(index_string, " ");
			strcat(index_string, index);
			(*rank_table)[key] = index_string;
		}
	}
	
	fclose(current_rank_file);
	remove(current_rank_file_path);
	return file_size;
}

// this function is for sort comparing
int cmp(char* pointer1, char* pointer2)
{
	int i = 36;
	while (pointer1[i] != '\0' && pointer2[i] != '\0') {
		if (pointer1[i] < pointer2[i]){
			return 1;
		} else if (pointer1[i] > pointer2[i]){
			return 0;
		} else {
			i++;
			continue;
		}
	}
	return 0;
}

// build BWT codes
// read files(geneated by last step) one by one, and we will get suffix array
// BWT is suffix array - 1
void BWTCode(int delimiter, long file_size, const char* temp_folder_name, const char*  orginal_file_name, const char* bwt_file_name, map<array<int, 2>, string>* rank_table,  map<int, int> delimiter_table){
	char postion_file_path[MAX_PATH_LENGTH] = {0};
	strcpy(postion_file_path, temp_folder_name);
	strcat(postion_file_path, "/");
	strcat(postion_file_path, "postion.aux ");
	FILE* postion_file = fopen(postion_file_path, "wb");
	
	FILE* orginal_file = fopen(orginal_file_name, "rb");
	FILE* bwt_file = fopen(bwt_file_name, "wb");
	
	map<array<int, 2>, string>::iterator it;
	for ( it = rank_table->begin(); it != rank_table->end(); it++ ){
		int no_of_index = 1;
		for (int i=0; i< it->second.length(); i++) {
			if(it->second[i] == ' '){
				no_of_index ++ ;
			}
		}

		if(no_of_index == 1){
			stringstream temp(it->second);
			int index = 0;
			temp >> index;
			
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
			
			int ascii = current_char;
			if(ascii == delimiter){
				int delimiter_index = delimiter_table[postion];
				fwrite(&delimiter_index, sizeof(int), 1, postion_file);
			}
		} else {
			char** pointers = new char*[no_of_index];
			stringstream ss(it->second);
			string index_string;
			int ith = 0;
			
			while (getline(ss, index_string, ' ')) {
				stringstream temp(index_string);
				int index = 0;
				temp >> index;
				

				int b_size = BUFFER_SIZE;
				if (file_size - index < BUFFER_SIZE){
					b_size = file_size - index;
				}
				char* buffer = new char[b_size + 1];
				fseek(orginal_file, index, SEEK_SET);
				fread(buffer, sizeof(char), b_size, orginal_file); 
				buffer[b_size] = '\0';
				
				string eight_bits = bitset<32>(index).to_string();
				char* substring = new char[40 + BUFFER_SIZE];
				strcpy(substring, eight_bits.c_str());
				strcat(substring, buffer);
				
				pointers[ith] = substring;
				ith++;
				delete[] buffer;
			}
			
			// sort these substrings
			sort(pointers, pointers + no_of_index, cmp); 
			for (int i = 0; i < no_of_index; i++) {
				string temp = pointers[i];
				
				int length = temp.length();
				
				string index_string = temp.substr (0,32);
				int index = stoi(index_string, nullptr, 2);

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
				
				int ascii = current_char;
				if(ascii == delimiter){
					int delimiter_index = delimiter_table[postion];
					fwrite(&delimiter_index, sizeof(int), 1, postion_file);
				}
			}

			for (int i = 0; i < no_of_index; i++) {
				delete[] pointers[i];
			}
			delete[] pointers;
		}
	}
	
	fclose(postion_file);
	fclose(orginal_file);
	fclose(bwt_file);
}

