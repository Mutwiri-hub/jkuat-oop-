// JKUAT Student Information System (console) - C++17
// Compile: g++ -std=c++17 -o jkuat_is jkuat_is.cpp
// Run:     ./jkuat_is   (data saved in jkuat_data.txt)

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <limits>
using namespace std;

struct Mark { string unit; int score; };

struct Student {
    string reg, name, course;
    int year = 1;
    double billed = 0, paid = 0;   // KES
    vector<Mark> marks;
};

const string DB = "jkuat_data.txt";
vector<Student> students;

// ---------- helpers ----------
string clean(string s) { replace(s.begin(), s.end(), '|', '/'); return s; }

string readLine(const string& prompt) {
    string s;
    cout << prompt;
    getline(cin, s);
    return clean(s);
}

double readNum(const string& prompt, double lo, double hi) {
    while (true) {
        cout << prompt;
        string s; getline(cin, s);
        try {
            double v = stod(s);
            if (v >= lo && v <= hi) return v;
        } catch (...) {}
        cout << "  Invalid. Enter a number between " << lo << " and " << hi << ".\n";
    }
}

string grade(int s) {
    if (s >= 70) return "A";
    if (s >= 60) return "B";
    if (s >= 50) return "C";
    if (s >= 40) return "D";
    return "E";
}

Student* find(const string& reg) {
    for (auto& s : students)
        if (s.reg == reg) return &s;
    return nullptr;
}

// ---------- persistence ----------
void save() {
    ofstream f(DB);
    for (auto& s : students) {
        f << "S|" << s.reg << '|' << s.name << '|' << s.course << '|'
          << s.year << '|' << s.billed << '|' << s.paid << '\n';
        for (auto& m : s.marks)
            f << "M|" << s.reg << '|' << m.unit << '|' << m.score << '\n';
    }
}

vector<string> split(const string& line) {
    vector<string> out; string tok; stringstream ss(line);
    while (getline(ss, tok, '|')) out.push_back(tok);
    return out;
}

void load() {
    ifstream f(DB);
    string line;
    while (getline(f, line)) {
        auto p = split(line);
        try {
            if (p[0] == "S" && p.size() >= 7) {
                Student s;
                s.reg = p[1]; s.name = p[2]; s.course = p[3];
                s.year = stoi(p[4]); s.billed = stod(p[5]); s.paid = stod(p[6]);
                students.push_back(s);
            } else if (p[0] == "M" && p.size() >= 4) {
                if (Student* s = find(p[1])) s->marks.push_back({p[2], stoi(p[3])});
            }
        } catch (...) { /* skip bad line */ }
    }
}

// ---------- features ----------
void addStudent() {
    Student s;
    s.reg = readLine("Reg No (e.g. CS282-0001/2024): ");
    if (s.reg.empty() || find(s.reg)) { cout << "Empty or duplicate Reg No.\n"; return; }
    s.name   = readLine("Full name: ");
    s.course = readLine("Course (e.g. Diploma in IT): ");
    s.year   = (int)readNum("Year of study (1-6): ", 1, 6);
    s.billed = readNum("Fees billed (KES): ", 0, 1e7);
    students.push_back(s);
    save();
    cout << "Student added.\n";
}

void listStudents() {
    if (students.empty()) { cout << "No students yet.\n"; return; }
    cout << left << setw(20) << "REG NO" << setw(25) << "NAME"
         << setw(8) << "YEAR" << "FEE BALANCE (KES)\n" << string(70, '-') << '\n';
    for (auto& s : students)
        cout << left << setw(20) << s.reg << setw(25) << s.name.substr(0, 24)
             << setw(8) << s.year << fixed << setprecision(2) << s.billed - s.paid << '\n';
}

void searchStudent() {
    string q = readLine("Search by Reg No or name: ");
    transform(q.begin(), q.end(), q.begin(), ::tolower);
    bool found = false;
    for (auto& s : students) {
        string a = s.reg + " " + s.name;
        transform(a.begin(), a.end(), a.begin(), ::tolower);
        if (a.find(q) != string::npos) {
            cout << s.reg << " | " << s.name << " | " << s.course
                 << " | Year " << s.year << '\n';
            found = true;
        }
    }
    if (!found) cout << "No match.\n";
}

void addMark() {
    Student* s = find(readLine("Reg No: "));
    if (!s) { cout << "Student not found.\n"; return; }
    string unit = readLine("Unit code (e.g. DIT 0302): ");
    int score = (int)readNum("Score (0-100): ", 0, 100);
    for (auto& m : s->marks)
        if (m.unit == unit) { m.score = score; save(); cout << "Mark updated.\n"; return; }
    s->marks.push_back({unit, score});
    save();
    cout << "Mark recorded.\n";
}

void transcript() {
    Student* s = find(readLine("Reg No: "));
    if (!s) { cout << "Student not found.\n"; return; }
    cout << "\n=== JKUAT ACADEMIC RECORD ===\n"
         << s->name << " | " << s->reg << "\n" << s->course << " | Year " << s->year << "\n\n";
    if (s->marks.empty()) { cout << "No marks recorded.\n"; return; }
    double total = 0;
    cout << left << setw(15) << "UNIT" << setw(8) << "SCORE" << "GRADE\n" << string(30, '-') << '\n';
    for (auto& m : s->marks) {
        cout << left << setw(15) << m.unit << setw(8) << m.score << grade(m.score) << '\n';
        total += m.score;
    }
    double avg = total / s->marks.size();
    cout << string(30, '-') << "\nMean: " << fixed << setprecision(1) << avg
         << " (" << grade((int)avg) << ")\n";
}

void payFees() {
    Student* s = find(readLine("Reg No: "));
    if (!s) { cout << "Student not found.\n"; return; }
    cout << "Balance: KES " << fixed << setprecision(2) << s->billed - s->paid << '\n';
    double amt = readNum("Amount paid (KES, e.g. via M-Pesa): ", 0, 1e7);
    string ref = readLine("Payment reference (M-Pesa code): ");
    s->paid += amt;
    save();
    cout << "Recorded " << ref << ". New balance: KES " << s->billed - s->paid << '\n';
}

void deleteStudent() {
    string reg = readLine("Reg No to delete: ");
    auto it = remove_if(students.begin(), students.end(),
                        [&](const Student& s) { return s.reg == reg; });
    if (it == students.end()) { cout << "Student not found.\n"; return; }
    students.erase(it, students.end());
    save();
    cout << "Deleted.\n";
}

int main() {
    load();
    while (true) {
        cout << "\n===== JKUAT INFORMATION SYSTEM =====\n"
             << "1. Add student\n2. List students\n3. Search student\n"
             << "4. Record marks\n5. View transcript\n6. Record fee payment\n"
             << "7. Delete student\n0. Exit\n";
        int c = (int)readNum("Choice: ", 0, 7);
        switch (c) {
            case 1: addStudent(); break;
            case 2: listStudents(); break;
            case 3: searchStudent(); break;
            case 4: addMark(); break;
            case 5: transcript(); break;
            case 6: payFees(); break;
            case 7: deleteStudent(); break;
            case 0: save(); cout << "Goodbye.\n"; return 0;
        }
    }
}
