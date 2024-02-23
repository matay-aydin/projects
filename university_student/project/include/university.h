#ifndef _UNIVERSITY
#define _UNIVERSITY
#include "student.h"
#include <string>

class University
{
public:
    University(std::string pName, double pW_gpa,
               double pW_gre, double pW_toefl,
               double pBias, std::string pCountry = "");
    University(const University &);
    void evaluate_student(const Student &) const;

private:
    std::string name{""};
    double w_gpa;
    double w_gre;
    double w_toefl;
    double bias;
    std::string country{""};
};

#endif