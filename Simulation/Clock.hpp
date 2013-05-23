#pragma once

#include "../Computer.hpp"
#include "../Simulation.hpp"

namespace sim {

namespace defaults {

constexpr std::chrono::microseconds CLOCK_BASE_PERIOD { 16666 };

}

}

enum class ClockOperation {
    SetDivider,
    StoreElapsed,
    EnableInterrupts
};

struct ClockState {
    Word divider { 1 };
    bool isOn { false };
    Word elapsed { 0 };
};

class Clock : public Simulation<ClockState>, public Device {
    Computer& _computer;
    std::shared_ptr<ProcessorInterrupt> _procInt { nullptr };
    ClockState _state {};
public:
    explicit Clock( Computer& computer ) :
        _computer { computer },
        _procInt { computer.nextInterrupt( this) } {}

    virtual std::unique_ptr<ClockState> run();

    virtual DeviceId id() const { return 0x12d0b402; }

    void handleInterrupt( ClockOperation op, ProcessorState* proc );
};
