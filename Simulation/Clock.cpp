#include "Clock.hpp"

#include <iostream>

namespace sim {

std::unique_ptr<void>
Clock::run() {
    setActive();

    while ( isActive() ) {
        if ( _procInt->isActive() ) {
            std::cout << "Clock got interrupt!" << std::endl;
            
            _procInt->respond();
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }

    return nullptr;
}

}
