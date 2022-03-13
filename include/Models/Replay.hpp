#pragma once

#include <stdlib.h>
#include <string>
#include <vector>

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"

using namespace std;

class ReplayInfo {
public:
    ReplayInfo();
    ~ReplayInfo();

    string version;
    string gameVersion;
    string timestamp;
        
    string playerID;
    string playerName;
    string platform;

    string trackingSytem;
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

class Vector3 {
public:
    Vector3();
    Vector3(UnityEngine::Vector3 vec);
    ~Vector3();

    float x;
    float y;
    float z;
};

class Quaternion {
public:
    Quaternion();
    Quaternion(UnityEngine::Quaternion quat);
    ~Quaternion();

    float x;
    float y;
    float z;
    float w;
};

class Transform {
public:
    Transform();
    ~Transform();

    Vector3* position;
    Quaternion* rotation;
};

class Frame {
public:
    Frame();
    ~Frame();

    float time;
    int fps;
    Transform* head;
    Transform* leftHand;
    Transform* rightHand;
};

class ReplayNoteCutInfo {
public:
    ReplayNoteCutInfo();
    ~ReplayNoteCutInfo();

    bool speedOK;
    bool directionOK;
    bool saberTypeOK;
    bool wasCutTooSoon;
    float saberSpeed;
    Vector3* saberDir = new Vector3();
    int saberType;
    float timeDeviation;
    float cutDirDeviation;
    Vector3* cutPoint = new Vector3();
    Vector3* cutNormal  = new Vector3();
    float cutDistanceToCenter;
    float cutAngle;
    float beforeCutRating;
    float afterCutRating;
};

enum NoteEventType {
    good = 0,
    bad = 1,
    miss = 2,
    bomb = 3
};

class NoteEvent {
public:
    NoteEvent();
    ~NoteEvent();

    int noteID;
    float eventTime;
    float spawnTime;
    NoteEventType eventType = NoteEventType::good;
    ReplayNoteCutInfo* noteCutInfo = new ReplayNoteCutInfo();
};

class WallEvent {
public:
    WallEvent();
    ~WallEvent();

    int wallID;
    float energy;
    float time;
    float spawnTime;
};

class AutomaticHeight {
public:
    AutomaticHeight();
    ~AutomaticHeight();

    float height;
    float time;
};

class Pause {
public:
    Pause();
    ~Pause();

    long duration;
    float time;
};

class Replay
{
public:
    Replay();
    ~Replay();

    void Encode(ofstream& stream);
    static ReplayInfo* DecodeInfo(ifstream& stream);

    ReplayInfo* info = new ReplayInfo();
    vector<Frame*> frames;
    vector<NoteEvent*> notes;
    vector<WallEvent*> walls;
    vector<AutomaticHeight*> heights;
    vector<Pause*> pauses;
private:
    void Encode(char value, ofstream& stream);
    void Encode(int value, ofstream& stream);
    void Encode(long value, ofstream& stream);
    void Encode(bool value, ofstream& stream);
    void Encode(float value, ofstream& stream);
    void Encode(string value, ofstream& stream);

    template<class T>
    void Encode(vector<T> list, ofstream& stream);
    void Encode(ReplayInfo* info, ofstream& stream);
    void Encode(Vector3* vector, ofstream& stream);
    void Encode(Quaternion* quaternion, ofstream& stream);
    void Encode(Frame* frame, ofstream& stream);
    void Encode(NoteEvent* note, ofstream& stream);
    void Encode(WallEvent* wall, ofstream& stream);
    void Encode(AutomaticHeight* height, ofstream& stream);
    void Encode(Pause* pause, ofstream& stream);

    static char DecodeChar(ifstream& stream);
    static int DecodeInt(ifstream& stream);
    static long DecodeLong(ifstream& stream);
    static bool DecodeBool(ifstream& stream);
    static float DecodeFloat(ifstream& stream);
    static string DecodeString(ifstream& stream);
};