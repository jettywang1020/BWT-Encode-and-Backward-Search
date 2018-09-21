#include <iostream>
#include <map>
#include <sstream>
#include <array>

using namespace std;
int main(int argc, char *argv[]) {
	
	char** ps = (char**) malloc(sizeof(char)*3);
	
	char a[] = "ISSISSIPPI$";
	char* aa = a;
	char b[] = "ISSIPPI$";
	char* bb = b;
	char c[] = "IPPI$";
	char* cc = c;
	
	ps[0] = aa;
	ps[1] = bb;
	ps[2] = cc;
	
	sort(ps, ps+3); 
	for (int i = 0; i < 3; ++i) 
		cout << ps[i] << " ";
	
	return 0; 
}