

#include <iostream>
#include "himl.h"
#include <sstream>
#include <fstream>

using namespace himl;

int main()
{
    std::ifstream in("./test.txt");
    std::string str = "";
    std::string tmp;
    while (getline(in, tmp)) {
        str += tmp;
        str += "\r\n";
    }
    HimlParser parser;
    Value* value0 = parser.parse(str);
    printf("%s\n", parser.getError().c_str());
    std::string jstr;
    value0->to_string(jstr);

    Value* value1 = parser.parse(jstr);
    printf("%s\n", parser.getError().c_str());
    std::string jstr2;
    value1->to_string(jstr2);
    printf("%s\n", jstr2.c_str());
}
