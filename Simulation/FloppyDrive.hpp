#pragma once

#include "../Computer.hpp"
#include "../Simulation.hpp"

namespace nebula {

namespace sim {

const int FLOPPY_WORDS_PER_SECTOR = 512;
const int FLOPPY_SECTORS_PER_TRACK = 18;
const int FLOPPY_TRACKS_PER_DISK = 80;

const std::chrono::microseconds FLOPPY_TRACK_SEEK_DURATION { 2400 };
const std::chrono::microseconds FLOPPY_WORD_ACCESS_DURATION { 33 };

// FloppyDrive runs at roughly 10 kHz.
const std::chrono::microseconds FLOPPY_SLEEP_DURATION { 100 };

}

using Sector = std::array<Word, sim::FLOPPY_WORDS_PER_SECTOR>;
using Track = std::array<Sector, sim::FLOPPY_SECTORS_PER_TRACK>;
using Disk = std::array<Track, sim::FLOPPY_TRACKS_PER_DISK>;

enum class FloppyDriveStateCode : Word {
    NoMedia = 0,
    Ready = 1,
    ReadyWP = 2,
    Busy = 3
};

enum class FloppyDriveErrorCode : Word {
    None = 0,
    Busy = 1,
    NoMedia = 2,
    Protected = 3,
    Eject = 4,
    BadSector = 5,
    Broken = 0xffff
};

struct FloppyDriveState {
    Disk disk;
    FloppyDriveStateCode stateCode { FloppyDriveStateCode::Ready };
    FloppyDriveErrorCode errorCode { FloppyDriveErrorCode::None };

    explicit FloppyDriveState();

    Sector& getSector( Word index );
};

enum class FloppyDriveOperation {
    Poll,
    EnableInterrupts,
    Read,
    Write
};

class FloppyDrive : public Simulation<FloppyDriveState>, public Device {
    Computer& _computer;
    std::shared_ptr<ProcessorInterrupt> _procInt { nullptr };
    FloppyDriveState _state;
    std::shared_ptr<Memory> _memory { nullptr };

    std::future<void> _resultF;

    void readToMemory( const Sector& sector, Word loc );
    void writeFromMemory( Sector& sector, Word loc );

    void handleInterrupt( FloppyDriveOperation op, ProcessorState* proc );
public:
    explicit FloppyDrive( Computer& computer );

    virtual std::unique_ptr<FloppyDriveState> run() override;
    
    inline virtual DeviceInfo info() const noexcept override {
        return DeviceInfo {
            device::Id { 0x4fd524c5 },
            device::Manufacturer { 0x1eb37e91 },
            device::Version { 0x000b }
        };
    }
};

}
