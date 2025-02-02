#include "Code_reader.h"
#include <iostream>

int main() {
	std::string save{ "Multi.txt" }, check{ "D:/Projects/CPP/Softserve/Task3" };
	std::string buff;
	
	Code_reader reader{ {check} ,{".vs", ".git"}, save};
	reader.set_priority(CODE_READER_COMMENT_PRIORITY);
	reader.check();

	std::cout << "File processed: " << reader.get_file_processed()
		<< "\nCode lines: " << reader.get_code_amount()
		<< "\nComment lines: " << reader.get_comment_amount()
		<< "\nBlank lines: " << reader.get_blank_amount()
		<< "\nTotal lines: " << reader.get_code_amount() + reader.get_comment_amount() + reader.get_blank_amount()
		<< "\nTime spent: " << reader.get_duration();
}