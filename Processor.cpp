#include "Processor.hpp"

#include <functional>

#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>

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

static Word fetchNextWord( Processor& proc ) {
    return mode::Direct {}.load( proc );
}

namespace mode {

Word Direct::load( Processor& proc ) const {
    common( proc );

    auto pc = proc.read( Special::Pc );
    proc.write( Special::Pc, pc + 1 );
    return proc.memory().read( pc );
}

void Direct::store( Processor& proc, Word value ) const {
    common( proc );

    auto pc = proc.read( Special::Pc );
    proc.write( Special::Pc, pc + 1 );
    proc.memory().write( pc, value );
}

}

namespace instruction {

void Unary::execute( Processor& proc ) const {
    assert( ! "Unary instructions are unsupported!" );
}

void Binary::execute( Processor& proc ) const {
    using namespace boost::phoenix::placeholders;

    auto load = [&proc]( std::shared_ptr<AddressingMode> addr ) {
        return addr->load( proc );
    };

    auto store = [&proc]( std::shared_ptr<AddressingMode> addr, Word value ) {
        addr->store( proc, value );
    };

    auto applyDouble = [&]( std::function<DoubleWord( DoubleWord, DoubleWord )> f,
                            std::function<void( DoubleWord )> action ) {
        auto y = DoubleWord { load( addressA ) };
        auto x = DoubleWord { load( addressB ) };
        auto z = f( x, y );

        store( addressB, static_cast<Word>( z ) );
        action( z );
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
    default: return {};
    }
}

template <>
optional<SpecialOpcode> decode( const Word& w ) {
    switch ( w ) {
    case 0x01: return SpecialOpcode::Jsr;
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

// static optional<Address> decodeFastDirect( Word w ) {
//     if ( w >= 0x20 && w <= 0x3f ) {
//         if ( w == 0x20 ) {
//             return address::fastDirect( 0xffff );
//         } else {
//             return address::fastDirect( w - 0x21 );
//         }
//     } else {
//         return {};
//     }
// }

// static
// std::function<optional<Address>( Word )> 
// makeDecoder( Word value,
//              std::function<Address()> f ) {
//     auto dec = [value, f]( Word w ) -> optional<Address> {
//         if ( w == value ) {
//             return f();
//         } else {
//             return {};
//         }
//     };

//     return dec;
// }

std::shared_ptr<AddressingMode>
decodeAddress( AddressContext context, Word word ) {
    std::shared_ptr<AddressingMode> addr = nullptr;

    auto decodePeek = decoderByValue<mode::Peek>( 0x18 );
    // auto decodePick = makeDecoder( 0x1a, address::pick );
    // auto decodeSp = makeDecoder( 0x1b, address::sp );
    // auto decodePc = makeDecoder( 0x1c, address::pc );
    // auto decodeEx = makeDecoder( 0x1d, address::ex );
    // auto decodeIndirect = makeDecoder( 0x1e, address::indirect );
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
    // TRY( decodePc );
    // TRY( decodeEx );
    // TRY( decodeIndirect );
    // TRY( decodeDirect );
    
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

static std::shared_ptr<Instruction> fetchNextInstruction( Processor& proc ) {
    auto word = fetchNextWord( proc );
    auto ins = decodeInstruction( word );

    if ( ins ) {
        return ins;
    } else {
        throw error::MalformedInstruction { word };
    }
}

void executeNext( Processor& proc ) {
    auto ins = fetchNextInstruction( proc );
    ins->execute( proc );
}
