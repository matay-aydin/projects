#ifndef _STUDENT
#define _STUDENT
#include <string>

class Student
{
public:
    Student(std::string pName = "", double pGpa = 0.0,
            double pGre = 0.0, int pToefl = 0, int pApp = 0);
    Student(const Student &);
    ~Student();
    void set_name(std::string);
    void set_app(int) const;
    std::string get_name() const;
    double get_gpa() const;
    double get_gre() const;
    int get_toefl() const;
    int get_app() const;

private:
    std::string name{""};
    double gpa{0.0};
    double gre{0.0};
    int toefl{0};
    mutable int app{0};
};

#endif