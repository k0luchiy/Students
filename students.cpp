#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>

std::mutex mtx;


struct Student {
	int id;
	std::string name;
	int age;

	Student() = default;
	Student(int id, const std::string& name, int age) : id(id), name(name), age(age) {}
	~Student() = default;
};

struct StudentsDB {
	using students_shared = std::shared_ptr<std::vector<Student>>;
	using iterator = std::vector<Student>::iterator;

	students_shared students;

	StudentsDB() {
		students = std::make_shared <std::vector<Student>>();
	};

	explicit StudentsDB(students_shared students) : students(students) {}

	void push_back(const Student& student) {
		students->push_back(student);
	}

	template <typename... Args>
	void emplace_back(Args... args) {
		students->emplace_back(std::forward<Args>(args)...);
	}

	std::vector<Student>::iterator findById(int studentId) {
		auto it = std::find_if(students->begin(), students->end(),
			[studentId](const Student& student) { return student.id == studentId; });
		return it;
	}

	bool remove(int studentId) {
		auto it = findById(studentId);
		if (it == students->end()) {
			return false;
		}

		students->erase(it);
		return true;
	}

	Student find(int studentId) {
		auto it = findById(studentId);
		if (it == students->end()) {
			return Student();
		}
		return *it;
	}

	iterator begin() {
		return students->begin();
	}

	iterator end() {
		return students->end();
	}
};


void addStudents(StudentsDB& students) {
	std::lock_guard<std::mutex> lock(mtx);
	for (size_t i = 0; i < 100; ++i) {
		students.emplace_back(i, "", i);
	}
}

void readStudents(StudentsDB& students) {
	std::lock_guard<std::mutex> lock(mtx);
	for (auto student : students) {
		std::cout << student.id << " " << student.name << " " << student.age << "\n";
	}
}



int main()
{
	std::vector<Student> stud_vec{
		{ 1, "anton", 19 },
		{ 2, "vasya", 31 },
		{ 3, "maria", 20 }
	};
	StudentsDB studentsDB(std::make_shared<std::vector<Student>>(stud_vec));

	for (auto student : studentsDB) {
		std::cout << student.id << " " << student.name << " " << student.age << "\n";
	}

	studentsDB.remove(2);

	for (auto student : studentsDB) {
		std::cout << student.id << " " << student.name << " " << student.age << "\n";
	}

	Student student = studentsDB.find(3);
	std::cout << student.id << " " << student.name << " " << student.age << "\n";


	std::thread t1(addStudents, std::ref(studentsDB));
	std::thread t2(readStudents, std::ref(studentsDB));
	
	t1.join();
	t2.join();

}
