// Simulation/Processor.hpp
//
// Copyright 2013 Jesse Haber-Kucharsky
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "../Computer.hpp"
#include "../ProcessorState.hpp"
#include "../Simulation.hpp"

#include <atomic>
#include <chrono>

DEFINE_LOGGER( PROC, "Processor" )

namespace nebula {

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

    Processor( const Processor& ) = delete;

    virtual std::unique_ptr<ProcessorState> run() override;
};

}
