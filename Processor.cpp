#include "Processor.hpp"

#include <functional>

#include <boost/phoenix/core/argument.hpp>
#include <boost/phoenix/operator.hpp>

using boost::static_visitor;
using boost::apply_visitor;

Word Processor::read( Register reg ) const {
    return _registers[registerIndex( reg )];
}

Word Processor::read( Special spec ) const {
    switch ( spec ) {
    case Special::Pc: return _pc;
    case Special::Sp: return _sp;
    case Special::Ex: return _ex;
    }
}

Word Processor::read( Word loc) const {
    if ( loc >= _memory.size() ) {
        throw error::InvalidMemoryLocation { MemoryOperation::Read, loc };
    }

    return _memory[loc];
}

void Processor::write( Register reg, Word value ) {
    _registers[registerIndex( reg )] = value;
}

void Processor::write( Special spec, Word value ) {
    switch ( spec ) {
    case Special::Pc: _pc = value; return;
    case Special::Sp: _sp = value; return;
    case Special::Ex: _ex = value; return;
    }
}

void Processor::write( Word loc, Word value ) {
    if ( loc >= _memory.size() ) {
        throw error::InvalidMemoryLocation { MemoryOperation::Write, loc };
    }

    _memory[loc] = value;
}

static Word fetchNextWord( Processor& proc ) {
    return address::read( proc, address::direct() );
}

namespace address {

Address registerDirect( const Register& reg ) {
    Address addr;
    addr.type = AddressType::RegisterDirect;
    addr.reg = reg;
    return addr;
}

Address registerIndirect( const Register& reg ) {
    Address addr;
    addr.type = AddressType::RegisterIndirect;
    addr.reg = reg;
    return addr;
}

Address registerIndirectOffset( const Register& reg ) {
    Address addr;
    addr.type = AddressType::RegisterIndirectOffset;
    addr.reg = reg;
    return addr;
}

Address push() {
    Address addr;
    addr.type = AddressType::Push;
    return addr;
}

Address pop() {
    Address addr;
    addr.type = AddressType::Pop;
    return addr;
}

Address peek() {
    Address addr;
    addr.type = AddressType::Peek;
    return addr;
}

Address pick() {
    Address addr;
    addr.type = AddressType::Pick;
    return addr;
}

Address sp() {
    Address addr;
    addr.type = AddressType::Sp;
    return addr;
}

Address pc() {
    Address addr;
    addr.type = AddressType::Pc;
    return addr;
}

Address ex() {
    Address addr;
    addr.type = AddressType::Ex;
    return addr;
}

Address indirect() {
    Address addr;
    addr.type = AddressType::Indirect;
    return addr;
}

Address direct() {
    Address addr;
    addr.type = AddressType::Direct;
    return addr;
}

Address fastDirect( Word word ) {
    Address addr;
    addr.type = AddressType::FastDirect;
    addr.word = word;
    return addr;
}

bool isLiteral( const Address& addr ) {
    switch ( addr.type ) {
    case AddressType::FastDirect:
    case AddressType::Direct:
        return true;
    default:
        return false;
    }
}

using Location = variant<Word, Register, Special>;

static Location location( Processor& proc, const Address& addr ) {
    Word sp = proc.read( Special::Sp );
    Word pc = proc.read( Special::Pc );
    Word ex = proc.read( Special::Ex );

    switch ( addr.type ) {
    case AddressType::RegisterDirect: return addr.reg;
    case AddressType::RegisterIndirect: return proc.read( addr.reg );
    case AddressType::RegisterIndirectOffset:
        return fetchNextWord( proc ) + proc.read( addr.reg );
    case AddressType::Push:
        if ( sp == 0 ) {
            throw error::StackOverflow {};
        }

        proc.write( Special::Sp, sp - 1);
        return sp - 1;
    case AddressType::Pop:
        if ( sp >= proc.memorySize() - 1 ) {
            throw error::StackUnderflow {};
        }

        proc.write( Special::Sp, sp + 1 );
        return sp;
    case AddressType::Peek: return sp;
    case AddressType::Pick: return sp + fetchNextWord( proc );
    case AddressType::Sp: return sp;
    case AddressType::Pc: return pc;
    case AddressType::Ex: return ex;
    case AddressType::Indirect: return fetchNextWord( proc );
    case AddressType::Direct:
        proc.tickClock( 1 );
        proc.write( Special::Pc, pc + 1 );
        return pc;
    case AddressType::FastDirect: return addr.word;
    }
}

struct ReadVisitor : public static_visitor<Word> {
    Processor* proc = nullptr;

    ReadVisitor( Processor* proc ) :
        proc { proc } {}

    template <typename LocationType>
    Word operator()( const LocationType& loc ) const {
        return proc->read( loc );
    }
};

Word read( Processor& proc, const Address& addr ) {
    if ( addr.type == AddressType::FastDirect ) {
        return addr.word;
    } else {
        auto loc = location( proc, addr );
        return apply_visitor( ReadVisitor { &proc }, loc );
    }
}

struct WriteVisitor : public static_visitor<void> {
    Processor* proc = nullptr;
    Word value;

