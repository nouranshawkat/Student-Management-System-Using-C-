#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>
using namespace std;

class Course;
class Grade;

class Gradebook {
    map<int, map<int, double>> grades;
    Gradebook() = default;

public:
    static Gradebook& getInstance() {
        static Gradebook instance;
        return instance;
    }

    void addGrade(int studentId, int courseId, double percentage);
    double getGrade(int studentId, int courseId) const;
    vector<Grade> getStudentGrades(int studentId) const;

    Gradebook(const Gradebook&) = delete;
    void operator=(const Gradebook&) = delete;
};

class Grade {
    int studentId;
    int courseId;
    double percentageGrade;

public:
    Grade(int sid, int cid, double g)
        : studentId(sid), courseId(cid), percentageGrade(g) {}

    double getPercentageGrade() const { return percentageGrade; }
    int getStudentId() const { return studentId; }
    int getCourseId() const { return courseId; }
};

void Gradebook::addGrade(int studentId, int courseId, double percentage) {
    grades[studentId][courseId] = percentage;
}

double Gradebook::getGrade(int studentId, int courseId) const {
    auto studentIt = grades.find(studentId);
    if (studentIt != grades.end()) {
        auto courseIt = studentIt->second.find(courseId);
        if (courseIt != studentIt->second.end()) {
            return courseIt->second;
        }
    }
    return -1.0;
}

vector<Grade> Gradebook::getStudentGrades(int studentId) const {
    vector<Grade> result;
    auto studentIt = grades.find(studentId);
    if (studentIt != grades.end()) {
        for (const auto& courseEntry : studentIt->second) {
            result.emplace_back(studentId, courseEntry.first, courseEntry.second);
        }
    }
    return result;
}

class Course {
    int courseId;
    string name;
    string description;
    int credits;
    vector<Course*> prerequisites;
    vector<class Student*> enrolledStudents;

public:
    Course(int id, const string& n, const string& desc, int cred)
        : courseId(id), name(n), description(desc), credits(cred) {}

    void addPrerequisite(Course* course) {
        prerequisites.push_back(course);
    }

    bool validatePrerequisites(int studentId) const {
        for (const auto& prereq : prerequisites) {
            double grade = Gradebook::getInstance().getGrade(studentId, prereq->getCourseId());
            if (grade < 60.0) return false;
        }
        return true;
    }

    void enrollStudent(Student* student);
    void dropStudent(Student* student);

    string getName() const { return name; }
    int getCredits() const { return credits; }
    int getCourseId() const { return courseId; }
};

class Student {
    int studentId;
    string name;
    string dateOfBirth;
    string contactInfo;
    int academicYear;
    int semester;
    double gpa;
    vector<Course*> enrolledCourses;

    string getLetterGrade(double percentage) const {
        if (percentage >= 90.0) return "A";
        else if (percentage >= 80.0) return "B";
        else if (percentage >= 70.0) return "C";
        else if (percentage >= 60.0) return "D";
        else return "F";
    }

public:
    Student(int id, const string& n, const string& dob, const string& contact, int year, int sem)
        : studentId(id), name(n), dateOfBirth(dob), contactInfo(contact),
        academicYear(year), semester(sem), gpa(0.0) {}

    void enroll(Course* course);
    void autoEnroll(const vector<Course*>& semesterCourses);
    void drop(Course* course);
    double calculateGPA() const;
    void saveToFile() const;
    void printTranscript() const;

    int getId() const { return studentId; }
    vector<Course*>& getEnrolledCourses() { return enrolledCourses; }
};

void Course::enrollStudent(Student* student) {
    enrolledStudents.push_back(student);
}

void Course::dropStudent(Student* student) {
    auto it = find(enrolledStudents.begin(), enrolledStudents.end(), student);
    if (it != enrolledStudents.end()) enrolledStudents.erase(it);
}

void Student::enroll(Course* course) {
    enrolledCourses.push_back(course);
    course->enrollStudent(this);
}

