#include "include/Models/Replay.hpp"
#include "include/main.hpp"
#include <fstream>

enum StructType {
    info = 0,
    frames = 1,
    notes = 2,
    walls = 3,
    heights = 4,
    pauses = 5
};

void Replay::Encode(ofstream& s) {
    getLogger().info("Started encode");
    Encode((int)0x442d3d69, s);
    Encode((char)1, s);

    for (char a = 0; a < ((char)StructType::pauses) + 1; a++)
    {
        Encode(a, s);
        StructType type = (StructType)a;
        switch (type)
        {
            case StructType::info:
            getLogger().info("Started encode info");
                Encode(info, s);
                break;
            case StructType::frames:
            getLogger().info("Started encode frames");
                Encode(frames, s);
                break;
            case StructType::notes:
            getLogger().info("Started encode notes");
                Encode(notes, s);
                break;
            case StructType::walls:
            getLogger().info("Started encode walls");
                Encode(walls, s);
                break;
            case StructType::heights:
            getLogger().info("Started encode heights");
                Encode(heights, s);
                break;
            case StructType::pauses:
            getLogger().info("Started encode pauses");
                Encode(pauses, s);
                break;
        }
    }
}

void Replay::Encode(char value, ofstream& stream) {
    stream.write(reinterpret_cast<const char *>(&value), sizeof(char));
}
void Replay::Encode(int value, ofstream& stream) {
    stream.write(reinterpret_cast<const char *>(&value), sizeof(int));
}
void Replay::Encode(long value, ofstream& stream) {
    stream.write(reinterpret_cast<const char *>(&value), sizeof(long));
}
void Replay::Encode(bool value, ofstream& stream) {
    stream.write(reinterpret_cast<const char *>(&value), sizeof(char));
}
void Replay::Encode(float value, ofstream& stream) {
    stream.write(reinterpret_cast<const char *>(&value), sizeof(int));
}
void Replay::Encode(string value, ofstream& stream) {
    const char* cstring = value.c_str();
    size_t stringLength = strlen(cstring);
    Encode((int)stringLength, stream);
    stream.write(cstring, stringLength);
}

template<class T>
void Replay::Encode(vector<T> list, ofstream& stream) {
    size_t count = list.size();
    Encode((int)count, stream);
    for (size_t i = 0; i < count; i++)
    {
        Encode(list[i], stream);
    }
}

void Replay::Encode(ReplayInfo* info, ofstream& stream) {
    Encode(info->version, stream);
    Encode(info->gameVersion, stream);
    Encode(info->timestamp, stream);

    Encode(info->playerID, stream);
    Encode(info->playerName, stream);
    Encode(info->platform, stream);

    Encode(info->trackingSytem, stream);
    Encode(info->hmd, stream);
    Encode(info->controller, stream);

    Encode(info->hash, stream);
    Encode(info->songName, stream);
    Encode(info->mapper, stream);
    Encode(info->difficulty, stream);

    Encode(info->score, stream);
    Encode(info->mode, stream);
    Encode(info->environment, stream);
    Encode(info->modifiers, stream);
    Encode(info->jumpDistance, stream);
    Encode(info->leftHanded, stream);
    Encode(info->height, stream);

    Encode(info->startTime, stream);
    Encode(info->failTime, stream);
    Encode(info->speed, stream);
}
void Replay::Encode(Vector3* vector, ofstream& stream) {
    Encode(vector->x, stream);
    Encode(vector->y, stream);
    Encode(vector->z, stream);
}
void Replay::Encode(Quaternion* quaternion, ofstream& stream) {
    Encode(quaternion->x, stream);
    Encode(quaternion->y, stream);
    Encode(quaternion->z, stream);
    Encode(quaternion->w, stream);
}
void Replay::Encode(Frame* frame, ofstream& stream) {
    Encode(frame->time, stream);
    Encode(frame->fps, stream);
    Encode(frame->head->position, stream);
    Encode(frame->head->rotation, stream);
    Encode(frame->leftHand->position, stream);
    Encode(frame->leftHand->rotation, stream);
    Encode(frame->rightHand->position, stream);
    Encode(frame->rightHand->rotation, stream);
}
void Replay::Encode(NoteEvent* note, ofstream& stream) {
    Encode(note->noteID, stream);
    Encode(note->eventTime, stream);
    Encode(note->spawnTime, stream);
    Encode((int)note->eventType, stream);
    if (note->eventType == NoteEventType::good || note->eventType == NoteEventType::bad) {
        ReplayNoteCutInfo* info = note->noteCutInfo; 
        Encode(info->speedOK, stream);
        Encode(info->directionOK, stream);
        Encode(info->saberTypeOK, stream);
        Encode(info->wasCutTooSoon, stream);
        Encode(info->saberSpeed, stream);
        Encode(info->saberDir, stream);
        Encode(info->saberType, stream);
        Encode(info->timeDeviation, stream);
        Encode(info->cutDirDeviation, stream);
        Encode(info->cutPoint, stream);
        Encode(info->cutNormal, stream);
        Encode(info->cutDistanceToCenter, stream);
        Encode(info->cutAngle, stream);
        Encode(info->beforeCutRating, stream);
        Encode(info->afterCutRating, stream);
    }
}
void Replay::Encode(WallEvent* wall, ofstream& stream) {
    Encode(wall->wallID, stream);
    Encode(wall->energy, stream);
    Encode(wall->time, stream);
    Encode(wall->spawnTime, stream);
}
void Replay::Encode(AutomaticHeight* height, ofstream& stream) {
    Encode(height->height, stream);
    Encode(height->time, stream);
}
void Replay::Encode(Pause* pause, ofstream& stream) {
    Encode(pause->duration, stream);
    Encode(pause->time, stream);
}

Vector3::Vector3(UnityEngine::Vector3 vec) {
    x = vec.x;
    y = vec.y;
    z = vec.z;
}

Quaternion::Quaternion(UnityEngine::Quaternion quat) {
    x = quat.x;
    y = quat.y;
    z = quat.z;
    w = quat.w;
}

Replay::Replay() {}
Replay::~Replay() {}
ReplayInfo::ReplayInfo() {}
ReplayInfo::~ReplayInfo() {}
Frame::Frame() {}
Frame::~Frame() {}
Transform::Transform() {}
Transform::~Transform() {}
ReplayNoteCutInfo::ReplayNoteCutInfo() {}
ReplayNoteCutInfo::~ReplayNoteCutInfo() {}
NoteEvent::NoteEvent() {}
NoteEvent::~NoteEvent() {}
WallEvent::WallEvent() {}
WallEvent::~WallEvent() {}
AutomaticHeight::AutomaticHeight() {}
AutomaticHeight::~AutomaticHeight() {}
Pause::Pause() {}
Pause::~Pause() {}
Vector3::Vector3() {}
Vector3::~Vector3() {}
Quaternion::Quaternion() {}
Quaternion::~Quaternion() {}