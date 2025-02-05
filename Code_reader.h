#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <deque>
#include <functional>
#include <algorithm>
#include <string>
#include <filesystem>
#include <chrono> 
#include <set>

//Simple library to count code, comment and blank lines in cpp, h, hpp, c files

const enum Code_reader_priority {
	CODE_READER_CODE_PRIORITY,
	CODE_READER_COMMENT_PRIORITY,
	CODE_READER_MIXED_PRIORITY
};

class Thread_pool;

class Code_reader {
private:
	Thread_pool* pool;
	std::mutex mutex;

	unsigned long long code;
	unsigned long long blank;
	unsigned long long comment;

	unsigned int file_processed;

	std::set<std::string> to_ignore;
	std::set<std::string> to_check;

	std::string save_path;

	std::chrono::steady_clock::time_point start;
	std::chrono::milliseconds time_spent;

	unsigned short priority;

	//main logic methods to check directories or files
	void check_directory(const std::string);
	void check_file(const std::string, const std::string);
public:
	Code_reader();
	Code_reader(const std::set<std::string>& check_path);
	Code_reader(const std::set<std::string>& check_path, const std::set<std::string>& ignore_path);
	Code_reader(const std::set<std::string>& check_path, const std::set<std::string>& ignore_path, const std::string& save_location);
	Code_reader(const std::set<std::string>& check_path, const std::set<std::string>& ignore_path,
		const std::string& save_location, const Code_reader_priority& value);

	~Code_reader();

	//add path to ignore
	void ignore(const std::string&);

	//remove paths from ignore set
	void remove_ignore(const std::string&);
	void remove_all_ignore();
	
	//add path to check
	void add_to_check(const std::string&);

	//remove paths from check set
	void remove_check(const std::string&);
	void remove_all_check();

	//main logic public method (calls check_directory and/or check_file)
	void check();

	//method to set path for final output file
	void set_save_location(const std::string&);

	//methods to set and get priority
	void set_priority(const Code_reader_priority&);
	unsigned short get_priority() const;

	//some getters for different variables
	unsigned long long get_code_amount() const;
	unsigned long long get_blank_amount() const;
	unsigned long long get_comment_amount() const;
	unsigned int get_file_processed() const;

	unsigned long long get_total() const;
	std::chrono::milliseconds get_duration() const;
};