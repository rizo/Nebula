#ifndef __PROCESSOR_HPP__
#define __PROCESSOR_HPP__

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

using Word = std::uint16_t;
using DoubleWord = std::uint32_t;
using SignedWord = std::int16_t;
using SignedDoubleWord = std::uint32_t;

using boost::format;
using boost::optional;
using boost::variant;

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

enum class AddressType {
    RegisterDirect,
    RegisterIndirect,
    RegisterIndirectOffset,
    Push,
    Pop,
    Peek,
    Pick,
    Sp,
    Pc,
    Ex,
    Indirect,
    Direct,
    FastDirect
};

struct Address {
    AddressType type;

    union {
        Word word;
        Register reg;
        Special spec;
    };
};

namespace address {

Address registerDirect( const Register& reg );
Address registerIndirect( const Register& reg );
Address registerIndirectOffset( const Register& reg );
Address push();
Address pop();
Address peek();
Address pick();
Address sp();
Address pc();
Address ex();
Address indirect();
Address direct();
Address fastDirect( Word word );

bool isLiteral( const Address& addr );

Word read( Processor& proc, const Address& addr );
void write( Processor& proc, const Address& addr, Word value );

}

enum class Opcode {
    Set,
    Add,
    Sub,
};

enum class SpecialOpcode {
    Jsr
};

namespace instruction {

struct Unary {
    SpecialOpcode opcode;
    Address address;
};

struct Binary {
    Opcode opcode;
    Address addressB;
    Address addressA;
};

}

using Instruction = variant<instruction::Unary, instruction::Binary>;

template <typename T>
optional<T> decode( const Word& ) { return {}; }

enum class AddressContext { A, B };
optional<Address> decodeAddress( const Word& word, AddressContext context );

void execute( Processor& proc, const Instruction& ins );
void executeNext( Processor& proc );

#endif // __PROCESSOR_HPP__
