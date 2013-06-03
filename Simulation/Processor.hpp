#pragma once

#include "../Computer.hpp"
#include "../ProcessorState.hpp"
#include "../Simulation.hpp"

#include <atomic>
#include <chrono>

namespace sim {

const std::chrono::microseconds PROCESSOR_TICK_DURATION { 10 };

}

class Processor : public Simulation<ProcessorState> {
    Computer& _computer;
    std::unique_ptr<ProcessorState> _proc { nullptr };
    std::chrono::microseconds _tickDuration;

    void handleInterrupt();
    void executeSpecial( const instruction::Unary* ins );
public:
    explicit Processor( Computer& computer ) :
        Simulation<ProcessorState> {},
        _computer( computer ),
        _proc { make_unique<ProcessorState>( computer.memory() ) },
        _tickDuration { sim::PROCESSOR_TICK_DURATION } {}

    Processor() = delete;
    Processor( const Processor& ) = delete;

    virtual std::unique_ptr<ProcessorState> run();
};
