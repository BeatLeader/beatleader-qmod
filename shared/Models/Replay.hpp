#pragma once

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "sombrero/shared/FastVector3.hpp"
#include "sombrero/shared/FastQuaternion.hpp"
#include "sombrero/shared/Vector3Utils.hpp"


using namespace std;

struct ReplayInfo {
    string version;
    string gameVersion;
    string timestamp;

    ReplayInfo(string version, string gameVersion, string timestamp) : version(version),
                                                                                            gameVersion(gameVersion),
                                                                                            timestamp(timestamp) {}

    string playerID;
    string playerName;
    string platform;

    string trackingSystem;
    string hmd;
    string controller;

    string hash;
    string songName;
    string mapper;
    string difficulty;

    int score;
    string mode;
    string environment;
    string modifiers;
    float jumpDistance = 0;
    bool leftHanded = false;
    float height = 0;

    float startTime = 0;
    float failTime = 0;
    float speed = 0;
};

struct ReplayTransform {
    Sombrero::FastVector3 position;
    Sombrero::FastQuaternion rotation;

    constexpr ReplayTransform(Sombrero::FastVector3 const &position, Sombrero::FastQuaternion const &rotation) : position(position),
                                                                                                 rotation(rotation) {}
};

struct Frame {
    float time;
    int fps;
    ReplayTransform head;
    ReplayTransform leftHand;
    ReplayTransform rightHand;

    constexpr Frame(float time, int fps, ReplayTransform const &head, ReplayTransform const &leftHand, ReplayTransform const &rightHand) : time(
            time), fps(fps), head(head), leftHand(leftHand), rightHand(rightHand) {}
};

struct ReplayNoteCutInfo {
    bool speedOK;
    bool directionOK;
    bool saberTypeOK;
    bool wasCutTooSoon;
    float saberSpeed;
    bool cutDistanceToCenterPositive;
    Sombrero::FastVector3 saberDir;
    int saberType;
    float timeDeviation;
    float cutDirDeviation;
    Sombrero::FastVector3 cutPoint;
    Sombrero::FastVector3 cutNormal;
    float cutDistanceToCenter;
    float cutAngle;
    float beforeCutRating;
    float afterCutRating;
};

enum struct NoteEventType {
    GOOD = 0,
    BAD = 1,
    MISS = 2,
    BOMB = 3
};

struct NoteEvent {
    int noteID;
    float spawnTime;
    float eventTime;
    NoteEventType eventType = NoteEventType::GOOD;
    ReplayNoteCutInfo noteCutInfo;

    constexpr NoteEvent(int noteId, float spawnTime) : noteID(noteId), spawnTime(spawnTime) {}
};

struct WallEvent {
    int wallID;
    float spawnTime;

    float energy;
    float time;

    constexpr WallEvent(int wallId, float spawnTime) : wallID(wallId), spawnTime(spawnTime) {}
};

struct AutomaticHeight {
    constexpr AutomaticHeight(float height, float time) : height(height), time(time) {}

    float height;
    float time;
};

struct Pause {
    long duration;
    float time;
};

class Replay
{
public:
    void Encode(ofstream& stream) const;
    static std::optional<ReplayInfo> DecodeInfo(ifstream& stream);

    explicit Replay(ReplayInfo info) : info(std::move(info)) {}
    Replay(Replay&&) = default; // make move default, avoid copying


    ReplayInfo info;
    vector<Frame> frames;
    vector<NoteEvent> notes;
    vector<WallEvent> walls;
    vector<AutomaticHeight> heights;
    vector<Pause> pauses;
private:
    static void Encode(char value, ofstream& stream);
    static void Encode(int value, ofstream& stream);
    static void Encode(long value, ofstream& stream);
    static void Encode(bool value, ofstream& stream);
    static void Encode(float value, ofstream& stream);
    static void Encode(string value, ofstream& stream);

    // template methods MUST be header only
    template<class T>
    static void Encode(vector<T> const& list, ofstream& stream) {
        size_t count = list.size();
        Encode((int)count, stream);
        for (size_t i = 0; i < count; i++)
        {
            Encode(list[i], stream);
        }
    }

    static void Encode(ReplayInfo const &info, ofstream& stream);
    static void Encode(Sombrero::FastVector3 const &vector, ofstream& stream);
    static void Encode(Sombrero::FastQuaternion const &quaternion, ofstream& stream);
    static void Encode(Frame const &frame, ofstream& stream);
    static void Encode(NoteEvent const &note, ofstream& stream);
    static void Encode(WallEvent const &wall, ofstream& stream);
    static void Encode(AutomaticHeight const &height, ofstream& stream);
    static void Encode(Pause const &pause, ofstream& stream);

    static char DecodeChar(ifstream& stream);
    static int DecodeInt(ifstream& stream);
    static long DecodeLong(ifstream& stream);
    static bool DecodeBool(ifstream& stream);
    static float DecodeFloat(ifstream& stream);
    static string DecodeString(ifstream& stream);
};