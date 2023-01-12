#include <bits/stdc++.h>
using namespace std;
ifstream fin;
ofstream fout;

struct Database {
	string database_name;
	bool append;

	Database(string _database_name, bool _append) {
		database_name = _database_name;
		append = _append;
	}

	void close() {
		fin.close();
		fout.close();
	}

	void open() {
		auto status = ios::in | ios::out | ios::app;
		if (!append)
			status = ios::in | ios::out | ios::trunc;

		fin.open(database_name);
		fout.open(database_name, status);
	}
};

struct Question {
	int id;
	int from_user_id;
	int to_user_id;
	bool anonymous;
	int parent_question_id;	
	string question;
	string answer;
	bool null_question;

	vector<Question> thread_questions;

	void add_thread(Question thread) {
		thread_questions.push_back(thread);
	}

	void print_to() {
		print_to_structure();
		if (thread_questions.size() == 0) return;
	
		for (auto thread: thread_questions)
			thread.print_to_thread_structure();
	}

	void print_to_structure() {
		cout << "Question Id (" << id << ") ";
		if (!anonymous) cout << "from user id(" << from_user_id << ")";

		cout << "	";
		cout << "	Question: " << question << "\n";

		if (!is_answered()) return;
		cout << "	Answer: " << answer << "\n\n";
	}

	void print_to_thread_structure() {
		cout << "	Thread: ";
		cout << "Question Id (" << id << ") ";
		if (!anonymous) cout << "from user id(" << from_user_id << ")";

		cout << "	";
		cout << "Question: " << question << "\n";
		
		if (!is_answered()) {cout << "\n"; return;}
		cout << "	Thread 		Answer: " << answer << "\n\n";
	}

	void print_from() {
		cout << "Question Id (" << id << ") ";
		if (!anonymous) cout << "!AQ ";

		cout << "to user id (" << to_user_id << ")";
		cout << "		Question: " << question;

		if (answer.size() == 0) cout << "	NOT Answered YET";
		else cout << "	Answer: " << answer;

		cout << "\n";
	}

	bool is_answered() {
		if (answer.size() == 0) return false;
		return true;
	}
};

struct User {
	int id;
	string name;
	string email;
	string username;
	string password;

	vector<Question> questions_to;
	vector<Question> questions_from;

	User() {}

	void print_questions_to_me() {
		for (auto question: questions_to)
			question.print_to();
	}

	void print_questions_from_me() {
		for (auto question: questions_from)
			question.print_from();

		cout << endl;
	}

	void answer_question() {
		cout << "Enter Question id or -1 to cancel: ";
		int id;
		cin >> id;

		if (id == -1) {
			cout << "Cancelled\n";
			return;
		}

		for (auto &question: questions_to) {
			if (id != question.id) {
				for (auto &thread_question: question.thread_questions) {
					if (thread_question.id != id) continue;

					thread_question.print_to_thread_structure();
					cout << "\n";
					if (thread_question.is_answered()) cout << "Warning: Already answered. Answer will be updated\n";

					cout << "Enter Answer: ";
					string answer;
					cin.ignore();
					getline(cin, answer);

					thread_question.answer = answer;
					return;
				}
			}

			question.print_to_structure();
			cout << "\n";
			if (question.is_answered()) cout << "Warning: Already answered. Answer will be updated\n";

			cout << "Enter Answer: ";
			string answer;
			cin.ignore();
			getline(cin, answer);

			question.answer = answer;
			return;
		}

		cout << "Question Not Found\n";
	}

	void delete_question() {
		cout << "Enter Question id or -1 to cancel: ";
		int id;
		cin >> id;

		if (id == -1) {
			cout << "Cancelled\n";
			return;
		}

		for (auto it = questions_to.begin(); it != questions_to.end(); it++) {
			if (it->id != id) {
				for (auto it2 = it->thread_questions.begin(); it2 != it->thread_questions.end(); it2++) {
					if (it2->id != id) continue;

					it->thread_questions.erase(it2);
					return;
				}
			}

			questions_to.erase(it);
			return;
		}

		cout << "Question Not Found\n";
	}
};


struct QuestionLoader {
	Database questions_database = Database("questions.txt", true);
	vector<Question> questions_to;
	vector<Question> questions_from;

	QuestionLoader(User& user) {
		load_questions(user);
		user.questions_to = questions_to;
		user.questions_from = questions_from;
	}

