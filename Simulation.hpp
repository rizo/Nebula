// Simulation.hpp
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

#include "Fundamental.hpp"

#include <atomic>
#include <future>

namespace nebula {

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

    inline bool isActive() const noexcept { return _isActive.load(); }
    inline void setActive() noexcept { _isActive.store( true ); }
    
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

template <typename StateType>
bool isReady( std::future<std::unique_ptr<StateType>>& f ) {
    return f.wait_for( std::chrono::seconds { 0 } ) == std::future_status::ready;
}

}

}
