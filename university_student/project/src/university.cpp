#include "university.h"
#include <iostream>

University::University(std::string pName, double pW_gpa,
                       double pW_gre, double pW_toefl,
                       double pBias, std::string pCountry)
    : name{pName}, w_gpa{pW_gpa}, w_gre{pW_gre},
      w_toefl{pW_toefl}, bias{pBias}, country{pCountry}
{
}

University::University(const University &uni)
    : University::University(uni.name, uni.w_gpa, uni.w_gre,
                             uni.w_toefl, uni.bias, uni.country)
{
}

void University::evaluate_student(const Student &s) const
{
    int sapp = s.get_app();
    s.set_app(sapp + 1);
    double z;
    z = this->w_gpa * s.get_gpa() + this->w_gre * s.get_gre();
    z += this->w_toefl * s.get_toefl() + this->bias;
    if (z >= 0)
        std::cout << s.get_name() << " is admitted to "
                  << this->name << "." << std::endl;
    else
        std::cout << s.get_name() << " is rejected from "
                  << this->name << "." << std::endl;
}