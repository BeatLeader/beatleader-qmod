#pragma once

#include <array>
#include <atomic>
#include <condition_variable>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <thread>
#include <vector>

namespace Dance {

    struct DanceWall {
        float StartTime = 0, EndTime = 0, X0 = 0, X1 = 0, Y0 = 0, Y1 = 0;

        DanceWall() = default;
        DanceWall(float startTime, float endTime, float x0, float x1, float y0, float y1)
            : StartTime(startTime), EndTime(endTime), X0(x0), X1(x1), Y0(y0), Y1(y1) {}

        // out of line on purpose: DanceScore.cpp turns off FMA contraction
        static DanceWall FromGrid(float time, float duration, float lineIndex, float lineLayer, float width, float height);
    };

    struct DanceMeasures {
        double Tilt = 0;
        double BeatBounce = 0;
        double BeatLock = 0;
        double FreeSway = 0;
        double Sway = 0;
        double Steadiness = 0;
        double CountedFraction = 0;
    };

    struct DanceResult {
        bool Valid = false;
        double Score = 0;
        double VoteShare = 0;
        std::optional<DanceMeasures> Measures;
    };

    struct DanceLive {
        bool Ready = false;
        double Level = 0;
        double CountedFraction = 0;
        bool ShouldDrain = false;
        float SongTime = 0;
    };

    namespace Libm {
        double Atan2(double y, double x);
        double Cos(double x);
        double Sin(double x);
        double Exp(double x);
        double Log(double x);
    }

    class Workspace;

    struct GridData {
        std::span<double> T;
        std::array<std::span<double>, 10> Ch;
    };

    class FrameBuffer {
    public:
        std::vector<double> T;
        std::array<std::vector<double>, 10> Ch;
        int Count = 0;

        explicit FrameBuffer(int capacity);

        void Add(double t, double a, double b, double c, double d, double e, double f, double g, double h, double i, double j);
        void CopyTailTo(FrameBuffer& dst, double from) const;
        GridData Grid(double tStart, double tEnd, Workspace& w) const;

    private:
        void Grow(int capacity);
    };

    class Workspace {
    public:
        void Reset();
        std::span<double> D(int n);
        std::span<bool> B(int n);

    private:
        template <class TItem>
        struct Bucket {
            int Size = 0, Used = 0;
            std::vector<std::unique_ptr<TItem[]>> Items;
        };
        std::vector<Bucket<double>> doubles;
        std::vector<Bucket<bool>> bools;

        template <class TItem>
        static std::span<TItem> Rent(std::vector<Bucket<TItem>>& buckets, int n);
    };

    class DanceScorer {
    public:
        static constexpr int FormulaVersion = 3;
        static constexpr double SampleRate = 50;
        static constexpr double MinSeconds = 30;
        static constexpr int MinFrames = 500;
        static constexpr double EndSeconds = 2.5;
        static constexpr double LiveWindow = 8;
        static constexpr double LiveInterval = 1;
        static constexpr double LiveMinCounted = 0.5;
        static constexpr double SuggestedDrainPerSecond = 0.05;

        DanceScorer(std::span<const float> noteTimes, std::span<const DanceWall> walls, float songSpeed = 1.0f, float songLength = 0.0f);
        ~DanceScorer();
        DanceScorer(const DanceScorer&) = delete;
        DanceScorer& operator=(const DanceScorer&) = delete;

        bool LiveInBackground = true;
        bool LiveEnabled = true;

        DanceLive Live() const;

        void AddFrame(float songTime, float hx, float hy, float hz, float qx, float qy, float qz, float qw,
            float lx, float ly, float lz, float rx, float ry, float rz);
        void AddPause(float songTime);

        std::optional<DanceMeasures> WindowMeasures() const;
        DanceResult Finish() const;

    private:
        double speed;
        std::vector<DanceWall> walls;
        double gridPeriod;
        std::vector<double> pauses, jumps;
        FrameBuffer frames;
        double lastSongTime = -std::numeric_limits<double>::infinity();
        double lastLive = -std::numeric_limits<double>::infinity();

        mutable std::mutex liveLock;
        DanceLive live;
        std::atomic<bool> liveBusy{false};
        FrameBuffer liveSnapshot{4096};
        Workspace liveWork;
        std::vector<double> livePauses, liveJumps;
        double liveNow = 0;

        std::mutex workerLock;
        std::condition_variable workerWake;
        bool workerPending = false, workerStop = false;
        std::thread worker;

        void UpdateLive();
        void ComputeLive();
        void QueueLive();
        void WorkerLoop();
    };

}
