#pragma once

#include "../Computer.hpp"
#include "../Simulation.hpp"

namespace sim {

const std::chrono::microseconds CLOCK_BASE_PERIOD { 16666 };

}

enum class ClockOperation {
    SetDivider,
    StoreElapsed,
    EnableInterrupts
};

struct ClockState {
    Word divider { 1 };
    bool isOn { false };
    bool interruptsEnabled { false };
    Word elapsed { 0 };
    Word message { 0 };
};

class Clock : public Simulation<ClockState>, public Device {
    Computer& _computer;
    std::shared_ptr<ProcessorInterrupt> _procInt { nullptr };
    ClockState _state {};
public:
    explicit Clock( Computer& computer ) :
        Simulation<ClockState> {},
        _computer( computer ),
        _procInt { computer.nextInterrupt( this ) } {}

    virtual std::unique_ptr<ClockState> run();

    virtual DeviceInfo info() const {
        return {
            device::Id { 0x12d0b402 },
            device::Manufacturer { 0 },
            device::Version { 1 }
        };
    }

    void handleInterrupt( ClockOperation op, ProcessorState* proc );
};
