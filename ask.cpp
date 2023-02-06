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

	void open_write() {
		auto status = ios::in | ios::out | ios::app;
		if (!append)
			status = ios::in | ios::out | ios::trunc;

		fout.open(database_name, status);
	}

	void open_read() {
		fin.open(database_name);
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

	void print_feed() {
		if (!is_answered()) return;

		if (parent_question_id != -1) cout << "Thread Parent Question ID (" << parent_question_id << ") ";
		cout << "Question ID (" << id << ") ";

		if (!anonymous) cout << "from user id(" << from_user_id << ") ";
		cout << "To user id(" << to_user_id << ")		";

		cout << "Question: " << question << "\n";
		cout << "	Answer: " << answer << "\n";
	}
};

struct User {
	int id;
	bool allow_AQ;
	string name;
	string email;
	string username;
	string password;

	vector<Question> questions;

	User() {}

	void print_questions_to_me() {
		for (auto question: questions) {
			if (question.to_user_id != id) continue;
			question.print_to();
		}
	}

	void print_questions_from_me() {
		for (auto question: questions) {
			if (question.from_user_id == id) question.print_from();
			
			for (auto thread_q: question.thread_questions)
				if (thread_q.from_user_id == id) thread_q.print_from();
		}

		cout << endl;
	}

	void answer_question() {
		cout << "Enter Question id or -1 to cancel: ";
		int q_id;
		cin >> q_id;

		if (q_id == -1) {
			cout << "Cancelled\n";
			return;
		}

		for (auto &question: questions) {
			if (question.to_user_id != id) continue;

			if (q_id != question.id) {
				for (auto &thread_question: question.thread_questions) {
					if (thread_question.id != q_id) continue;

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
			} else {
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
		}

		cout << "Question Not Found\n";
	}

	void delete_question() {
		cout << "Enter Question id or -1 to cancel: ";
		int q_id;
		cin >> q_id;

		if (q_id == -1) {
			cout << "Cancelled\n";
			return;
		}

		for (auto it = questions.begin(); it != questions.end(); it++) {
			if (it->to_user_id != id) continue;

			if (it->id == q_id) {
				questions.erase(it);
				return;
			}

			for (auto it2 = it->thread_questions.begin(); it2 != it->thread_questions.end(); it2++) {
				if (it2->id != q_id) continue;

				it->thread_questions.erase(it2);
				return;
			}
		}

		cout << "Question Not Found\n";
	}

	void feed() {
		for (auto question: questions) {
			if (question.from_user_id == id || question.to_user_id == id) question.print_feed();

			for (auto thread_q: question.thread_questions) {
				if (thread_q.from_user_id != id && thread_q.to_user_id != id) continue;
				thread_q.print_feed();
			}
		}
	}

	void ask_question(vector<User>& users, int questions_count) {
		int user_id;
		cout << "Enter user id or -1 to cancel: ";
		cin >> user_id;

		if (user_id == -1) return; // cancelled
		if (user_id < 0) {cout << "Invalid User ID\n"; return;}
		if (user_id == id) {cout << "Cannot Ask To Yourself\n"; return;}

		int user_idx = user_exist(users, user_id);
		if (user_idx == -1) {
			cout << "User Doesn't Exist Please Try Again\n";
			return;
		}

		char anonymous = 'n';
		if (!users[user_idx].allow_AQ) cout << "Note: Anonymous questions are not allowed for this user\n";
		else {
			while (true) {
				cout << "Do you want to send you question anonymously (y/n): ";
				cin >> anonymous;

				if (anonymous == 'y' || anonymous == 'n') break;
				cout << "Invalid Choice.\n";
			}
		}

		int q_id;
		cout << "For thread question: Enter Question id or -1 for new question: ";
		cin >> q_id;

		// Code Refactor Here
		if (q_id == -1) {
			Question q;
			q.id = questions_count;
			q.from_user_id = id;
			q.to_user_id = user_id;
			q.anonymous = anonymous == 'y' ? true : false;
			q.parent_question_id = -1;

			string question;
			cout << "Enter question text: ";
			cin.ignore();
			getline(cin, question);

			q.question = question;
			q.answer = "";

			questions.push_back(q);
		} else {
			int parent_question_idx = parent_question_exist(questions, user_id, q_id);
			if (parent_question_idx == -1) {cout << "Question was not found.\n"; return;}

			Question thread_q;

			string question;
			cout << "Enter question text: ";			
			cin.ignore();
			getline(cin, question);

			thread_q.id = questions_count;
			thread_q.from_user_id = id;
			thread_q.to_user_id = user_id;
			thread_q.anonymous = anonymous == 'y' ? true : false;
			thread_q.parent_question_id = questions[parent_question_idx].id;
			thread_q.question = question;
			thread_q.answer = "";

			questions[parent_question_idx]
			.thread_questions
			.push_back(thread_q);
		}
	}

	int user_exist(vector<User>& users, int user_id) {
		for (int i = 0; i < users.size(); i++)
			if (users[i].id == user_id) return i;

		return -1;
	}

	int parent_question_exist(vector<Question>& questions, int to_user_id, int q_id) {
		for (int i = 0; i < questions.size(); i++) {
			Question q = questions[i];
			if (q.to_user_id == to_user_id && q.id == q_id) return i;
		}
	
		return -1;
	}
};

struct QuestionLoader {
	Database questions_database = Database("questions.txt", false);
	vector<Question> questions;

	QuestionLoader(User& user) {
		load_questions(user);
	}

	void load_questions(User& user) {
		questions.clear();
		questions_database.open_read();
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

			if (current_q.parent_question_id == -1) {
				questions.push_back(current_q);
				continue;
			} else {
				questions[questions.size()-1].thread_questions.push_back(current_q);
				continue;
			}
		}

		user.questions = questions;
		questions_database.close();
	}

	void save_questions(User& user) {
		questions_database.open_write();

		// cout << "While Saving " << user.questions.size() << endl;
		for (auto q: user.questions) {
			save_question(q);
			for (auto thread_q: q.thread_questions)
				save_question(thread_q);
		}

		questions_database.close();
	}

	void save_question(Question& q) {
		char seperator = ',';

		fout << q.id << seperator;
		fout << q.from_user_id << seperator << q.to_user_id << seperator;
		fout << q.anonymous << seperator;
		fout << q.parent_question_id << seperator;
		fout << q.question << endl;
		fout << q.answer << endl;
	}

	int get_questions_count(User& user) {
		int q_count = get_num_questions(questions);
		return q_count;
	}

	int get_num_questions(vector<Question>& questions) {
		int count = 0;
		for (auto q: questions)
			count += 1 + q.thread_questions.size();

		return count;
	}
};

struct Registration {
	Database users_database = Database("users.txt", true);
	int current_user_idx;
	vector<User> users;