    WriteVisitor( Processor* proc, const Word& value ) :
        proc { proc },
        value { value } {}

    template <typename LocationType>
    void operator()( const LocationType& loc ) const {
        proc->write( loc, value );
    }
};

void write( Processor& proc, const Address& addr, Word value ) {
    if ( isLiteral( addr ) ) {
        // Nothing.
    } else {
        auto loc = location( proc, addr );
        apply_visitor( WriteVisitor { &proc, value }, loc );
    }
}

}

struct ExecuteVisitor : static_visitor<void> {
    Processor* proc = nullptr;

    explicit ExecuteVisitor( Processor* proc ) : proc { proc } {}

    template <typename InstructionType>
    void operator()( const InstructionType& ins ) const {
        alwaysExecute( *proc, ins );
    }
};

void execute( Processor& proc, const Instruction& ins ) {
    apply_visitor( ExecuteVisitor { &proc }, ins );
}

using BinaryFunction = std::function<Word (Word, Word)>;

void alwaysExecute( Processor& proc, const instruction::Binary& ins ) {
    using namespace boost::phoenix::placeholders;

    auto read = [&proc]( const Address& addr ) {
        return address::read( proc, addr );
    };

    auto write = [&proc]( const Address& addr, Word value ) {
        address::write( proc, addr, value );
    };

    auto apply = [&]( BinaryFunction f ) {
        auto x = read( ins.addressB );
        auto y = read( ins.addressA );

        write( ins.addressB, f( x, y ) );
    };

    switch ( ins.opcode ) {
    case Opcode::Set: write( ins.addressB, read( ins.addressA ) );
    case Opcode::Add: apply( arg1 + arg2 );
    case Opcode::Sub: apply( arg1 - arg2 );
    }
}

void alwaysExecute( Processor&, const instruction::Unary& ) {
    assert( false );
}

template <>
optional<Register> decode( const Word& word ) {
    switch ( word ) {
    case 0: return Register::A;
    case 1: return Register::B;
    case 2: return Register::C;
    case 3: return Register::X;
    case 4: return Register::Y;
    case 5: return Register::Z;
    case 6: return Register::I;
    case 7: return Register::J;
    default: return {};
    }
}

template <>
optional<Opcode> decode( const Word& word ) {
    switch ( word ) {
    case 0x01: return Opcode::Set;
    case 0x02: return Opcode::Add;
    case 0x03: return Opcode::Sub;
    default: return {};
    }
}

template <typename U, typename V>
static optional<V> map( const optional<U>& a, std::function<V ( const U& )> f ) {
    if ( a ) {
        return f( *a );
    } else {
        return {};
    }
}

optional<Address> decodeAddress( const Word& word, AddressContext context ) {
    optional<Address> addr;

    auto regDirect = [&word]() {
        return map<Register, Address>( decode<Register>( word ), address::registerDirect );
    };

    auto regIndirect = [&word]() {
        return map<Register, Address>( decode<Register>( word - 0x8 ), address::registerIndirect );
    };

    auto regIndirectOffset = [&word]() {
        return map<Register, Address>( decode<Register>( word - 0x10 ), address::registerIndirectOffset );
    };

    auto push = [&word, &context]() -> Address {
        if ( context == AddressContext::B && word == 0x18) {
            return address::push();
        } else {
            return {};
        }
    };

    auto pop = [&word, &context]() -> Address {
        if ( context == AddressContext::A && word == 0x18 ) {
            return address::pop();
        } else {
            return {};
        }
    };

#define VALUE( val, f ) [&word]() -> Address { if ( word == val ) return f(); else return {}; }

    auto peek = VALUE( 0x19, address::peek );
    auto pick = VALUE( 0x1a, address::pick );
    auto sp = VALUE( 0x1b, address::sp );
    auto pc = VALUE( 0x1c, address::pc );
    auto ex = VALUE( 0x1d, address::ex );
    auto indirect = VALUE( 0x1e, address::indirect );
    auto direct = VALUE( 0x1f, address::direct );

#undef VALUE

    auto fastDirect = [&word]() -> Address {
        if ( word >= 0x20 && word <= 0x3f ) {
            return address::fastDirect( word - 0x21 );
        } else {
            return {};
        }
    };

#define TRY( f ) addr = f(); if ( addr ) return addr;

    TRY( regDirect );
    TRY( regIndirect );
    TRY( regIndirectOffset );
    TRY( push );
    TRY( pop );
    TRY( peek );
    TRY( pick );
    TRY( sp );
    TRY( pc );
    TRY( ex );
    TRY( indirect );
    TRY( direct );
    TRY( fastDirect );
    
#undef TRY

    return {};
}
