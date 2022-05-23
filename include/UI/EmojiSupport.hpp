#pragma once

#include "TMPro/TMP_Text.hpp"
#include "TMPro/TextMeshProUGUI.hpp"

using namespace std;

#include <string>

class EmojiSupport
{
public:
   static void AddSupport(TMPro::TextMeshProUGUI* text);
   static void Reset();
};
