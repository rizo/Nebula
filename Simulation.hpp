#pragma once

#include "Fundamental.hpp"

#include <atomic>
#include <future>

template <typename StateType>
class Simulation {
private:
    std::atomic<bool> _isActive;
public:
    explicit Simulation() :
        _isActive { false } {}

    explicit Simulation( const Simulation& s ) :
        _isActive { s._isActive.load() } {}
    
    virtual std::unique_ptr<StateType> run() = 0;

    bool isActive() const { return _isActive.load(); }
    void setActive() { _isActive.store( true ); }
    
    virtual void stop() {
        _isActive.store( false );
    }

    virtual ~Simulation() {};
};

namespace sim {

template <typename StateType>
std::future<std::unique_ptr<StateType>>
launch( Simulation<StateType>& sim ) {
    return std::async( std::launch::async,
                       [&sim] { return sim.run(); } );
}

}
