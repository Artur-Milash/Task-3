#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <deque>
#include <functional>
#include <vector>
#include <algorithm>
#include <string>
#include <filesystem>
#include <chrono> 


class Thread_pool;

class Code_reader {
private:
	using time_type = std::chrono::steady_clock::time_point;

	Thread_pool* pool;
	std::mutex mutex;

	unsigned long long code;
	unsigned long long blank;
	unsigned long long comment;

	unsigned int file_processed;

	std::vector<std::string> ignore_vec;
	std::vector<std::string> to_check;

	std::string save_path;
	time_type start;

	void check_directory(const std::string);
	void check_file(const std::string, const std::string);
public:
	Code_reader();
	Code_reader(const std::vector<std::string>& check_path);
	Code_reader(const std::vector<std::string>& check_path, const std::vector<std::string>& ignore_path);
	Code_reader(const std::vector<std::string>& check_path, const std::vector<std::string>& ignore_path, const std::string& save_location);

	~Code_reader();

	void ignore(const std::string&);
	
	void add_folder_to_check(const std::string&);

	void check();

	void set_save_location(const std::string&);

	unsigned long long get_code_amount() const;
	unsigned long long get_blank_amount() const;
	unsigned long long get_comment_amount() const;
	unsigned int get_file_processed() const;
};