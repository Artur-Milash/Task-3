#include "Code_reader.h"
#include <iostream>

int main(){
	Code_reader reader{ {"D:/Projects/CPP/Softserve/Task3"},{".vs", ".git"}, "Multi_thr_code_reader.txt"};
	reader.check();
	
}