void Student::autoEnroll(const vector<Course*>& semesterCourses) {
    for (auto course : semesterCourses) {
        enroll(course);
    }
}

void Student::drop(Course* course) {
    auto it = find(enrolledCourses.begin(), enrolledCourses.end(), course);
    if (it != enrolledCourses.end()) {
        enrolledCourses.erase(it);
        course->dropStudent(this);
    }
}

double Student::calculateGPA() const {
    double totalPoints = 0.0;
    int totalCredits = 0;
    auto& gb = Gradebook::getInstance();

    for (const auto& course : enrolledCourses) {
        double grade = gb.getGrade(studentId, course->getCourseId());
        if (grade >= 0) {
            string letter = getLetterGrade(grade);
            double gradePoint;
            if (letter == "A") gradePoint = 4.0;
            else if (letter == "B") gradePoint = 3.0;
            else if (letter == "C") gradePoint = 2.0;
            else if (letter == "D") gradePoint = 1.0;
            else gradePoint = 0.0;

            totalPoints += gradePoint * course->getCredits();
            totalCredits += course->getCredits();
        }
    }
    return totalCredits > 0 ? totalPoints / totalCredits : 0.0;
}

void Student::saveToFile() const {
    ofstream file("students.txt", ios::app);
    if (file) {
        file << studentId << "," << name << "," << dateOfBirth << ","
            << contactInfo << "," << academicYear << "," << semester << "\n";
    }
}

void Student::printTranscript() const {
    auto& gb = Gradebook::getInstance();
    cout << "\nTranscript for " << name << " (" << studentId << ")\n"
        << "Year: " << academicYear << " Semester: " << semester << "\n";

    cout << left << setw(25) << "Course" << setw(10) << "Credits"
        << setw(8) << "Grade" << "Letter" << endl;
    cout << string(50, '-') << endl;

    for (const auto& course : enrolledCourses) {
        double grade = gb.getGrade(studentId, course->getCourseId());
        stringstream gradeFormatted;
        string letterGrade;
        string gradeStr;

        if (grade >= 0) {
            gradeFormatted << fixed << setprecision(0) << grade << "%";
            gradeStr = gradeFormatted.str();
            letterGrade = getLetterGrade(grade);
        }
        else {
            gradeStr = "In progress";
            letterGrade = "";
        }

        cout << left << setw(25) << course->getName()
            << setw(10) << course->getCredits()
            << setw(8) << gradeStr
            << letterGrade << endl;
    }

    cout << string(50, '-') << endl;
    cout << "GPA: " << fixed << setprecision(2) << calculateGPA() << "\n\n";
}

class Curriculum {
    map<pair<int, int>, vector<Course*>> semesterMap;

