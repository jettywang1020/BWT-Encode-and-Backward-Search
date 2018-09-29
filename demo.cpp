#include <iostream>
#include <map>
#include <sstream>
#include <array>

using namespace std;

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
		//if (strcmp(suffixes[i].str, suffixes[len-1].str) < 0){
			Swap(&suffixes[i], &suffixes[pvt++]);
		}
	}

	// move the pivot value into its place
	Swap(&suffixes[pvt], &suffixes[len-1]);

	// and invoke on the subsequences. does NOT include the pivot-slot
	QuickSort(suffixes, pvt++, delimter);
	QuickSort(suffixes + pvt, len - pvt, delimter);
}


int main () 
{ 
    char str[] = "ab$acb$abb$";
	
	char *buffer = str;

	int delimiter = 36;

	struct Suffix* suffixes = (struct Suffix*)malloc(sizeof(struct Suffix)*11);
	
	
	for (int i=0; i<11; i++) {
		suffixes[i].str = &buffer[i];
		suffixes[i].postion = i;
	}

	QuickSort(suffixes, 11, delimiter);
	for (int i=0; i<11; i++) {
		cout << suffixes[i].str << endl;
	}
	
	return 0; 
} 