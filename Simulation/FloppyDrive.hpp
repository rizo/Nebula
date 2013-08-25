#pragma once

#include "../Computer.hpp"
#include "../Simulation.hpp"

#include <bitset>

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

using Sector = std::vector<Word>;
using Track = std::vector<Sector>;
using Disk = std::vector<Track>;

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
    bool interruptsEnabled { false };
    Word message { 0 };

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

    // The results of an asynchronous read or write operation.
    bool _isReading { false };
    std::future<void> _readF;

    bool _isWriting { false };
    std::future<std::pair<Word, Sector>> _writeF;

    void sendInterruptIfEnabled();
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