    void initializeCourses() {
        // First Year Semester 1 (1,1)
        Course* ap1 = new Course(1101, "Applied Physics-I", "Fundamental physics principles", 4);
        Course* ce1 = new Course(1102, "Communicative English-I", "Basic communication skills", 3);
        Course* cm = new Course(1103, "Calculus & Matrices", "Advanced mathematics", 4);
        Course* ec1 = new Course(1104, "Engineering Chemistry-I", "Basic chemical principles", 4);
        Course* es = new Course(1105, "Environmental Science", "Environmental studies", 3);
        Course* bcs = new Course(1106, "Basic Computer Science", "Introduction to computing", 4);
        Course* ws1 = new Course(1107, "Workshop-I", "Practical sessions", 2);
        semesterMap[{1, 1}] = { ap1, ce1, cm, ec1, es, bcs, ws1 };

        // First Year Semester 2 (1,2)
        Course* ap2 = new Course(1201, "Applied Physics-II", "Advanced physics topics", 4);
        ap2->addPrerequisite(ap1);
        Course* ce2 = new Course(1202, "Communicative English-II", "Advanced communication", 3);
        Course* beee = new Course(1203, "Basic Electrical Engineering", "Electrical fundamentals", 4);
        Course* ed = new Course(1204, "Engineering Drawing", "Technical drawing techniques", 3);
        Course* ec2 = new Course(1205, "Engineering Chemistry-II", "Advanced chemistry", 4);
        ec2->addPrerequisite(ec1);
        Course* coa = new Course(1206, "Computer Organization", "Computer architecture basics", 4);
        coa->addPrerequisite(bcs);
        Course* ws2 = new Course(1207, "Workshop-II", "Practical sessions", 2);
        semesterMap[{1, 2}] = { ap2, ce2, beee, ed, ec2, coa, ws2 };

        // Second Year Semester 1 (2,1)
        Course* math1 = new Course(2101, "Mathematics-I", "Discrete mathematics", 4);
        Course* dsa = new Course(2102, "Data Structures", "Algorithms and structures", 4);
        dsa->addPrerequisite(bcs);
        Course* ctn = new Course(2103, "Circuit Theory", "Network analysis", 4);
        ctn->addPrerequisite(beee);
        Course* co = new Course(2104, "Computer Organization", "Hardware organization", 4);
        co->addPrerequisite(coa);
        Course* deld = new Course(2105, "Digital Electronics", "Logic design principles", 4);
        deld->addPrerequisite(beee);
        Course* ppl = new Course(2106, "Programming Languages", "Language paradigms", 4);
        ppl->addPrerequisite(bcs);
        semesterMap[{2, 1}] = { math1, dsa, ctn, co, deld, ppl };

        // Second Year Semester 2 (2,2)
        Course* math2 = new Course(2201, "Mathematics-II", "Advanced mathematics topics", 4);
        math2->addPrerequisite(math1);
        Course* automata = new Course(2202, "Automata Theory", "Formal languages and automata", 4);
        automata->addPrerequisite(math1);
        Course* or_opt = new Course(2203, "Operations Research & Optimization", "Optimization techniques", 4);
        or_opt->addPrerequisite(math1);
        Course* pce = new Course(2204, "Principles of Communication English", "Technical communication", 3);
        pce->addPrerequisite(ce2);
        Course* aca = new Course(2205, "Advanced Computer Architecture", "Advanced hardware design", 4);
        aca->addPrerequisite(co);
        Course* trw = new Course(2206, "Technical Report Writing", "Writing technical documents", 3);
        trw->addPrerequisite(pce);
        semesterMap[{2, 2}] = { math2, automata, or_opt, pce, aca, trw };

        // Third Year Semester 5 (3,1)
        Course* os = new Course(3101, "Operating Systems", "OS concepts and design", 4);
        os->addPrerequisite(dsa);
        os->addPrerequisite(co);
        Course* dbms = new Course(3102, "Database Management Systems", "Database design and SQL", 4);
        dbms->addPrerequisite(dsa);
        Course* daa = new Course(3103, "Design & Analysis of Algorithms", "Advanced algorithms", 4);
        daa->addPrerequisite(dsa);
        Course* micro = new Course(3104, "Microprocessors & Microcontrollers", "Microprocessor architecture", 4);
        micro->addPrerequisite(deld);
        micro->addPrerequisite(co);
        Course* cs = new Course(3105, "Control Systems", "Control theory and systems", 4);
        cs->addPrerequisite(ctn);
        cs->addPrerequisite(math1);
        semesterMap[{3, 1}] = { os, dbms, daa, micro, cs };

        // Third Year Semester 6 (3,2)
        Course* cn = new Course(3201, "Computer Networks", "Network architectures and protocols", 4);
        cn->addPrerequisite(os);
        cn->addPrerequisite(dsa);
        Course* swe = new Course(3202, "Software Engineering", "Software development lifecycle", 4);
        swe->addPrerequisite(ppl);
        Course* cg = new Course(3203, "Computer Graphics & Multimedia", "Graphics programming and multimedia systems", 4);
        cg->addPrerequisite(dsa);
        cg->addPrerequisite(math1);
        Course* ssa = new Course(3204, "System Software & Administration", "System software and admin tasks", 4);
        ssa->addPrerequisite(os);
        Course* otu = new Course(3205, "Object Technology & UML", "Object-oriented design and UML modeling", 4);
        otu->addPrerequisite(ppl);
        semesterMap[{3, 2}] = { cn, swe, cg, ssa, otu };

        // Fourth Year Semester 7 (4,1)
        Course* lp = new Course(4101, "Language Processors", "Compilers and interpreters", 4);
        lp->addPrerequisite(automata);
        lp->addPrerequisite(daa);
        Course* ai = new Course(4102, "Artificial Intelligence", "AI principles and techniques", 4);
        ai->addPrerequisite(daa);
        ai->addPrerequisite(math2);
        Course* vpwt = new Course(4103, "Visual Programming & Web Technology", "GUI and web development", 4);
        vpwt->addPrerequisite(ppl);
        vpwt->addPrerequisite(dbms);
        Course* fma = new Course(4104, "Financial Management & Accounts", "Financial basics for engineers", 3);
        Course* elective1 = new Course(4105, "Elective I", "Specialization course", 3);
        Course* pt = new Course(4106, "Practical Training", "Industry training", 3);
        semesterMap[{4, 1}] = { lp, ai, vpwt, fma, elective1, pt };

        // Fourth Year Semester 8 (4,2)
        Course* vep = new Course(4201, "Values & Ethics in Profession", "Professional ethics", 2);
        Course* im = new Course(4202, "Industrial Management", "Management principles", 3);
        Course* elective2 = new Course(4203, "Elective II", "Specialization course", 3);
        Course* elective3 = new Course(4204, "Elective III", "Specialization course", 3);
        Course* cvv = new Course(4205, "Comprehensive Viva-Voce", "Oral examination", 2);
        Course* pd = new Course(4206, "Personality Development", "Soft skills training", 2);
        Course* seminar = new Course(4207, "Seminar", "Research presentation", 2);
        Course* project = new Course(4208, "Project Work", "Capstone project", 6);
        project->addPrerequisite(swe);
        project->addPrerequisite(daa);
        project->addPrerequisite(os);
        semesterMap[{4, 2}] = { vep, im, elective2, elective3, cvv, pd, seminar, project };
    };

public:
    Curriculum() { initializeCourses(); }

