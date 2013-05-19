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

namespace instruction {

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

// void execute( Processor& proc, const Instruction& ins ) {
//     apply_visitor( ExecuteVisitor { &proc }, ins );
// }

// static void alwaysExecute( Processor& proc, const instruction::Binary& ins ) {
//     using namespace boost::phoenix::placeholders;

//     auto read = [&proc]( const Address& addr ) {
//         return address::read( proc, addr );
//     };

//     auto write = [&proc]( const Address& addr, Word value ) {
//         address::write( proc, addr, value );
//     };

//     auto apply = [&]( std::function<Word( Word, Word )> f ) {
//         auto x = read( ins.addressB );
//         auto y = read( ins.addressA );

//         write( ins.addressB, f( x, y ) );
//     };

//     auto applyExpanded = [&]( std::function<SignedDoubleWord( SignedDoubleWord, SignedDoubleWord )> f,
//                               std::function<void( SignedDoubleWord )> action ) {
//         auto x = static_cast<SignedDoubleWord>( read( ins.addressB ) );
//         auto y = static_cast<SignedDoubleWord>( read( ins.addressA ) );
        
//         auto z = f( x, y );
//         write( ins.addressB, static_cast<Word>( z ) );
//         action( z );
//     };

//     switch ( ins.opcode ) {
//     case Opcode::Set:
//         write( ins.addressB, read( ins.addressA ) );
//         break;
//     case Opcode::Add: applyExpanded( arg1 + arg2, [&proc]( SignedDoubleWord z ) {
//             if ( z > 0xffff ) {
//                 proc.write( Special::Ex, 1 );
//             } else {
//                 proc.write( Special::Ex, 0 );
//             }
//         });
//         break;
//     case Opcode::Sub:
//         apply( arg1 - arg2 );
//         break;
//     }
// }

// static void alwaysExecute( Processor&, const instruction::Unary& ) {
//     assert( false );
// }

// template <>
// optional<Register> decode( const Word& word ) {
//     switch ( word ) {
//     case 0: return Register::A;
//     case 1: return Register::B;
//     case 2: return Register::C;
//     case 3: return Register::X;
//     case 4: return Register::Y;
//     case 5: return Register::Z;
//     case 6: return Register::I;
//     case 7: return Register::J;
//     default: return {};
//     }
// }

// template <>
// optional<Opcode> decode( const Word& word ) {
//     switch ( word ) {
//     case 0x01: return Opcode::Set;
//     case 0x02: return Opcode::Add;
//     case 0x03: return Opcode::Sub;
//     default: return {};
//     }
// }

// template <>
// optional<SpecialOpcode> decode( const Word& word ) {
//     switch ( word ) {
//     case 0x01: return SpecialOpcode::Jsr;
//     default: return {};
//     }
// }

// template <typename U, typename V>
// static optional<V> map( const optional<U>& a, std::function<V ( const U& )> f ) {
//     if ( a ) {
//         return f( *a );
//     } else {
//         return {};
//     }
// }

// static optional<Address> decodeRegisterDirect( Word w ) {
//     return map<Register, Address>( decode<Register>( w ), address::registerDirect );
// }

// static optional<Address> decodeRegisterIndirect( Word w ) {
//     return map<Register, Address>( decode<Register>( w - 0x8 ), address::registerIndirect );
// }

// static optional<Address> decodeRegisterIndirectOffset( Word w ) {
//     return map<Register, Address>( decode<Register>( w - 0x10 ), address::registerIndirectOffset );
// }

// static optional<Address> decodePush( Word w ) {
//     if ( w == 0x18 ) {
//         return address::push();
//     } else {
//         return {};
//     }
// }

// static optional<Address> decodePop( Word w ) {
//     if ( w == 0x18 ) {
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

// optional<Address> decodeAddress( const Word& word, AddressContext context ) {
//     optional<Address> addr;

//     auto decodePeek = makeDecoder( 0x19, address::peek );
//     auto decodePick = makeDecoder( 0x1a, address::pick );
//     auto decodeSp = makeDecoder( 0x1b, address::sp );
//     auto decodePc = makeDecoder( 0x1c, address::pc );
//     auto decodeEx = makeDecoder( 0x1d, address::ex );
//     auto decodeIndirect = makeDecoder( 0x1e, address::indirect );
//     auto decodeDirect = makeDecoder( 0x1f, address::direct );

// #define TRY( f ) addr = f( word ); if ( addr ) return addr;

//     if ( context == AddressContext::A ) {
//         TRY( decodePop );
//         TRY( decodeFastDirect );
//     }

//     TRY( decodeRegisterDirect );
//     TRY( decodeRegisterIndirect );
//     TRY( decodeRegisterIndirectOffset );
//     TRY( decodePush );
//     TRY( decodePop );
//     TRY( decodePeek );
//     TRY( decodePick );
//     TRY( decodeSp );
//     TRY( decodePc );
//     TRY( decodeEx );
//     TRY( decodeIndirect );
//     TRY( decodeDirect );
    
// #undef TRY

//     return {};
// }

// template <>
// optional<Instruction> decode( const Word& word ) {
//     auto binary = [word]() -> optional<instruction::Binary> {
//         auto opcode = decode<Opcode>( word & 0x1f );
//         if ( ! opcode ) return {};

//         auto addrA = decodeAddress( (word & 0xfc00) >> 10, AddressContext::A );
//         if ( ! addrA ) return {};

//         auto addrB = decodeAddress( (word & 0x3e0) >> 5, AddressContext::B );
//         if ( ! addrB ) return {};

//         return instruction::Binary { *opcode, *addrB, *addrA };
//     };

//     auto unary = [word]() -> optional<instruction::Unary> {
//         auto opcode = decode<SpecialOpcode>( (word & 0x3e0) >> 5 );
//         if ( ! opcode ) return {};

//         auto addr = decodeAddress( (word & 0xfc00) >> 10, AddressContext::A );
//         if ( ! addr ) return {};

//         return instruction::Unary { *opcode, *addr };
//     };

//     auto binIns = binary();
//     if ( binIns ) return Instruction { *binIns };

//     auto unIns = unary();
//     if ( unIns ) return Instruction { *unIns };

//     return {};
// }

// static Instruction fetchNextInstruction( Processor& proc ) {
//     auto word = fetchNextWord( proc );
//     auto ins = decode<Instruction>( word );

//     if ( ins ) {
//         return *ins;
//     } else {
//         throw error::MalformedInstruction { word };
//     }
// }

// void executeNext( Processor& proc ) {
//     auto ins = fetchNextInstruction( proc );

//     execute( proc, ins );
// }