	void load_questions(User& user) {
		questions_database.open();
		string line;

		while (getline(fin, line)) {
			istringstream iss(line);
			Question current_q;
			
			string question;
			string answer;

			char seperator;
			iss >> current_q.id >> seperator;
			iss >> current_q.from_user_id >> seperator >> current_q.to_user_id >> seperator;
			iss >> current_q.anonymous >> seperator;
			iss >> current_q.parent_question_id >> seperator;
			getline(iss, current_q.question);
			getline(fin, current_q.answer);			

			if (current_q.from_user_id != user.id && current_q.to_user_id != user.id) continue;

			if (current_q.parent_question_id != -1) {
				if (current_q.from_user_id == user.id) questions_from[current_q.parent_question_id].add_thread(current_q);
				if (current_q.to_user_id == user.id) questions_to[current_q.parent_question_id].add_thread(current_q);
				continue;
			}

			if (current_q.from_user_id == user.id) questions_from.push_back(current_q);
			if (current_q.to_user_id == user.id) questions_to.push_back(current_q);
		}
	}
};

struct Registration {
	Database users_database = Database("users.txt", true);
	int current_user_idx;
	vector<User> users;

	Registration() {
		load_users();
		for (auto user: users)
			cout << user.username << endl;

		bool registration_success = false;
		while (!registration_success) {
			int choice = menu();
			if (choice == 1) registration_success = login();
			if (choice == 2) registration_success = signup();
		}
	}

	void load_users() {
		users_database.open();

		string line;
		int line_num = 0;
		while (getline(fin, line)) {
			User current_user;

			istringstream iss(line);
			iss >> current_user.id;

			getline(fin, current_user.name);
			getline(fin, current_user.email);
			getline(fin, current_user.username);
			getline(fin, current_user.password);

			users.push_back(current_user);			
		}

		users_database.close();
	}

	bool login() {
		cout << "Enter user name & password: ";
		string username, password;
		cin >> username >> password;

		current_user_idx = user_exist(username, password); 
		if (current_user_idx == -1) {
			cout << "Invalid Login Credentials\n\n"; 
			return false;
		}

		cout << "Logged In Successfully\n";
		return true;
	}

	bool signup() {
		cout << "Enter name, email, username & password: ";
		string name, email, username, password;
		cin >> name >> email >> username >> password;

		int current_user_idx = user_exist(username, password);
		if (current_user_idx != -1) {
			cout << "User Already Registered\n\n";
			return false;
		}

		store_user(name, email, username, password);
		cout << "Signup Completed Successfully\n";
		return true;
	}

	int user_exist(string username, string password) {
		if (username == "" || password == "") return -1;
		for (int i = 0; i < users.size(); i++)
			if (users[i].username == username && users[i].password == password) return i;

		return -1;
	}
	void store_user(string name, string email, string username, string password) {
		users_database.open();
		
		fout << users.size() << "\n";
		fout << name << "\n";
		fout << email << "\n";
		fout << username << "\n";
		fout << password << "\n";
	
		User new_user;
		new_user.id = users.size();
		new_user.name = name;
		new_user.email = email;
		new_user.username = username;
		new_user.password = password;

		users.push_back(new_user);
	
		users_database.close();
	}

	int menu() {
		cout << "Menu:\n";
		cout << "	1: Login\n";
		cout << "	2: Sign Up\n\n";
		
		int choice = -1;
		while (choice == -1) {
			cout << "Enter number in range 1 - 2: ";
			cin >> choice;
	
			if (choice < 1 || choice > 2) {
				cout << "ERROR: invalid number...Try again\n\n";
				choice = -1;
			}
		}

		return choice;
	}

	User get_user() {
		return users[current_user_idx];
	}

	void list_users() {
		for (auto user: users) {
			cout << "ID: " << user.id << "		";
			cout << "Name: " << user.name << "\n";
		}
	}
};

struct Ask {
	Ask() {
		Registration reg = Registration();
		User current_user = reg.get_user();
		QuestionLoader question_loader = QuestionLoader(current_user);
		
		while (true) {
			int choice = menu();
			cout << "\n";

			if (choice == 1) current_user.print_questions_to_me();
			if (choice == 2) current_user.print_questions_from_me();
			if (choice == 3) current_user.answer_question();
			if (choice == 4) current_user.delete_question();
			if (choice == 6) reg.list_users(); 
			if (choice == 8) break;
		}
	}

	int menu() {
		cout << "\nMenu:\n";
		cout << "	1: Print Questions To Me\n";
		cout << "	2: Print Questions From Me\n";
		cout << "	3: Answer Question\n";
		cout << "	4: Delete Question\n";
		cout << "	5: Ask Question\n";
		cout << "	6: List System Users\n";
		cout << "	7: Feed\n";
		cout << "	8: Logout\n\n";

		int choice = -1;
		while (choice == -1) {
			cout << "Enter number in range 1 - 8: ";
			cin >> choice;
	
			if (choice < 1 || choice > 8) {
				cout << "ERROR: invalid number...Try again\n\n";
				choice = -1;
			}
		}

		return choice;
	}
};


int main() {
	cout << boolalpha;

	Ask ask = Ask();

	return 0;
}