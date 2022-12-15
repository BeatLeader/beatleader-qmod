#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

template <typename T>
inline string to_string_wprecision(const T& a_value, const int n = 6)
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

inline string toLower(string_view const value) {
    string result;
    result.resize(value.size());
    transform(value.begin(), value.end(), result.begin(), asciitolower);

    return result;
}

inline size_t findCutPosition(string_view const str, size_t max_size) {
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

inline string truncate(string_view const str, size_t width, bool show_ellipsis=true)
{
    if (str.size() > width) {
        if (show_ellipsis) {
            return std::string(str.substr(0, findCutPosition(str, width))) + "...";
        } else {
            return std::string(str.substr(0, findCutPosition(str, width)));
        }
    }
            
    return std::string(str);
}

inline vector<string> split(string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}