// C++ program for building suffix array of a given text
#include <iostream>
#include <cstring>
#include <algorithm>
using namespace std;

// Structure to store information of a suffix
struct suffix
{
	int index; // To store original index
	int rank[2]; // To store ranks and next rank pair
};

// A comparison function used by sort() to compare two suffixes
// Compares two pairs, returns 1 if first pair is smaller
int cmp(struct suffix a, struct suffix b)
{
	return (a.rank[0] == b.rank[0])? (a.rank[1] < b.rank[1] ?1: 0):(a.rank[0] < b.rank[0] ?1: 0);
}

// This is the main function that takes a string 'txt' of size n as an
// argument, builds and return the suffix array for the given string
int *buildSuffixArray(char *txt, long n)
{
	// A structure to store suffixes and their indexes
	struct suffix suffixes[n];

	// Store suffixes and their indexes in an array of structures.
	// The structure is needed to sort the suffixes alphabatically
	// and maintain their old indexes while sorting
	for (int i = 0; i < n; i++)
	{
		suffixes[i].index = i;
		suffixes[i].rank[0] = txt[i] - 'a';
		suffixes[i].rank[1] = ((i+1) < n)? (txt[i + 1] - 'a'): -1;
	}

	// Sort the suffixes using the comparison function
	// defined above.
	sort(suffixes, suffixes+n, cmp);

	// At this point, all suffixes are sorted according to first
	// 2 characters. Let us sort suffixes according to first 4
	// characters, then first 8 and so on
	int ind[n]; // This array is needed to get the index in suffixes[]
				// from original index. This mapping is needed to get
				// next suffix.
	for (int k = 4; k < 2*n; k = k*2)
	{
		// Assigning rank and index values to first suffix
		int rank = 0;
		int prev_rank = suffixes[0].rank[0];
		suffixes[0].rank[0] = rank;
		ind[suffixes[0].index] = 0;

		// Assigning rank to suffixes
		for (int i = 1; i < n; i++)
		{
			// If first rank and next ranks are same as that of previous
			// suffix in array, assign the same new rank to this suffix
			if (suffixes[i].rank[0] == prev_rank && suffixes[i].rank[1] == suffixes[i-1].rank[1])
			{
				prev_rank = suffixes[i].rank[0];
				suffixes[i].rank[0] = rank;
			}
			else // Otherwise increment rank and assign
			{
				prev_rank = suffixes[i].rank[0];
				suffixes[i].rank[0] = ++rank;
			}
			ind[suffixes[i].index] = i;
		}

		// Assign next rank to every suffix
		for (int i = 0; i < n; i++)
		{
			int nextindex = suffixes[i].index + k/2;
			suffixes[i].rank[1] = (nextindex < n)?suffixes[ind[nextindex]].rank[0]: -1;
		}

		// Sort the suffixes according to first k characters
		sort(suffixes, suffixes+n, cmp);
	}

	// Store indexes of all sorted suffixes in the suffix array
	int *suffixArr = new int[n];
	for (int i = 0; i < n; i++)
		suffixArr[i] = suffixes[i].index;

	// Return the suffix array
	return suffixArr;
}

// A utility function to print an array of given size
void printArr(int arr[], long n)
{
	for (int i = 0; i < n; i++)
		cout << arr[i] << " ";
	cout << endl;
}

// Driver program to test above functions
int main()
{
	FILE* orginal_file = fopen("original/1mb.txt", "r");
	// set the file pointer to the end
	fseek(orginal_file , 0 , SEEK_END);
	// get the file size
	long file_size = ftell(orginal_file);
	// reset file pointer
	rewind(orginal_file);
	// allocate memory to contain the whole file
	char* buffer = (char*) malloc(sizeof(char) * (file_size + 1));
	// allocate memory failed
	if (buffer == NULL) {
		cout << "Memory error" << endl;
		exit(0);
	}
	// read file and the reult is the number of chars
	fread(buffer, 1, file_size, orginal_file); 
	buffer[file_size] = '\0';
	cout << 1 << endl;
	//cout << buffer << endl;
	//char txt[] = "banana";
	//int n = strlen(buffer);
	int *suffixArr = buildSuffixArray(buffer, file_size);

	printArr(suffixArr, file_size);
	return 0;
}
