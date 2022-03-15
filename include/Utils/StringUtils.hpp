#pragma once
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

template <typename T>
inline string to_string_wprecision(const T a_value, const int n = 6) 
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}