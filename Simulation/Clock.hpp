#pragma once

#include "../Computer.hpp"
#include "../Simulation.hpp"

class Clock : public Simulation<void>, public Device {
    Computer& _computer;
    std::shared_ptr<ProcessorInterrupt> _procInt { nullptr };
public:
    explicit Clock( Computer& computer ) :
        _computer { computer },
        _procInt { computer.nextInterrupt( *this) } {}

    virtual std::unique_ptr<void> run();

    virtual DeviceId id() const { return 0x12d0b402; }
};
