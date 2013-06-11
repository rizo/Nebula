#pragma once

#include "Memory.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>

DEFINE_LOGGER( PSTATE, "ProcessorState" )

namespace nebula {

enum class Register {
    A, B, C,
    X, Y, Z,
    I, J
};

enum class Special {
    Pc, Sp, Ex
};

namespace processor {

const Word STACK_BEGIN = 0xffff;

}

namespace error {

class StackOverflow : public std::out_of_range {
public:
    explicit StackOverflow() :
        std::out_of_range {
            (format( "Stack overflow (size 0x04%x)" ) % processor::STACK_BEGIN).str()
        } {
    }
};

class StackUnderflow : public std::out_of_range {
public:
    explicit StackUnderflow() :
        std::out_of_range {
            (format( "Stack underflow (size 0x04%x)" ) % processor::STACK_BEGIN).str()
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

    inline Word word() const noexcept { return _word; }
};

}

// Forward declaration.
class Instruction;

class ProcessorState final {
    std::array<Word, 8> _registers;
    Word _pc;
    Word _sp;
    Word _ex;

    int _clock;
    std::shared_ptr<Instruction> _lastInstruction { nullptr };
    bool _doSkip { false };

    std::shared_ptr<Memory> _memory = nullptr;

    using RegisterIndex = decltype( _registers )::size_type;
    static inline RegisterIndex registerIndex( Register reg ) noexcept {
        return static_cast<RegisterIndex>( reg );
    }

public:
    explicit ProcessorState( std::shared_ptr<Memory> memory ) :
        _registers { { 0, 0, 0, 0, 0, 0, 0, 0 } },
        _pc { 0 },
        _sp { processor::STACK_BEGIN },
        _ex { 0 },
        _clock { 0 },
        _memory { memory } {}

    Word read( Register reg ) const noexcept;
    Word read( Special spec ) const noexcept;

    void write( Register reg, Word val ) noexcept;
    void write( Special spec, Word val ) noexcept;

    inline int clock() const noexcept { return _clock; }
    inline void tickClock( int ticks ) noexcept { _clock += ticks; }
    inline void clearClock() noexcept { _clock = 0; }

    inline bool doSkip() const noexcept { return _doSkip; }
    void setSkip( bool value ) noexcept { _doSkip = value; }

    inline Memory* memory() noexcept { return _memory.get(); }

    const Instruction* lastInstruction() const noexcept { return _lastInstruction.get(); }

    void executeNext();
};

class AddressingMode {
public:
    virtual Word load( ProcessorState& proc ) = 0;
    virtual void store( ProcessorState& proc, Word value ) = 0;
    
    virtual int size() const noexcept { return 0; }

    virtual ~AddressingMode() = default;
};

class LongAddressingMode : public AddressingMode {
    optional<Word> _next {};
public:
    explicit LongAddressingMode() : AddressingMode {} {}

    Word next( ProcessorState& proc );

    virtual int size() const noexcept override { return 1; };

    virtual ~LongAddressingMode() = default;
};

namespace mode {

struct RegisterDirect final : public AddressingMode {
    Register reg;

    explicit RegisterDirect( Register reg ) : reg { reg } {}

    virtual Word load( ProcessorState& proc ) override { return proc.read( reg ); }
    virtual void store( ProcessorState& proc, Word value ) override { proc.write( reg, value ); }
};

struct RegisterIndirect final : public AddressingMode {
    Register reg;

    explicit RegisterIndirect( Register reg ) : reg { reg } {}

    inline virtual Word load( ProcessorState& proc ) override {
        return proc.memory()->read( proc.read( reg ) );
    }

    inline virtual void store( ProcessorState& proc, Word value ) override {
        proc.memory()->write( proc.read( reg ), value );
    }
};

struct RegisterIndirectOffset final : public LongAddressingMode {
    Register reg;

    explicit RegisterIndirectOffset( Register reg ) :
        LongAddressingMode {},
        reg { reg } {}

    virtual Word load( ProcessorState& proc ) override;
    virtual void store( ProcessorState& proc, Word value ) override;
};

struct Push final : public AddressingMode {
    inline virtual Word load( ProcessorState& ) override {
        assert( ! "Attempt to load from a 'push' address!" );
        return 0;
    }

    virtual void store( ProcessorState& proc, Word value ) override;
};

struct Pop final : public AddressingMode {
    virtual Word load( ProcessorState& proc ) override;

    virtual void store( ProcessorState&, Word ) override {
        assert( ! "Attempt to store to a 'pop' address!" );
    }
};

struct Peek final : public AddressingMode {
    inline virtual Word load( ProcessorState& proc ) override {
        return proc.memory()->read( proc.read( Special::Sp ) );
    }

    inline virtual void store( ProcessorState& proc, Word value ) override {
        proc.memory()->write( proc.read( Special::Sp ), value );
    }
};

struct Pick final : public LongAddressingMode {
    explicit Pick() : LongAddressingMode {} {}

    virtual Word load( ProcessorState& proc ) override;
    virtual void store( ProcessorState& proc, Word value ) override;
};

struct Pc final : public AddressingMode {
    virtual Word load( ProcessorState& proc ) override {
        return proc.read( Special::Pc );
    }

    virtual void store( ProcessorState& proc, Word value ) override {
        proc.write( Special::Pc, value );
    }
};

struct Indirect final : public LongAddressingMode {
    explicit Indirect() : LongAddressingMode {} {}

    virtual Word load( ProcessorState& proc ) override;
    virtual void store( ProcessorState& proc, Word value ) override;
};

struct Direct final : public LongAddressingMode {
    explicit Direct() : LongAddressingMode {} {}

    virtual Word load( ProcessorState& proc ) override;
    virtual void store( ProcessorState& proc, Word value ) override;
};

struct FastDirect final : public AddressingMode {
    Word value;

    explicit FastDirect( Word value ) : value { value } {}
    FastDirect() = delete;

    inline virtual Word load( ProcessorState& ) override { return value; }
    inline virtual void store( ProcessorState&, Word ) override { /* Nothing. */ }
};

}

enum class Opcode {
    Set,
    Add,
    Sub,
    Bor,
    Ife,
    Ifn,
    Ifg,
    Ifl
};

enum class SpecialOpcode {
    Jsr,
    Int,
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
    virtual int size() const noexcept= 0;
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

    virtual void execute( ProcessorState& proc ) const override;

    inline virtual int size() const noexcept override { return address->size(); }
};

struct Binary : public Instruction {
    Opcode opcode;
    std::shared_ptr<AddressingMode> addressB { nullptr };
    std::shared_ptr<AddressingMode> addressA { nullptr };

    explicit Binary( Opcode opcode,
                     std::shared_ptr<AddressingMode> addressB,
                     std::shared_ptr<AddressingMode> addressA ) :
        opcode { opcode },
        addressB { addressB },
        addressA { addressA } {
    }

    virtual void execute( ProcessorState& proc ) const override;
    
    inline virtual int size() const noexcept override { return addressA->size() + addressB->size(); }
};

}

void advance( ProcessorState& proc, int numWords );
void dumpToLog( ProcessorState& proc );

enum class AddressContext { A, B };

template <typename T>
optional<T> decode( const Word& ) {
    return {};
}

std::shared_ptr<AddressingMode> decodeAddress( AddressContext context, Word word );
std::shared_ptr<Instruction> decodeInstruction( Word word );

}
