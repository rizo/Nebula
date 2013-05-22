#pragma once

#include "../ProcessorState.hpp"
#include "../Simulation.hpp"

#include <atomic>
#include <chrono>

namespace sim {

namespace defaults {

constexpr std::chrono::microseconds PROCESSOR_TICK_DURATION { 10 };

}

class Processor : public Simulation<ProcessorState> {
    std::unique_ptr<ProcessorState> _proc { nullptr };
    std::chrono::microseconds _tickDuration;
public:
    explicit Processor( std::unique_ptr<ProcessorState>&& proc ) :
        _proc { std::move( proc ) },
        _tickDuration { defaults::PROCESSOR_TICK_DURATION } {}

    Processor() = delete;

    virtual std::unique_ptr<ProcessorState> run();
    
};

}
