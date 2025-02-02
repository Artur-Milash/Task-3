#include "For_test1.h"
#include <iostream>

//Thread_pool

class Thread_pool {
private:
	static Thread_pool* singleton;

	static std::vector<std::thread>* threads;
	static std::deque<std::function<void()>>* tasks;
	static std::mutex* mutex;
	static std::condition_variable* var;
	static bool* end;

	Thread_pool(const unsigned int&);
	~Thread_pool() = default;
public:
	void operator=(const Thread_pool&) = delete;
	Thread_pool(const Thread_pool&) = delete;
	Thread_pool(Thread_pool&&) = delete;

	static Thread_pool* get_instance(const unsigned int&);
	static Thread_pool* get_instance();

	static void destroy();

	void add_task(std::function<void()>);

	std::size_t get_threads_amount() const;
};

Thread_pool* Thread_pool::singleton = nullptr;

std::vector<std::thread>* Thread_pool::threads = nullptr;
std::deque<std::function<void()>>* Thread_pool::tasks = nullptr;
std::mutex* Thread_pool::mutex = nullptr;
std::condition_variable* Thread_pool::var = nullptr;
bool* Thread_pool::end = nullptr;

Thread_pool::Thread_pool(const unsigned int& amount) {
	threads = new std::vector<std::thread>{};
	tasks = new std::deque<std::function<void()>>;
	mutex = new std::mutex;
	var = new std::condition_variable;
	end = new bool{ false };

	if (amount == 0)
		throw std::invalid_argument("Threads amount can't be 0!");

	for (unsigned int i = 0; i < amount; i++) {
		threads->emplace_back([this] {
			do {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(*mutex);

					var->wait(lock, [this] {return *end || !tasks->empty(); });

					if (*end && tasks->empty())
						return;

					task = std::move(tasks->front());
					tasks->pop_front();
				}
				task();
			} while (true);
			});
	}
}

Thread_pool* Thread_pool::get_instance(const unsigned int& amount) {
	if (singleton == nullptr) {
		singleton = new Thread_pool{ amount };
	}
	return singleton;
}
Thread_pool* Thread_pool::get_instance() {
	return get_instance(std::thread::hardware_concurrency());
}

void Thread_pool::destroy() {
	{
		std::unique_lock<std::mutex> lock{ *mutex };
		*end = 1;
	}
	var->notify_all();

	for (auto& obj : *threads) {
		obj.join();
	}

	delete threads;
	delete tasks;
	delete var;
	delete end;
	delete mutex;
	delete singleton;

	singleton = nullptr;
}

void Thread_pool::add_task(std::function<void()> task) {
	{
		std::unique_lock<std::mutex> lock{ *mutex };
		tasks->emplace_front(std::move(task));
	}
	var->notify_one();
}

std::size_t Thread_pool::get_threads_amount() const {
	return threads->size();
}


//Code_reader

Code_reader::Code_reader()
	: Code_reader({}, { ".vs", ".git" }, "Code_reader_result.txt", CODE_READER_CODE_PRIORITY) {}
Code_reader::Code_reader(const std::set<std::string>& check_path)
	: Code_reader{ check_path, {".git", ".vs"}, "Code_reader_result.txt", CODE_READER_CODE_PRIORITY } {}
Code_reader::Code_reader(const std::set<std::string>& check_path, const std::set<std::string>& ignore_path)
	: Code_reader{ check_path, ignore_path, "Code_reader_result.txt", CODE_READER_CODE_PRIORITY } {}
Code_reader::Code_reader(const std::set<std::string>& check_path, const std::set<std::string>& ignore_path, const std::string& save_location)
	: Code_reader{ check_path, ignore_path, save_location, CODE_READER_CODE_PRIORITY } {}

Code_reader::Code_reader(const std::set<std::string>& check_path, const std::set<std::string>& ignore_path,
	const std::string& save_location, const Code_reader_priority& value)
	: code{ 0 }, blank{ 0 }, comment{ 0 }, file_processed{ 0 }, to_check{ check_path }, to_ignore{ ignore_path },
	pool{ Thread_pool::get_instance() }, start{ std::chrono::high_resolution_clock::now() },
	save_path{ save_location }, mutex{}, time_spent{ 0 }, priority{ (unsigned short)value } {}

Code_reader::~Code_reader() {
	Thread_pool::destroy();

	std::ofstream file{ save_path };
	file << "File processed: " << file_processed
		<< "\nCode lines: " << code
		<< "\nComment lines: " << comment
		<< "\nBlank lines: " << blank
		<< "\nTotal lines: " << code + blank + comment
		<< "\nTime spent: " << time_spent;
}

void Code_reader::ignore(const std::string& value) {
	to_ignore.emplace(value);
}

void Code_reader::add_folder_to_check(const std::string& value) {
	to_check.emplace(value);
}

