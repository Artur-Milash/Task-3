#include "Code_reader.h"
#include <iostream>

int main(){
		std::string str{ "Multi.txt" }, check{ "D:/Projects/CPP/Softserve/Task3"};
		std::string buff;
		{
			Code_reader reader{ {check} ,{".vs", ".git"}, str};
			reader.check();
		}

}