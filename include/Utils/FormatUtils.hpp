#include <stdlib.h>
#include <string>
#include <vector>

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Color.hpp"

#include "include/Utils/StringUtils.hpp"

using namespace std;
using namespace UnityEngine;

namespace FormatUtils {

        inline string GetHeadsetNameById(int id) {
            switch (id) {
                case 1: return "Rift";
                case 2: return "Vive";
                case 4: return "Vive Pro";
                case 8: return "WMR";
                case 16: return "Rift S";
                case 32: return "Quest";
                case 64: return "Index";
                case 128: return "Vive Cosmos";
                case 256: return "Quest 2";
            }

            return "Unknown HMD";
        }

        const int Second = 1;
        const int Minute = 60 * Second;
        const int Hour = 60 * Minute;
        const int Day = 24 * Hour;
        const int Month = 30 * Day;
        const int Year = 365 * Day;

        inline string GetRelativeTimeString(string timeSet) {
            int timeSetSeconds = std::stoi(timeSet);
            int nowSeconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            int delta = nowSeconds - timeSetSeconds;
            if (delta < 1 * Minute) return delta == 1 ? "one second ago" : to_string(delta) + " seconds ago";
            if (delta < 2 * Minute) return "a minute ago";
            if (delta < 45 * Minute) return to_string(delta / Minute) + " minutes ago";
            if (delta < 90 * Minute) return "an hour ago";
            if (delta < 24 * Hour) return to_string(delta / Hour) + " hours ago";
            if (delta < 48 * Hour) return "yesterday";
            if (delta < 30 * Day) return to_string(delta / Day) + " days ago";
            if (delta < 12 * Month) {
                int months = delta / Month;
                return months <= 1 ? "one month ago" : to_string(months) + " months ago";
            }
            int years = delta / Year;
            return years <= 1 ? "one year ago" : to_string(years) + " years ago";
        }

        inline string FormatRank(int rank, bool withPrefix) {
            return (withPrefix ? "<size=70%>#</size>" : "") + to_string(rank);
        }

        inline string FormatUserName(string userName) {
            return "<noparse>" + userName + "</noparse>";
        }

        const string ModifiersColor = "";

        inline string FormatModifiers(string modifiers) {
            return modifiers.length() == 0 ? "" : "<color=#999999>" + modifiers;
        }

        static Color lowAccColor = UnityEngine::Color(0.93, 1, 0.62, 1);
        static Color highAccColor = UnityEngine::Color(1, 0.39, 0.28, 1);

        inline string rgb2hex(Color color) { 
            stringstream ss; 
            ss << std::hex << ((int)(color.r * 255.0) << 16 | (int)(color.g * 255.0) << 8 | (int)(color.b * 255.0)); 
            return ss.str();
        }

        inline string getAccColorString(float acc) {
            auto lerpValue = pow(acc, 14.0f);
            auto color = Color::Lerp(lowAccColor, highAccColor, lerpValue);
            return rgb2hex(color);
        }

        inline string formatAcc(float accuracy) {
            return "<color=#" + getAccColorString(accuracy) + ">" + to_string_wprecision(accuracy * 100, 2) + "<size=70%>%</size>";
        }

        inline string FormatPP(float value) {
            return "<color=#B856FF>" + to_string_wprecision(value, 2) + "<size=70%>pp</size>";
        }
    }