#pragma once

#include "../Computer.hpp"
#include "../ProcessorState.hpp"
#include "../Simulation.hpp"

#include <atomic>
#include <chrono>

namespace sim {

namespace defaults {

constexpr std::chrono::microseconds PROCESSOR_TICK_DURATION { 10 };

}

}

class Processor : public Simulation<ProcessorState> {
    Computer& _computer;
    std::unique_ptr<ProcessorState> _proc { nullptr };
    std::chrono::microseconds _tickDuration;
public:
    explicit Processor( Computer& computer ) :
        _computer { computer },
        _proc { make_unique<ProcessorState>( computer.memory() ) },
        _tickDuration { sim::defaults::PROCESSOR_TICK_DURATION } {}

    Processor() = delete;

    virtual std::unique_ptr<ProcessorState> run();
};