void Code_reader::check_directory(const std::string path) {
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		const std::string file_name = file.path().string().substr(path.length() + 1, std::string::npos);

		bool is_ignored = 0;
		for (const auto& ign : to_ignore) {
			if (file.path() == ign || file_name == ign) {
				is_ignored = 1;
				break;
			}
		}

		if (!is_ignored) {
			if (std::filesystem::is_directory(file.path())) {
				pool->add_task([this, file] {
					check_directory(file.path().string());
					});
			}
			else if (std::filesystem::is_regular_file(file.path())) {
				pool->add_task([this, file, file_name] {
					check_file(file.path().string(), file_name);
					});

			}
		}

	}
}
void Code_reader::check_file(const std::string path, const std::string file_name) {
	const std::string file_extension = file_name.substr(file_name.find_first_of('.') + 1, std::string::npos);
	if (file_extension == "cpp" || file_extension == "h" || file_extension == "c" || file_extension == "hpp") {
		std::ifstream file{ path };

		std::string line;
		bool is_comment = 0, is_counted = 0;
		unsigned long long comment_count{ 0 }, code_count{ 0 }, blank_count{ 0 };

		while (std::getline(file, line)) {

			if (line.find("//") != std::string::npos && !is_comment) {
				std::size_t position = line.find("//"), first_text = line.find_first_of('"'),
					last_text = line.find_last_of('"');
				bool is_text = 0;

				if (last_text != std::string::npos && last_text != first_text) {
					if (position < last_text)
						is_text = 1;
				}

				if (line.substr(0, position).find_first_not_of("/*\t ") != std::string::npos && !is_text) {
					switch (priority) {
					case CODE_READER_CODE_PRIORITY:
						break;
					case CODE_READER_COMMENT_PRIORITY:
						comment_count++;
						is_counted = 1;
						break;
					case CODE_READER_MIXED_PRIORITY:
						comment_count++;
						break;
					}
				}
				else if (!is_text) {
					comment_count++;
					is_counted = 1;
				}
			}
			else if (line.find("/*") != std::string::npos && !is_comment) {
				std::size_t position = line.find("/*"), first_text = line.find_first_of('"'),
					last_text = line.find_last_of('"');
				bool is_text = 0;

				if (last_text != std::string::npos && last_text != first_text) {
					if (position < last_text)
						is_text = 1;
				}

				if (line.substr(0, position).find_first_not_of("/*\t ") != std::string::npos && !is_text) {
					switch (priority) {
					case CODE_READER_CODE_PRIORITY:
						code_count++;
						is_comment = 1;
						break;
					case CODE_READER_COMMENT_PRIORITY:
						comment_count++;
						is_counted = 1;
						is_comment = 1;
						break;
					case CODE_READER_MIXED_PRIORITY:
						code_count++;
						comment_count++;
						is_counted = 1;
						is_comment = 1;
						break;
					}
				}
				else if (!is_text) {
					comment_count++;
					is_counted = 1;
					is_comment = 1;
				}

				if (is_comment && line.find("*/") != std::string::npos && !is_text) {
					std::size_t last_pos = line.find("*/");
					if (line[last_pos - 1] != '/') {
						is_comment = 0;
					}
				}
			}
			else if (is_comment) {
				comment_count++;
				if (line.find("*/") != std::string::npos)
					is_comment = 0;
				is_counted = 1;
			}

			if (line.find_first_not_of("/*\t \n") != std::string::npos && !is_comment && !is_counted) {
				code_count++;
			}
			else if (!is_comment && !is_counted) {
				blank_count++;
			}
			is_counted = 0;
		}

		file.close();
		{
			std::lock_guard<std::mutex> lock{ mutex };

			file_processed++;
			code += code_count;
			comment += comment_count;
			blank += blank_count;
		}
	}
}

void Code_reader::check() {
	for (auto& element : to_check) {
		if (std::filesystem::is_directory(element)) {
			pool->add_task([this, &element] {
				check_directory(element);
				});
		}
		else if (std::filesystem::is_regular_file(element)) {
			const std::string file_name = element.substr(element.find_last_of('/') + 1, std::string::npos);
			check_file(element, file_name);
		}
	}

	Thread_pool::destroy();
	pool = Thread_pool::get_instance();

	auto end = std::chrono::high_resolution_clock::now();
	time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

void Code_reader::set_priority(const Code_reader_priority& value) {
	priority = value;
}
unsigned short Code_reader::get_priority() const {
	return priority;
}

void Code_reader::set_save_location(const std::string& path) {
	save_path = path;
}

unsigned long long Code_reader::get_code_amount() const {
	return code;
}
unsigned long long Code_reader::get_blank_amount() const {
	return blank;
}
unsigned long long Code_reader::get_comment_amount() const {
	return comment;
}
unsigned int Code_reader::get_file_processed() const {
	return file_processed;
}
std::chrono::milliseconds Code_reader::get_duration() const {
	return time_spent;
}