	Registration() {
		load_users();

		bool registration_success = false;
		while (!registration_success) {
			int choice = menu();
			if (choice == 1) registration_success = login();
			if (choice == 2) registration_success = signup();
		}
	}

	void load_users() {
		users_database.open_read();

		string line;
		int line_num = 0;
		while (getline(fin, line)) {
			User current_user;

			istringstream iss(line);
			iss >> current_user.id >> current_user.allow_AQ;

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
		string name, email, username, password;
		cout << "Enter name, email, username & password: ";
		cin >> name >> email >> username >> password;

		int current_user_idx = user_exist(username, password);
		if (current_user_idx != -1) {
			cout << "User Already Registered\n\n";
			return false;
		}

		char allow_AQ;
		while (true) {
			cout << "Do You Allow Anonymous Questions (y/n): ";
			cin >> allow_AQ;
	
			if (allow_AQ == 'y' || allow_AQ == 'n') break;
			cout << "Invalid Choice.\n";
		}

		store_user(name, email, username, password, allow_AQ == 'y' ? true : false);
		cout << "Signup Completed Successfully\n";
		return true;
	}

	int user_exist(string username, string password) {
		if (username == "" || password == "") return -1;
		for (int i = 0; i < users.size(); i++)
			if (users[i].username == username && users[i].password == password) return i;

		return -1;
	}

	void store_user(string name, string email, string username, string password, bool allow_AQ) {
		users_database.open_write();
		
		fout << users.size() << " " << allow_AQ << "\n";
		fout << name << "\n";
		fout << email << "\n";
		fout << username << "\n";
		fout << password << "\n";
	
		User new_user;
		current_user_idx = users.size();
		new_user.id = users.size();
		new_user.allow_AQ = allow_AQ;
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
	Registration reg = Registration();
	User current_user = reg.get_user();
	QuestionLoader question_loader = QuestionLoader(current_user);

	Ask() {	
		cout << "Hello " << current_user.name << endl;
		
		while (true) {
			int choice = menu();
			cout << "\n";

			if (choice == 1) update_sys_read(), current_user.print_questions_to_me();
			if (choice == 2) update_sys_read(), current_user.print_questions_from_me();

			if (choice == 3) current_user.answer_question(), update_sys_write();
			if (choice == 4) current_user.delete_question(), update_sys_write();
			if (choice == 5) {
				update_sys_read();
				current_user.ask_question(reg.users, question_loader.get_questions_count(current_user));
				update_sys_write();
			}
			if (choice == 6) reg.list_users();
			if (choice == 7) current_user.feed();
			if (choice == 8) break;
		}
	}

	void update_sys_write() {
		question_loader.save_questions(current_user);
	}

	void update_sys_read() {
		question_loader.load_questions(current_user);
	}

	int menu() {
		cout << "\nMenu:\n";
		cout << "	1: Print Questions To Me\n";
		cout << "	2: Print Questions From Me\n";
		cout << "	3: Answer Question\n";
		cout << "	4: Delete Question\n"; // We didn't test this feature yet
		cout << "	5: Ask Question\n";
		cout << "	6: List System Users\n";
		cout << "	7: Feed\n"; // This is currently not working
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