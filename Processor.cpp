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

template <>
optional<SpecialOpcode> decode( const Word& word ) {
    switch ( word ) {
    case 0x01: return SpecialOpcode::Jsr;
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

static optional<Address> decodeRegisterDirect( Word w ) {
    return map<Register, Address>( decode<Register>( w ), address::registerDirect );
}

static optional<Address> decodeRegisterIndirect( Word w ) {
    return map<Register, Address>( decode<Register>( w - 0x8 ), address::registerIndirect );
}

static optional<Address> decodeRegisterIndirectOffset( Word w ) {
    return map<Register, Address>( decode<Register>( w - 0x10 ), address::registerIndirectOffset );
}

static optional<Address> decodePush( Word w ) {
    if ( w == 0x18 ) {
        return address::push();
    } else {
        return {};
    }
}

static optional<Address> decodePop( Word w ) {
    if ( w == 0x18 ) {
        return address::pop();
    } else {
        return {};
    }
}

static optional<Address> decodeFastDirect( Word w ) {
    if ( w >= 0x20 && w <= 0x3f ) {
        if ( w == 0x20 ) {
            return address::fastDirect( 0xffff );
        } else {
            return address::fastDirect( w - 0x21 );
        }
    } else {
        return {};
    }
}

static
std::function<optional<Address>( Word )> 
makeDecoder( Word value,
             std::function<Address()> f ) {
    auto dec = [value, f]( Word w ) -> optional<Address> {
        if ( w == value ) {
            return f();
        } else {
            return {};
        }
    };

    return dec;
}

optional<Address> decodeAddress( const Word& word, AddressContext context ) {
    optional<Address> addr;

    auto decodePeek = makeDecoder( 0x19, address::peek );
    auto decodePick = makeDecoder( 0x1a, address::pick );
    auto decodeSp = makeDecoder( 0x1b, address::sp );
    auto decodePc = makeDecoder( 0x1c, address::pc );
    auto decodeEx = makeDecoder( 0x1d, address::ex );
    auto decodeIndirect = makeDecoder( 0x1e, address::indirect );
    auto decodeDirect = makeDecoder( 0x1f, address::direct );

#define TRY( f ) addr = f( word ); if ( addr ) return addr;

    if ( context == AddressContext::A ) {
        TRY( decodePop );
        TRY( decodeFastDirect );
    }

    TRY( decodeRegisterDirect );
    TRY( decodeRegisterIndirect );
    TRY( decodeRegisterIndirectOffset );
    TRY( decodePush );
    TRY( decodePop );
    TRY( decodePeek );
    TRY( decodePick );
    TRY( decodeSp );
    TRY( decodePc );
    TRY( decodeEx );
    TRY( decodeIndirect );
    TRY( decodeDirect );
    
#undef TRY

    return {};
}

template <>
optional<Instruction> decode( const Word& word ) {
    auto binary = [word]() -> optional<instruction::Binary> {
        auto opcode = decode<Opcode>( word & 0x1f );
        if ( ! opcode ) return {};

        auto addrA = decodeAddress( (word & 0xfc00) >> 10, AddressContext::A );
        if ( ! addrA ) return {};

        auto addrB = decodeAddress( (word & 0x3e0) >> 5, AddressContext::B );
        if ( ! addrB ) return {};

        return instruction::Binary { *opcode, *addrB, *addrA };
    };

    auto unary = [word]() -> optional<instruction::Unary> {
        auto opcode = decode<SpecialOpcode>( (word & 0x3e0) >> 5 );
        if ( ! opcode ) return {};

        auto addr = decodeAddress( (word & 0xfc00) >> 10, AddressContext::A );
        if ( ! addr ) return {};

        return instruction::Unary { *opcode, *addr };
    };

    auto binIns = binary();
    if ( binIns ) return Instruction { *binIns };

    auto unIns = unary();
    if ( unIns ) return Instruction { *unIns };

    return {};
}