    vector<Course*> getCoursesForSemester(int year, int sem) {
        auto key = make_pair(year, sem);
        return semesterMap.count(key) ? semesterMap[key] : vector<Course*>();
    }
};

int main() {
    Curriculum curriculum;

    int id, year, sem;
    string name, dob, contact;

    cout << "Student Registration System\n";
    cout << "Enter academic year (1-4): ";
    cin >> year;
    cout << "Enter semester (1-2): ";
    cin >> sem;

    if (year < 1 || year > 4 || sem < 1 || sem > 2) {
        cerr << "Invalid academic year/semester combination!\n";
        return 1;
    }

    cout << "Enter student ID: ";
    cin >> id;
    cin.ignore();

    cout << "Enter full name: ";
    getline(cin, name);

    cout << "Enter date of birth (YYYY-MM-DD): ";
    getline(cin, dob);

    cout << "Enter contact email: ";
    getline(cin, contact);

    Student student(id, name, dob, contact, year, sem);
    student.saveToFile();

    auto courses = curriculum.getCoursesForSemester(year, sem);
    student.autoEnroll(courses);

    cout << "\nEnter grades for enrolled courses (0-100):\n";
    for (auto course : student.getEnrolledCourses()) {
        double grade;
        do {
            cout << course->getName() << ": ";
            cin >> grade;
        } while (grade < 0 || grade > 100);
        Gradebook::getInstance().addGrade(student.getId(), course->getCourseId(), grade);
    }

    student.printTranscript();
    return 0;
}