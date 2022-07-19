#pragma once
#include <string>
#include <iostream>
#include <sstream>

#include "main.hpp"

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

inline string toLower(string const value) {
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

/// @brief Create a new csstr from a UTF16 string view.
/// @tparam creationType The creation type for the string.
/// @param inp The input string to create.
/// @return The returned string.
template<il2cpp_utils::CreationType creationType = il2cpp_utils::CreationType::Temporary>
Il2CppString* newcsstr2(std::u16string_view inp) {
    il2cpp_functions::Init();
    if constexpr (creationType == il2cpp_utils::CreationType::Manual) {
        auto len = inp.length();
        auto mallocSize = sizeof(Il2CppString) + sizeof(Il2CppChar) * (len + 1);
        // String never has any references anyways, malloc is safe here because the string gets copied over anyways.
        auto* str = reinterpret_cast<il2cpp_utils::__InternalCSStr*>(malloc(mallocSize));
        str->object.klass = il2cpp_functions::defaults->string_class;
        str->object.monitor = nullptr;
        str->length = len;
        for (size_t i = 0; i < len; i++) {
            str->chars[i] = inp[i];
        }
        str->chars[len] = '\0';
        return reinterpret_cast<Il2CppString*>(str);
    } else {
        return il2cpp_functions::string_new_utf16(reinterpret_cast<const Il2CppChar*>(inp.data()), inp.length());
    }
}

/// @brief Create a new csstr from a UTF8 string view.
/// @tparam creationType The creation type for the string.
/// @param inp The input string to create.
/// @return The returned string.
template<il2cpp_utils::CreationType creationType = il2cpp_utils::CreationType::Temporary>
Il2CppString* newcsstr2(std::string inp) {
    il2cpp_functions::Init();
    if constexpr (creationType == il2cpp_utils::CreationType::Manual) {
        // TODO: Perhaps manually call createManual instead
        auto len = inp.length();
        auto mallocSize = sizeof(Il2CppString) + sizeof(Il2CppChar) * (len + 1);
        // String never has any references anyways, malloc is safe here because the string gets copied over anyways.
        auto* str = reinterpret_cast<il2cpp_utils::__InternalCSStr*>(malloc(mallocSize));
        str->object.klass = il2cpp_functions::defaults->string_class;
        str->object.monitor = nullptr;
        str->length = len;
        for (size_t i = 0; i < len; i++) {
            str->chars[i] = inp[i];
        }
        str->chars[len] = '\0';
        return reinterpret_cast<Il2CppString*>(str);
    } else {
        return il2cpp_functions::string_new_len(inp.data(), inp.size());
    }
}
