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

inline char asciitolower(char in) {
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

inline size_t findCutPosition(string str, size_t max_size) {
  max_size -= 3;
  for (size_t pos = max_size; pos > 0; --pos)
  {
    unsigned char byte = static_cast<unsigned char>(str[pos]); //Perfectly valid
    if ((byte & 0xC0) != 0x80)
      return pos;
  }

  unsigned char byte = static_cast<unsigned char>(str[0]); //Perfectly valid
  if ((byte & 0xC0) != 0x80)
    return 0;

  return max_size;
}

inline string truncate(string str, size_t width, bool show_ellipsis=true)
{
    if (str.size() > width) {
        if (show_ellipsis) {
            return str.substr(0, findCutPosition(str, width)) + "...";
        } else {
            return str.substr(0, findCutPosition(str, width));
        }
    }
            
    return str;
}