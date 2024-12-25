#pragma once

#include <stdlib.h>
#include <string>
#include <vector>

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Color.hpp"

#include "include/Utils/StringUtils.hpp"
#include "include/Utils/ModConfig.hpp"
#include "shared/Models/Player.hpp"
#include "shared/Models/Clan.hpp"
#include "shared/Models/Score.hpp"
#include "shared/Models/ClanScore.hpp"
#include "include/API/PlayerController.hpp"
#include <regex>
using namespace std;
using namespace UnityEngine;

namespace FormatUtils {

        const int Second = 1;
        const int Minute = 60 * Second;
        const int Hour = 60 * Minute;
        const int Day = 24 * Hour;
        const int Month = 30 * Day;
        const int Year = 365 * Day;

        inline string GetRelativeTimeString(string_view timeSet) {
            int timeSetSeconds = std::stoi(timeSet.data());
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

        inline string GetRelativeFutureTimeString(string_view timeSet) {
            int timeSetSeconds = std::stoi(timeSet.data());
            int nowSeconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            int delta = -(nowSeconds - timeSetSeconds);
            if (delta < 1 * Minute) return delta == 1 ? "one second left" : to_string(delta) + " seconds left";
            if (delta < 2 * Minute) return "a minute left";
            if (delta < 45 * Minute) return to_string(delta / Minute) + " minutes left";
            if (delta < 90 * Minute) return "an hour left";
            if (delta < 24 * Hour) return to_string(delta / Hour) + " hours left";
            if (delta < 48 * Hour) return "tomorrow";
            if (delta < 30 * Day) return to_string(delta / Day) + " days left";
            if (delta < 12 * Month) {
                int months = delta / Month;
                return months <= 1 ? "one month left" : to_string(months) + " months left";
            }
            int years = delta / Year;
            return years <= 1 ? "one year left" : to_string(years) + " years left";
        }

        inline string FormatRank(int rank, bool withPrefix) {
            return (withPrefix ? "<size=70%>#</size>" : "") + to_string(rank);
        }

        inline string FormatUserName(string_view userName) {
            return "<noparse>" + string(userName) + "</noparse>";
        }

        const string ModifiersColor = "";

        inline string FormatModifiers(string_view modifiers) {
            return modifiers.length() == 0 ? "" : "<color=#999999>" + string(modifiers);
        }

        static Color lowAccColor = UnityEngine::Color(0.93, 1, 0.62, 1);
        static Color highAccColor = UnityEngine::Color(1, 0.39, 0.28, 1);

        inline string rgb2hex(Color const& color) {
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
            return "<color=#" + getAccColorString(accuracy) + ">" + to_string_wprecision(accuracy * 100, 2) + "<size=70%>%</size></color>";
        }

        inline string FormatPP(float value) {
            return "<color=#B856FF>" + to_string_wprecision(value, 2) + "<size=70%>pp</size></color>";
        }

        inline string FormatClanTag(string_view tag) {
            return "<alpha=#00>.<alpha=#FF><b><noparse>" + string(tag) + "</noparse></b><alpha=#00>.<alpha=#FF>";
        }

        inline string FormatNameWithClans(Player const& player, int limit, bool withClans) {
            string clansLabel = withClans ? "<size=90%>" : "";
            int clanCount = player.clans.size();
            if (withClans) {                
                if (clanCount == 2) {
                    clansLabel = "<size=80%>";
                } 
                else if (clanCount == 3) {
                    clansLabel = "<size=70%>";
                }

                for (size_t i = 0; i < clanCount; i++) {
                    Clan clan = player.clans[i];
                    clansLabel += "  <color=" + clan.color + ">" + clan.tag + "</color>";
                }
                clansLabel += "</size>";
            }

            string name = "";
            if (!player.name.empty() && player.name != "<blank>") {
                name = player.name;
            } else {
                name = "[REDACTED]";
            }

            return "<noparse>" + truncate(name, limit - clanCount * 3) + "</noparse>" + clansLabel;
        }

        inline string FormatPlayerScore(Score const& score) {
            string fcLabel = "<color=#FFFFFF>" + (string)(score.fullCombo ? "FC" : "") + (score.modifiers.length() > 0 && score.fullCombo ? "," : "") + score.modifiers;
            string name = "";

            if (!PlayerController::IsIncognito(score.player)) {
                name = getModConfig().ClansActive.GetValue() ? FormatNameWithClans(score.player, 24, true) : "<noparse>" + truncate(score.player.name, 24) + "</noparse>";
            } 
            else {
                name = "[REDACTED]";
            }

            string time = getModConfig().TimesetActive.GetValue() ? " <size=60%>" + GetRelativeTimeString(score.timeset) + "</size>" : "";

            return name + "<pos=40%>" + FormatPP(score.pp) + "   " + formatAcc(score.accuracy) + " " + fcLabel + time;
        }

        inline string FormatClanScore(ClanScore const& score) {
            string name = "";
            
            name = "<noparse>" + truncate(score.clan.name, 24) + "</noparse>  <color=" + score.clan.color + ">" + score.clan.tag + "</color>";

            string time = getModConfig().TimesetActive.GetValue() ? " <size=60%>" + GetRelativeTimeString(score.timeset) + "</size>" : "";

            return name + "<pos=40%>" + FormatPP(score.pp) + "   " + formatAcc(score.accuracy) + " " + time;
        }

        inline string GetFullPlatformName(string serverPlatform) {
            
            if (serverPlatform == "oculus") {
                return "Oculus Standalone";
            } else if (serverPlatform == "oculuspc") {
                return "Oculus PC";
            } else {
                return "Steam";
            }
        }

        inline float ParseFloat(string_view value) {
            return std::stof(value.data());
        }

        inline int ParseInt(string_view value) {
            return std::stoi(value.data());
        }

        inline bool ParseBool(string_view value) {
            return std::string(value) == "true";
        }

        template<typename T>
        inline T ParseEnum(string_view value) {
            return static_cast<T>(std::stoi(value.data()));
        }

        inline ::UnityEngine::Color ParseColor(string_view value) {
            return UnityEngine::Color(ParseFloat(value.substr(0, 2)), ParseFloat(value.substr(2, 4)), ParseFloat(value.substr(4, 6)), ParseFloat(value.substr(6, 8)));
        }
    }