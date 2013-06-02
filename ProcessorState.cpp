#include "ProcessorState.hpp"

#include <functional>

#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>

Word ProcessorState::read( Register reg ) const {
    return _registers[registerIndex( reg )];
}

Word ProcessorState::read( Special spec ) const {
    switch ( spec ) {
    case Special::Pc: return _pc;
    case Special::Sp: return _sp;
    case Special::Ex: return _ex;
    }
}

void ProcessorState::write( Register reg, Word value ) {
    _registers[registerIndex( reg )] = value;
}

void ProcessorState::write( Special spec, Word value ) {
    switch ( spec ) {
    case Special::Pc: _pc = value; return;
    case Special::Sp: _sp = value; return;
    case Special::Ex: _ex = value; return;
    }
}

static Word fetchNextWord( ProcessorState& proc ) {
    return mode::Direct {}.load( proc );
}

namespace mode {

Word Direct::load( ProcessorState& proc ) const {
    proc.tickClock( 1 );

    auto pc = proc.read( Special::Pc );
    proc.write( Special::Pc, pc + 1 );
    return proc.memory().read( pc );
}

void Direct::store( ProcessorState& proc, Word value ) const {
    proc.tickClock( 1 );

    auto pc = proc.read( Special::Pc );
    proc.write( Special::Pc, pc + 1 );
    proc.memory().write( pc, value );
}

}

namespace instruction {

void Unary::execute( ProcessorState& proc ) const {
    if ( proc.doSkip() ) {
        advance( proc, address->size() );
        return;
    }

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
        mode::Push {}.store( proc, proc.read( Special::Pc ) );
        proc.write( Special::Pc, address->load( proc ) );
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

    if ( proc.doSkip() ) {
        advance( proc,
                 addressA->size() + addressB->size() );
        return;
    }

    auto store = [&proc]( std::shared_ptr<AddressingMode> addr, Word value ) {
        addr->store( proc, value );
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
    case Opcode::Ife:
        skipUnless( arg1 == arg2 );
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
    case 0x12: return Opcode::Ife;
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

// static std::shared_ptr<AddressingMode>
// decodeRegisterIndirectOffset( Word w ) {
//     auto reg = decode<Register>( w - 0x10 );

//     if ( reg ) {
//         return std::make_shared<mode::RegisterIndirectOffset>( *reg );
//     } else {
//         return nullptr;
//     }
// }

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

// static optional<Address> decodePush( Word w ) {
//     if ( w == 0x18 ) {
//         return address::push();
//     } else {
//         return {};
//     }
// }

// static optional<Address> decodePop( Word w ) {
//  p   if ( w == 0x18 ) {
//         return address::pop();
//     } else {
//         return {};
//     }
// }


std::shared_ptr<AddressingMode>
decodeAddress( AddressContext context, Word word ) {
    std::shared_ptr<AddressingMode> addr = nullptr;

    auto decodePeek = decoderByValue<mode::Peek>( 0x18 );
    // auto decodePick = makeDecoder( 0x1a, address::pick );
    // auto decodeSp = makeDecoder( 0x1b, address::sp );
    auto decodePc = decoderByValue<mode::Pc>( 0x1c );
    // auto decodeEx = makeDecoder( 0x1d, address::ex );
    // auto decodeIndirect = makeDecoder( 0x1e, address::indirect );
    auto decodeDirect = decoderByValue<mode::Direct>( 0x1f );
    // auto decodeDirect = makeDecoder( 0x1f, address::direct );

#define TRY( f ) addr = f( word ); if ( addr ) return addr;

    // if ( context == AddressContext::A ) {
    //     TRY( decodePop );
    //     TRY( decodeFastDirect );
    // }

    TRY( decodeRegisterDirect );
    TRY( decodeRegisterIndirect );
    // TRY( decodeRegisterIndirectOffset );
    // TRY( decodePush );
    // TRY( decodePop );
    TRY( decodePeek );
    // TRY( decodePick );
    // TRY( decodeSp );
    TRY( decodePc );
    // TRY( decodeEx );
    // TRY( decodeIndirect );
    TRY( decodeDirect );
    TRY( decodeFastDirect );
    
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
    ins->execute( *this );

    if ( _doSkip ) {
        setSkip( false );
    } else {
        _lastInstruction = ins;
    }
}

void advance( ProcessorState& proc, int numWords ) {
    auto pc = proc.read( Special::Pc );
    proc.write( Special::Pc, pc + numWords );
}
