#include <chrono>

#include "services/state_controller.h"

namespace services
{

    StateController::StateController()
        : state(SystemState::Setup)
    {
        
    }

    // ------------------------------------------------------------
    // Public API
    // ------------------------------------------------------------

    void StateController::setState(SystemState newState)
    {
        if (state == newState)
            return;
        
        state = newState;
        onEnter(state);
        resetStateTimer();
    }

    SystemState StateController::getState() const
    {
        return state;
    }

    double StateController::getElapsedTime() const
    {
        return std::chrono::duration<double>(clock::now() - stateStartTime).count();
    }   

    // ------------------------------------------------------------
    // Internal helpers
    // ------------------------------------------------------------

    void StateController::onEnter(SystemState s)
    {
        switch (s)
        {
        case SystemState::Setup:
            // Initialize system, then move to Idle
            // (Or let main.cpp call setState(Idle) when ready)
            break;

        case SystemState::Off:
            if (onOffEnter)
                onOffEnter();
            break;
            
        case SystemState::Idle:
            // Reset flags, prepare for next RFID scan
            if (onIdleEnter)
                onIdleEnter();
            break;

        case SystemState::RfidDetected:
            // Trigger sound + lights via callback
            if (onRfidDetected)
                onRfidDetected();
            break;

        case SystemState::Playing:
            // Start animations or track progress
            if (onPlayingEnter)
                onPlayingEnter();
            break;

        case SystemState::RemotePlaying:
            // Start animations or track progress for remote play
            if (onRemotePlayingEnter)
                onRemotePlayingEnter();
            break;

        case SystemState::Cooldown:
            // Start cooldown timer
            if (onCooldownEnter)
                onCooldownEnter();
            break;
        }
    }
} 
