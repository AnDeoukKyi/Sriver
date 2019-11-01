#include <iostream>

using namespace std;

class User {
public:
	char *id;
	int posX = -1;
	int posY = -1;

	User(char *_id) {
		id = new char[strlen(_id) + 1];
		strcpy(id, _id);
	}
	User() {
		id = "NULL";
	}
	void Show() {
		cout << id << endl;
		cout << posX << endl;
		cout << posY << endl;
	}
	void Send() {

	}
	void setX(int x) {
		posX = x;
	}
	void setY(int y) {
		posY = y;
	}
	~User() {
		delete[]id;
	}
};

class UList {
public:
	int index = 10;
	User *uList[10];

	UList() {//디폴트값으로 NULL인 유저가 10명 들어감
		for (int i = 0; i < 10; i++) {
			AddUser(new User(), i);
		}
	};

	void AddUser(User *u, int i) {
		uList[i] = u;
	}

	void SendPos() {
		for (int i = 0; i < 10; i++)
			uList[i]->Send();
	}

	void ShowAll() {
		for (int i = 0; i < 10; i++) {
			uList[i]->Show();
		}
	}

	void DeleteUser(int i) {
		uList[i]->id = "NULL";
		uList[i]->setX(-1);
		uList[i]->setY(-1);
	}
	int FindNull() {
		for (int i = 0; i < 10; i++) {
			if (strcmp(uList[i]->id, "NULL") == 0)
				return i;//NULL이면
		}
		return -1;
	}

	int FindNull(int n) {
		if (strcmp(uList[n]->id, "NULL") == 0)
			return 1;//NULL이면
		else return -1;//User있으면
	}

	int CountNull() {
		int cnt = 0;
		for (int i = 0; i < 10; i++) {
			if (strcmp(uList[i]->id, "NULL") == 0) {
				cnt++;
			}
		}
		return cnt;
	}
	User *First() {
		for (int i = 0; i < 10; i++) {
			if (strcmp(uList[i]->id, "NULL") != 0) {
				return uList[i];
			}
		}
		return NULL;
	}

	int CountInUser() {
		for (int i = 0; i < 10; i++) {
			if (strcmp(uList[i]->id, "NULL") != 0) {
				return i;
			}
		}
	}

	int CountUser() {
		int cnt = 0;
		for (int i = 0; i < 10; i++) {
			if (strcmp(uList[i]->id, "NULL")) {
				cnt++;
			}
		}
		return cnt;
	}
};

UList uList;



int main() {
	uList.ShowAll();
	cout << "------------------------------------" << endl;
	cout << uList.CountNull() << endl;
	uList.AddUser(new User("안녕"), uList.FindNull());
	uList.AddUser(new User("하세요"), uList.FindNull());
	cout << "------------------------------------" << endl;
	uList.First()->Show();
	cout << "------------------------------------" << endl;
	uList.DeleteUser(1);
	uList.ShowAll();

	return 0;
}