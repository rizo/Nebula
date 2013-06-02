#pragma once

#include "Memory.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>

#include <boost/format.hpp>

using boost::format;

enum class Register {
    A, B, C,
    X, Y, Z,
    I, J
};

enum class Special {
    Pc, Sp, Ex
};

constexpr Word STACK_BEGIN = 0xffff;

namespace error {

class StackOverflow : public std::out_of_range {
public:
    explicit StackOverflow() :
        std::out_of_range {
            (format( "Stack overflow (size 0x04%x)" ) % STACK_BEGIN).str()
        } {
    }
};

class StackUnderflow : public std::out_of_range {
public:
    explicit StackUnderflow() :
        std::out_of_range {
            (format( "Stack underflow (size 0x04%x)" ) % STACK_BEGIN).str()
        } {
    }
};

class MalformedInstruction : public std::out_of_range {
    Word _word;
public:
    explicit MalformedInstruction( Word word ) :
        std::out_of_range {
            (format( "Malformed instruction 0x%04x" ) % word ).str()
        },
        _word { word } {
    }

    inline Word word() const { return _word; }
};

}

// Forward declaration.
class Instruction;

class ProcessorState {
    std::array<Word, 8> _registers;
    Word _pc;
    Word _sp;
    Word _ex;

    int _clock;
    std::shared_ptr<Instruction> _lastInstruction { nullptr };
    bool _doSkip { false };

    std::shared_ptr<Memory> _memory = nullptr;

    using RegisterIndex = decltype( _registers )::size_type;
    static inline RegisterIndex registerIndex( Register reg ) {
        return static_cast<RegisterIndex>( reg );
    }

public:
    ProcessorState() = delete;

    explicit ProcessorState( std::shared_ptr<Memory> memory ) :
        _registers { { 0, 0, 0, 0, 0, 0, 0, 0 } },
        _pc { 0 },
        _sp { STACK_BEGIN },
        _ex { 0 },
        _clock { 0 },
        _memory { memory } {}

    Word read( Register reg ) const;
    Word read( Special spec ) const;

    void write( Register reg, Word val );
    void write( Special spec, Word val );

    inline int clock() const { return _clock; }
    inline void tickClock( int ticks ) { _clock += ticks; }
    inline void clearClock() { _clock = 0; }

    inline bool doSkip() const { return _doSkip; }
    void setSkip( bool value ) { _doSkip = value; }

    inline Memory& memory() { return *_memory; }

    const Instruction* lastInstruction() const { return _lastInstruction.get(); }

    void executeNext();
};

class AddressingMode {
public:
    virtual Word load( ProcessorState& proc ) const = 0;
    virtual void store( ProcessorState& proc, Word value ) const = 0;
    
    virtual int size() const { return 0; }
};

namespace mode {

struct RegisterDirect : public AddressingMode {
    Register reg;

    explicit RegisterDirect( Register reg ) : reg { reg } {}
    RegisterDirect() = delete;

    virtual Word load( ProcessorState& proc ) const { return proc.read( reg ); }
    virtual void store( ProcessorState& proc, Word value ) const { proc.write( reg, value ); }
};

struct RegisterIndirect : public AddressingMode {
    Register reg;

    explicit RegisterIndirect( Register reg ) : reg { reg } {}
    RegisterIndirect() = delete;

    virtual Word load( ProcessorState& proc ) const {
        return proc.memory().read( proc.read( reg ) );
    }

    virtual void store( ProcessorState& proc, Word value ) const {
        proc.memory().write( proc.read( reg ), value );
    }
};

struct RegisterIndirectOffset : public AddressingMode {
    Register reg;

    explicit RegisterIndirectOffset( Register reg ) : reg { reg } {}
    RegisterIndirectOffset() = delete;

    virtual Word load( ProcessorState& proc ) const;
    virtual void store( ProcessorState& proc, Word value ) const;
};

struct Push : public AddressingMode {
    virtual Word load( ProcessorState& ) const {
        assert( ! "Attempt to load from a 'push' address!" );
    }

    virtual void store( ProcessorState& proc, Word value ) const {
        auto sp = proc.read( Special::Sp );
        proc.memory().write( sp - 1, value );
        proc.write( Special::Sp, sp - 1 );
    }
};

struct Pop : public AddressingMode {
    virtual Word load( ProcessorState& proc ) const {
        auto sp = proc.read( Special::Sp );
        auto word = proc.memory().read( sp );
        proc.write( Special::Sp, sp + 1 );
        return word;
    }

    virtual void store( ProcessorState&, Word ) const {
        assert( ! "Attempt to store to a 'pop' address!" );
    }
};

struct Peek : public AddressingMode {
    virtual Word load( ProcessorState& proc ) const {
        return proc.memory().read( proc.read( Special::Sp ) );
    }

    virtual void store( ProcessorState& proc, Word value ) const {
        proc.memory().write( proc.read( Special::Sp ), value );
    }
};

struct Pc : public AddressingMode {
    virtual Word load( ProcessorState& proc ) const {
        return proc.read( Special::Pc );
    }

    virtual void store( ProcessorState& proc, Word value ) const {
        proc.write( Special::Pc, value );
    }
};

struct Indirect : public AddressingMode {
    virtual Word load( ProcessorState& proc ) const;
    virtual void store( ProcessorState& proc, Word value ) const;
};

struct Direct : public AddressingMode {
    virtual Word load( ProcessorState& proc ) const;
    virtual void store( ProcessorState& proc, Word value ) const;

    virtual int size() const { return 1; }
};

struct FastDirect : public AddressingMode {
    Word value;

    explicit FastDirect( Word value ) : value { value } {}
    FastDirect() = delete;

    virtual Word load( ProcessorState& ) const { return value; }
    virtual void store( ProcessorState&, Word ) const { /* Nothing. */ }
};

}

enum class Opcode {
    Set,
    Add,
    Sub,
    Bor,
    Ife
};

enum class SpecialOpcode {
    Jsr,
    Iag,
    Ias,
    Rfi,
    Iaq,
    Hwn,
    Hwq,
    Hwi
};

class Instruction {
public:
    virtual void execute( ProcessorState& proc ) const = 0;
    virtual int size() const = 0;
};

namespace instruction {

struct Unary : public Instruction {
    SpecialOpcode opcode;
    std::shared_ptr<AddressingMode> address = nullptr;

    explicit Unary( SpecialOpcode opcode,
                    std::shared_ptr<AddressingMode> address ) :
        opcode { opcode },
        address { address } {
    }

    Unary() = delete;

    virtual void execute( ProcessorState& proc ) const;

    virtual int size() const { return address->size(); }
};

struct Binary : public Instruction {
    Opcode opcode;
    std::shared_ptr<AddressingMode> addressB = nullptr;
    std::shared_ptr<AddressingMode> addressA = nullptr;

    explicit Binary( Opcode opcode,
                     std::shared_ptr<AddressingMode> addressB,
                     std::shared_ptr<AddressingMode> addressA ) :
        opcode { opcode },
        addressB { addressB },
        addressA { addressA } {
    }

    Binary() = delete;

    virtual void execute( ProcessorState& proc ) const;
    
    virtual int size() const { return addressA->size() + addressB->size(); }
};

}

void advance( ProcessorState& proc, int numWords );

enum class AddressContext { A, B };

template <typename T>
optional<T> decode( const Word& ) {
    return {};
}

std::shared_ptr<AddressingMode> decodeAddress( AddressContext context, Word word );
std::shared_ptr<Instruction> decodeInstruction( Word word );
