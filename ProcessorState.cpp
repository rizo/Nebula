#include "ProcessorState.hpp"

#include <functional>

#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>

namespace nebula {

Word ProcessorState::read( Register reg ) const noexcept {
    return _registers[registerIndex( reg )];
}

Word ProcessorState::read( Special spec ) const noexcept {
    switch ( spec ) {
    case Special::Pc: return _pc;
    case Special::Sp: return _sp;
    case Special::Ex: return _ex;
    }

    // This shouldn't be necessary, but GCC complains (wrongly) that
    // not all cases are handled in the above.
    return 0;
}

void ProcessorState::write( Register reg, Word value ) noexcept {
    _registers[registerIndex( reg )] = value;
}

void ProcessorState::write( Special spec, Word value ) noexcept {
    switch ( spec ) {
    case Special::Pc: _pc = value; return;
    case Special::Sp: _sp = value; return;
    case Special::Ex: _ex = value; return;
    }
}

static Word fetchNextWord( ProcessorState& proc ) {
    proc.tickClock( 1 );

    auto pc = proc.read( Special::Pc );
    proc.write( Special::Pc, pc + 1 );
    return proc.memory().read( pc );
}

Word LongAddressingMode::next( ProcessorState& proc ) {
    if ( ! _next ) {
        _next = fetchNextWord( proc );
    }

    return *_next;
}

namespace mode {

Word RegisterIndirectOffset::load( ProcessorState& proc ) {
    return proc.memory().read( proc.read( reg ) + fetchNextWord( proc ) );
}

void RegisterIndirectOffset::store( ProcessorState& proc, Word value ) {
    proc.memory().write( proc.read( reg ) + fetchNextWord( proc ), value );
}

void Push::store( ProcessorState& proc, Word value ) {
    auto sp = proc.read( Special::Sp );
    proc.memory().write( sp - 1, value );
    proc.write( Special::Sp, sp - 1 );
}

Word Pop::load( ProcessorState& proc ) {
    auto sp = proc.read( Special::Sp );
    auto word = proc.memory().read( sp );
    proc.write( Special::Sp, sp + 1 );

    return word;
}

Word Pick::load( ProcessorState& proc ) {
    auto sp = proc.read( Special::Sp );
    return proc.memory().read( sp + next( proc ) );
}

void Pick::store( ProcessorState& proc, Word value ) {
    auto sp = proc.read( Special::Sp );
    proc.memory().write( sp + next( proc ), value );
}

Word Indirect::load( ProcessorState& proc ) {
    return proc.memory().read( next( proc ) );
}

void Indirect::store( ProcessorState& proc, Word value ) {
    proc.memory().write( next( proc ), value );
}

Word Direct::load( ProcessorState& proc ) {
    return next( proc );
}

void Direct::store( ProcessorState&, Word ) {
    // Nothing.
}

}

namespace instruction {

void Unary::execute( ProcessorState& proc ) const {
    Word loc;

    switch ( opcode ) {
    case SpecialOpcode::Iag:
    case SpecialOpcode::Ias:
    case SpecialOpcode::Rfi:
    case SpecialOpcode::Iaq:
    case SpecialOpcode::Hwn:
    case SpecialOpcode::Hwq:
    case SpecialOpcode::Hwi:
        // Handled by the parent simulation.
        break;
    case SpecialOpcode::Jsr:
        loc = address->load( proc );
        mode::Push {}.store( proc, proc.read( Special::Pc ) );
        proc.write( Special::Pc, loc );
        break;
    default:
        assert( ! "Unary instruction is unsupported!" );
    }
}

void Binary::execute( ProcessorState& proc ) const {
    using namespace boost::phoenix::placeholders;

    auto load = [&proc]( std::shared_ptr<AddressingMode> addr ) {
        return addr->load( proc );
    };

    auto store = [&proc]( std::shared_ptr<AddressingMode> addr, Word value ) {
        addr->store( proc, value );
    };

    auto apply = [&] ( std::function<Word( Word, Word )> f ) {
        auto y = load( addressA );
        auto x = load( addressB );

        store( addressB, f( x, y ) );
    };

    auto applyDouble = [&]( std::function<DoubleWord( DoubleWord, DoubleWord )> f,
                            std::function<void( DoubleWord )> action ) {
        auto yd = DoubleWord { load( addressA ) };
        auto xd = DoubleWord { load( addressB ) };
        auto zd = f( xd, yd );

        store( addressB, static_cast<Word>( zd ) );
        action( zd );
    };

    auto skipUnless = [&] ( std::function<bool( DoubleWord, DoubleWord )> f ) {
        auto y = load( addressA );
        auto x = load( addressB );

        proc.setSkip( ! f( x, y ) );
    };

    switch ( opcode ) {
    case Opcode::Set:
        store( addressB, load( addressA ) );
        break;
    case Opcode::Add:
        applyDouble( arg1 + arg2, [&proc]( DoubleWord z ) {
                if ( z > 0xffff ) {
                    proc.write( Special::Ex, 1 );
                } else {
                    proc.write( Special::Ex, 0 );
                }
            });
        break;
    case Opcode::Sub:
        applyDouble( arg1 - arg2, [&proc]( DoubleWord z ) {
                if ( z > 0xffff ) {
                    proc.write( Special::Ex, 0xffff );
                } else {
                    proc.write( Special::Ex, 0 );
                }
            });
        break;
    case Opcode::Bor:
        apply( arg1 | arg2 );
        break;
    case Opcode::Ife:
        skipUnless( arg1 == arg2 );
        break;
    case Opcode::Ifn:
        skipUnless( arg1 != arg2 );
        break;
    case Opcode::Ifg:
        skipUnless( arg1 > arg2 );
        break;
    case Opcode::Ifl:
        skipUnless( arg1 < arg2 );
        break;
    }
}

}

template <>
optional<Register> decode( const Word &w ) {
    switch ( w ) {
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
optional<Opcode> decode( const Word& w ) {
    switch ( w ) {
    case 0x01: return Opcode::Set;
    case 0x02: return Opcode::Add;
    case 0x03: return Opcode::Sub;
    case 0x0b: return Opcode::Bor;
    case 0x12: return Opcode::Ife;
    case 0x13: return Opcode::Ifn;
    case 0x14: return Opcode::Ifg;
    case 0x16: return Opcode::Ifl;
    default: return {};
    }
}

template <>
optional<SpecialOpcode> decode( const Word& w ) {
    switch ( w ) {
    case 0x01: return SpecialOpcode::Jsr;
    case 0x09: return SpecialOpcode::Iag;
    case 0x0a: return SpecialOpcode::Ias;
    case 0x0b: return SpecialOpcode::Rfi;
    case 0x10: return SpecialOpcode::Hwn;
    case 0x11: return SpecialOpcode::Hwq;
    case 0x12: return SpecialOpcode::Hwi;
    default: return {};
    }
}

static std::shared_ptr<AddressingMode>
decodeRegisterDirect( Word w ) {
    auto reg = decode<Register>( w );

    if ( reg ) {
        return std::make_shared<mode::RegisterDirect>( *reg );
    } else {
        return nullptr;
    }
}

static std::shared_ptr<AddressingMode>
decodeRegisterIndirect( Word w ) {
    auto reg = decode<Register>( w - 0x8 );

    if ( reg ) {
        return std::make_shared<mode::RegisterIndirect>( *reg );
    } else {
        return nullptr;
    }
}

static std::shared_ptr<AddressingMode>
decodeRegisterIndirectOffset( Word w ) {
    auto reg = decode<Register>( w - 0x10 );

    if ( reg ) {
        return std::make_shared<mode::RegisterIndirectOffset>( *reg );
    } else {
        return nullptr;
    }
}

static
std::shared_ptr<AddressingMode>
decodeFastDirect( Word w ) {
    Word value;

    if ( w >= 0x20 && w <= 0x3f ) {
        if ( w == 0x20 ) {
            value = 0xffff;
        } else {
            value = w - 0x21;
        }

        return std::make_shared<mode::FastDirect>( value );
    }

    return nullptr;
}

template <typename T>
static std::function<std::shared_ptr<T>( Word )>
decoderByValue( Word value ) {
    return [value]( Word w ) -> std::shared_ptr<T> {
        if ( w == value ) {
            return std::make_shared<T>();
        } else {
            return nullptr;
        }
    };
}

std::shared_ptr<AddressingMode>
decodeAddress( AddressContext context, Word word ) {
    std::shared_ptr<AddressingMode> addr = nullptr;

    auto decodePush = decoderByValue<mode::Push>( 0x18 );
    auto decodePop = decoderByValue<mode::Pop>( 0x18 );
    auto decodePeek = decoderByValue<mode::Peek>( 0x19 );
    // auto decodeSp = makeDecoder( 0x1b, address::sp );
    auto decodePick = decoderByValue<mode::Pick>( 0x1a );
    auto decodePc = decoderByValue<mode::Pc>( 0x1c );
    // auto decodeEx = makeDecoder( 0x1d, address::ex );
    auto decodeIndirect = decoderByValue<mode::Indirect>( 0x1e );
    auto decodeDirect = decoderByValue<mode::Direct>( 0x1f );
    // auto decodeDirect = makeDecoder( 0x1f, address::direct );

#define TRY( f ) addr = f( word ); if ( addr ) return addr;

    if ( context == AddressContext::A ) {
        TRY( decodePop );
        TRY( decodeFastDirect );
    }

    if ( context == AddressContext::B ) {
        TRY( decodePush );
    }

    TRY( decodeRegisterDirect );
    TRY( decodeRegisterIndirect );
    TRY( decodeRegisterIndirectOffset );
    TRY( decodePush );
    TRY( decodePeek );
    TRY( decodePick );
    // TRY( decodeSp );
    TRY( decodePc );
    // TRY( decodeEx );
    TRY( decodeIndirect );
    TRY( decodeDirect );
    
#undef TRY

    return {};
}

static std::shared_ptr<Instruction>
decodeBinaryInstruction( Word word ) {
    auto opcode = decode<Opcode>( word & 0x1f );
    if ( ! opcode ) return nullptr;

    auto addrA = decodeAddress( AddressContext::A, (word & 0xfc00) >> 10 );
    if ( ! addrA ) return nullptr;

    auto addrB = decodeAddress( AddressContext::B, (word & 0x3e0) >> 5 );
    if ( ! addrB ) return nullptr;

    return std::make_shared<instruction::Binary>( *opcode, addrB, addrA );
}

static std::shared_ptr<Instruction>
decodeUnaryInstruction( Word word ) {
    auto opcode = decode<SpecialOpcode>( (word & 0x3e0) >> 5 );
    if ( ! opcode ) return nullptr;

    auto addr = decodeAddress( AddressContext::A, (word & 0xfc00) >> 10 );
    if ( ! addr ) return nullptr;

    return std::make_shared<instruction::Unary>( *opcode, addr );
}

std::shared_ptr<Instruction> decodeInstruction( Word word ) {
    auto bins = decodeBinaryInstruction( word );
    if ( bins ) return bins;

    auto uins = decodeUnaryInstruction( word );
    if ( uins ) return uins;

    return nullptr;
}

static std::shared_ptr<Instruction> fetchNextInstruction( ProcessorState& proc ) {
    auto word = fetchNextWord( proc );
    auto ins = decodeInstruction( word );

    if ( ins ) {
        return ins;
    } else {
        throw error::MalformedInstruction { word };
    }
}

void ProcessorState::executeNext() {
    auto ins = fetchNextInstruction( *this );

    if ( doSkip() ) {
        advance( *this, ins->size() );
        setSkip( false );
    } else {
        ins->execute( *this );
        _lastInstruction = ins;
    }
}

void advance( ProcessorState& proc, int numWords ) {
    auto pc = proc.read( Special::Pc );
    proc.write( Special::Pc, pc + numWords );
}

void dumpToLog( ProcessorState& proc ) {
    LOG( PSTATE, info ) << "\n";
    LOG( PSTATE, info ) << format( "PC   : 0x%04x" ) % proc.read( Special::Pc );
    LOG( PSTATE, info ) << format( "SP   : 0x%04x" ) % proc.read( Special::Sp );
    LOG( PSTATE, info ) << format( "EX   : 0x%04x" ) % proc.read( Special::Ex );
    LOG( PSTATE, info ) << format( "A    : 0x%04x" ) % proc.read( Register::A );
    LOG( PSTATE, info ) << format( "B    : 0x%04x" ) % proc.read( Register::B );
    LOG( PSTATE, info ) << format( "C    : 0x%04x" ) % proc.read( Register::C );
    LOG( PSTATE, info ) << format( "X    : 0x%04x" ) % proc.read( Register::X );
    LOG( PSTATE, info ) << format( "Y    : 0x%04x" ) % proc.read( Register::Y );
    LOG( PSTATE, info ) << format( "Z    : 0x%04x" ) % proc.read( Register::Z );
    LOG( PSTATE, info ) << format( "I    : 0x%04x" ) % proc.read( Register::I );
    LOG( PSTATE, info ) << format( "J    : 0x%04x" ) % proc.read( Register::J );
    LOG( PSTATE, info ) << format( "skip : %s" ) % proc.doSkip();

    std::string stackContents { "" };
    const int STACK_SIZE = processor::STACK_BEGIN - proc.read( Special::Sp );

    for ( int i = 0; i < STACK_SIZE; ++i ) {
        stackContents += (format( "0x%04x, " ) % proc.memory().read( proc.read( Special::Sp ) + i )).str();
    }

    LOG( PSTATE, info ) << "stack: " << stackContents;
    LOG( PSTATE, info ) << "\n";
}

}
