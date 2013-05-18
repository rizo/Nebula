#ifndef __PROCESSOR_HPP__
#define __PROCESSOR_HPP__

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include <boost/format.hpp>
#include <boost/optional.hpp>

using Word = std::uint16_t;
using DoubleWord = std::uint32_t;
using SignedWord = std::int16_t;
using SignedDoubleWord = std::uint32_t;

using boost::format;
using boost::optional;

enum class Register {
    A, B, C,
    X, Y, Z,
    I, J
};

enum class Special {
    Pc, Sp, Ex
};

using Memory = std::vector<Word>;

constexpr Word STACK_BEGIN = 0xffff;

enum class MemoryOperation { Read, Write };

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

class InvalidMemoryLocation : public std::out_of_range {
    MemoryOperation _operation;
    Word _location;
public:
    explicit InvalidMemoryLocation( MemoryOperation operation, Word location ) :
        std::out_of_range {
            (format( "Cannot %s at memory location 0x%04x" )
                 % (operation == MemoryOperation::Read ? "read" : "write")
                 % location ).str()
        },
        _operation { operation },
        _location { location } {
    }

    inline MemoryOperation operation() const { return _operation; }
    inline Word location() const { return _location; }
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

class Processor {
    Memory _memory;
    std::array<Word, 8> _registers;
    Word _pc;
    Word _sp;
    Word _ex;
    int _clock;

    using RegisterIndex = decltype( _registers )::size_type;
    static inline RegisterIndex registerIndex( Register reg ) {
        return static_cast<RegisterIndex>( reg );
    }

public:
    explicit Processor( int memorySize ) :
        _memory { Memory( memorySize ) },
        _registers { { 0, 0, 0, 0, 0, 0, 0, 0 } },
        _pc { 0 },
        _sp { STACK_BEGIN },
        _ex { 0 },
        _clock { 0 } {
    }

    Word read( Register reg ) const;
    Word read( Special spec ) const;
    Word read( Word loc ) const;

    void write( Register reg, Word val );
    void write( Special spec, Word val );
    void write( Word loc, Word val );

    inline int clock() const { return _clock; }
    inline void tickClock( int ticks ) { _clock += ticks; }

    inline int memorySize() const { return _memory.size(); }
};

class AddressingMode {
public:
    virtual Word load( Processor& proc ) const = 0;
    virtual void store( Processor& proc, Word value ) const = 0;
};

namespace mode {

struct RegisterDirect : public AddressingMode {
    Register reg;

    explicit RegisterDirect( Register reg ) : reg { reg } {}

    virtual Word load( Processor& proc ) const { return proc.read( reg ); }
    virtual void store( Processor& proc, Word value ) const { proc.write( reg, value ); }
};

}

enum class Opcode {
    Set,
    Add,
    Sub,
};

enum class SpecialOpcode {
    Jsr
};

class Instruction {
    virtual void execute( Processor& proc ) const = 0;
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

    virtual void execute( Processor& proc ) const;
};

}

void executeNext( Processor& proc );

#endif // __PROCESSOR_HPP__
