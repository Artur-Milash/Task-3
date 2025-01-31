#include "Code_reader.h"
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
	/*std::cout << "Size in destr: " << tasks->size() << std::endl;
	while (!tasks->empty())
		std::this_thread::sleep_for(std::chrono::milliseconds(10));*/

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

Code_reader::Code_reader() : Code_reader({}, { ".vs", ".git" }, "Code_reader_result.txt") {}
Code_reader::Code_reader(const std::vector<std::string>& check_path) : Code_reader{ check_path, {".git", ".vs"}, "Code_reader_result.txt" } {}
Code_reader::Code_reader(const std::vector<std::string>& check_path, const std::vector<std::string>& ignore_path)
	: Code_reader{ check_path, ignore_path, "Code_reader_result.txt" } {}

Code_reader::Code_reader(const std::vector<std::string>& check_path, const std::vector<std::string>& ignore_path, const std::string& save_location)
	: code{ 0 }, blank{ 0 }, comment{ 0 }, file_processed{ 0 }, to_check{ check_path }, ignore_vec{ ignore_path },
	pool{ Thread_pool::get_instance() }, start{ std::chrono::high_resolution_clock::now() },
	save_path{ save_location }, mutex{}
{
}

Code_reader::~Code_reader() {
	Thread_pool::destroy();

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	std::ofstream file{ save_path };
	file << "File processed: " << file_processed
		<< "\nCode lines: " << code
		<< "\nComment lines: " << comment
		<< "\nBlank lines: " << blank
		<< "\nTotal lines: " << code + blank + comment
		<< "\nTime spent: " << duration;
}

void Code_reader::ignore(const std::string& value) {
	ignore_vec.emplace_back(value);
}

void Code_reader::add_folder_to_check(const std::string& value) {
	to_check.emplace_back(value);
}

void Code_reader::check_directory(const std::string path) {
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		const std::string file_name = file.path().string().substr(path.length() + 1, std::string::npos);

		bool is_ignored = 0;
		for (const auto& ign :ignore_vec) {
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
		bool is_comment = 0, counted = 0;
		unsigned long long comment_count{ 0 }, code_count{ 0 }, blank_count{ 0 };

		while (std::getline(file, line)) {
			auto comment_index = line.find_first_of("/");
			if (comment_index != std::string::npos) {
				auto text_index = line.find_first_of('"');
				if (line[comment_index + 1] == '/' && comment_index < text_index) {
					comment_count++;
					counted = 1;
				}
				else if (line[comment_index + 1] == '*' && comment_index < text_index) {
					is_comment = 1;
					comment_count++;
					if (line.find_last_of("/") != comment_index) {
						comment_index = line.find_last_of("/");
						if (line[comment_index - 1] == '*')
							is_comment = 0;
					}
				}
				else if (line[comment_index - 1] == '*') {
					is_comment = 0;
					counted = 1;
					comment_count++;
				}
			}
			else if (is_comment) {
				comment_count++;
			}

			if (line.find_first_not_of("/*\t \n") != std::string::npos && !counted && !is_comment) {
				code_count++;
			}
			else if (!counted && !is_comment) {
				blank_count++;
			}
			counted = 0;
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
	for (std::size_t i = 0; i < to_check.size(); i++) {
		if (std::filesystem::is_directory(to_check.at(i))) {
			pool->add_task([this, i] {
				check_directory(to_check.at(i));
				});
		}
		else if (std::filesystem::is_regular_file(to_check.at(i))) {
			const std::string file_name = to_check.at(i).substr(to_check.at(i).find_last_of('/') + 1, std::string::npos);
			check_file(to_check.at(i), file_name);
		}
	}
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