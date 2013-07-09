// Simulation/Clock.hpp
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
#include "../Simulation.hpp"

DEFINE_LOGGER( CLOCK, "Clock" )

namespace nebula {

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

    void handleInterrupt( ClockOperation op, ProcessorState* proc );
public:
    explicit Clock( Computer& computer ) :
        Simulation<ClockState> {},
        _computer( computer ),
        _procInt { computer.nextInterrupt( this ) } {}

    virtual std::unique_ptr<ClockState> run();

    virtual DeviceInfo info() const noexcept override {
        return DeviceInfo {
            device::Id { 0x12d0b402 },
            device::Manufacturer { 0 },
            device::Version { 1 }
        };
    }
};

}
