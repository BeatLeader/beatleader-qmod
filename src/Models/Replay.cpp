#include "include/Models/Replay.hpp"
#include "include/main.hpp"
#include <fstream>

enum struct StructType {
    info = 0,
    frames = 1,
    notes = 2,
    walls = 3,
    heights = 4,
    pauses = 5
};

void Replay::Encode(ofstream& s) const {
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
    const char* cstring = value.data();
    size_t stringLength = value.size();
    Encode((int)stringLength, stream);
    stream.write(cstring, stringLength);
}

void Replay::Encode(ReplayInfo const &info, ofstream& stream) {
    Encode(info.version, stream);
    Encode(info.gameVersion, stream);
    Encode(info.timestamp, stream);

    Encode(info.playerID, stream);
    Encode(info.playerName, stream);
    Encode(info.platform, stream);

    Encode(info.trackingSytem, stream);
    Encode(info.hmd, stream);
    Encode(info.controller, stream);

    Encode(info.hash, stream);
    Encode(info.songName, stream);
    Encode(info.mapper, stream);
    Encode(info.difficulty, stream);

    Encode(info.score, stream);
    Encode(info.mode, stream);
    Encode(info.environment, stream);
    Encode(info.modifiers, stream);
    Encode(info.jumpDistance, stream);
    Encode(info.leftHanded, stream);
    Encode(info.height, stream);

    Encode(info.startTime, stream);
    Encode(info.failTime, stream);
    Encode(info.speed, stream);
}
void Replay::Encode(Sombrero::FastVector3 const &vector, ofstream& stream) {
    Encode(vector.x, stream);
    Encode(vector.y, stream);
    Encode(vector.z, stream);
}
void Replay::Encode(Sombrero::FastQuaternion const &quaternion, ofstream& stream) {
    Encode(quaternion.x, stream);
    Encode(quaternion.y, stream);
    Encode(quaternion.z, stream);
    Encode(quaternion.w, stream);
}
void Replay::Encode(Frame const &frame, ofstream& stream) {
    Encode(frame.time, stream);
    Encode(frame.fps, stream);
    Encode(frame.head.position, stream);
    Encode(frame.head.rotation, stream);
    Encode(frame.leftHand.position, stream);
    Encode(frame.leftHand.rotation, stream);
    Encode(frame.rightHand.position, stream);
    Encode(frame.rightHand.rotation, stream);
}
void Replay::Encode(NoteEvent const &note, ofstream& stream) {
    Encode(note.noteID, stream);
    Encode(note.eventTime, stream);
    Encode(note.spawnTime, stream);
    Encode((int)note.eventType, stream);
    if (note.eventType == NoteEventType::GOOD || note.eventType == NoteEventType::BAD) {
        ReplayNoteCutInfo const& info = note.noteCutInfo;
        Encode(info.speedOK, stream);
        Encode(info.directionOK, stream);
        Encode(info.saberTypeOK, stream);
        Encode(info.wasCutTooSoon, stream);
        Encode(info.saberSpeed, stream);
        Encode(info.saberDir, stream);
        Encode(info.saberType, stream);
        Encode(info.timeDeviation, stream);
        Encode(info.cutDirDeviation, stream);
        Encode(info.cutPoint, stream);
        Encode(info.cutNormal, stream);
        Encode(info.cutDistanceToCenter, stream);
        Encode(info.cutAngle, stream);
        Encode(info.beforeCutRating, stream);
        Encode(info.afterCutRating, stream);
    }
}
void Replay::Encode(WallEvent const &wall, ofstream& stream) {
    Encode(wall.wallID, stream);
    Encode(wall.energy, stream);
    Encode(wall.time, stream);
    Encode(wall.spawnTime, stream);
}
void Replay::Encode(AutomaticHeight const &height, ofstream& stream) {
    Encode(height.height, stream);
    Encode(height.time, stream);
}
void Replay::Encode(Pause const &pause, ofstream& stream) {
    Encode(pause.duration, stream);
    Encode(pause.time, stream);
}


char Replay::DecodeChar(ifstream& stream) {
    char value;
    stream.read(reinterpret_cast<char *>(&value), sizeof(char));
    return value;
}
int Replay::DecodeInt(ifstream& stream) {
    int value;
    stream.read(reinterpret_cast<char *>(&value), sizeof(int));
    return value;
}
long Replay::DecodeLong(ifstream& stream) {
    long value;
    stream.read(reinterpret_cast<char *>(&value), sizeof(long));
    return value;
}
bool Replay::DecodeBool(ifstream& stream) {
    bool value;
    stream.read(reinterpret_cast<char *>(&value), sizeof(char));
    return value;
}
float Replay::DecodeFloat(ifstream& stream) {
    float value;
    stream.read(reinterpret_cast<char *>(&value), sizeof(int));
    return value;
}
string Replay::DecodeString(ifstream& stream) {
    size_t stringLength = (size_t)DecodeInt(stream);
    std::string cstring;
    cstring.resize(stringLength);
    
    stream.read(cstring.data(), stringLength);

    return cstring;
}

std::optional<ReplayInfo> Replay::DecodeInfo(ifstream& stream) {
    // TODO: Make a constant
    if (DecodeInt(stream) != 0x442d3d69 || DecodeChar(stream) != 1 || DecodeChar(stream) != 0) {
        return std::nullopt;
    }

    auto version = DecodeString(stream);
    auto gameVersion = DecodeString(stream);
    auto timestamp = DecodeString(stream);
    
    ReplayInfo info(version, gameVersion, timestamp);
    
    info.playerID = DecodeString(stream);
    info.playerName = DecodeString(stream);
    info.platform = DecodeString(stream);

    info.trackingSytem = DecodeString(stream);
    info.hmd = DecodeString(stream);
    info.controller = DecodeString(stream);

    info.hash = DecodeString(stream);
    info.songName = DecodeString(stream);
    info.mapper = DecodeString(stream);
    info.difficulty = DecodeString(stream);

    info.score = DecodeInt(stream);
    info.mode = DecodeString(stream);
    info.environment = DecodeString(stream);
    info.modifiers = DecodeString(stream);
    info.jumpDistance = DecodeFloat(stream);
    info.leftHanded = DecodeBool(stream);
    info.height = DecodeFloat(stream);

    info.startTime = DecodeFloat(stream);
    info.failTime = DecodeFloat(stream);
    info.speed = DecodeFloat(stream);
    return info;
}