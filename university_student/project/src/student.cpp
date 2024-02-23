#include "student.h"
#include <iostream>

Student::Student(std::string pName, double pGpa,
                 double pGre, int pToefl, int pApp)
    : name{pName}, gpa{pGpa}, gre{pGre}, toefl{pToefl}, app{pApp}
{
    std::cout << this->name << " logged in to the system." << std::endl;
}

Student::Student(const Student &s)
    : Student::Student(s.name, s.gpa, s.gre, s.toefl, s.app)
{
}

Student::~Student()
{
    std::cout << this->name << " logged out of the system with "
              << this->app << " application(s)." << std::endl;
}

void Student::set_name(std::string nName)
{
    this->name = nName;
}

void Student::set_app(int nApp) const
{
    this->app = nApp;
}

std::string Student::get_name() const
{
    return this->name;
}

double Student::get_gpa() const
{
    return this->gpa;
}

double Student::get_gre() const
{
    return this->gre;
}

int Student::get_toefl() const
{
    return this->toefl;
}

int Student::get_app() const
{
    return this->app;
}