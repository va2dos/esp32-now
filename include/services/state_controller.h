#pragma once
#include <functional>
#include <chrono>

namespace services
{
    enum class SystemState
    {
        Setup,
        Off,
        Idle,
        RfidDetected,
        Playing,
        RemotePlaying,
        Cooldown
    };

    class StateController
    {
    public:
        StateController();

        void setState(SystemState newState);
        SystemState getState() const;

        // Return elapsed time in seconds since entering the current state
        double getElapsedTime() const;

        // Event callbacks (set by services or main)
        std::function<void()> onOffEnter;
        std::function<void()> onIdleEnter;
        std::function<void()> onRfidDetected;
        std::function<void()> onPlayingEnter;
        std::function<void()> onRemotePlayingEnter;
        std::function<void()> onCooldownEnter;

    private:
        SystemState state;

        // For timing state durations, if needed
        using clock = std::chrono::steady_clock;
        clock::time_point stateStartTime;
        void resetStateTimer() { stateStartTime = clock::now(); }

        // Handle actions on state entry
        void onEnter(SystemState s);
    };
}