#include "Random.hpp"
#include "../ProcessorState.hpp"

#include <array>
#include <limits>

#include <gtest/gtest.h>

class RegisterTest : public ::testing::Test {
protected:
    ProcessorState _proc;
public:
    explicit RegisterTest() : _proc { ProcessorState { 0 } } {}
};

TEST_F( RegisterTest, Initialization ) {
    EXPECT_EQ( 0, _proc.read( Register::A ) );
    EXPECT_EQ( 0, _proc.read( Register::B ) );
    EXPECT_EQ( 0, _proc.read( Register::C ) );
    EXPECT_EQ( 0, _proc.read( Register::X ) );
    EXPECT_EQ( 0, _proc.read( Register::Y ) );
    EXPECT_EQ( 0, _proc.read( Register::Z ) );
    EXPECT_EQ( 0, _proc.read( Register::I ) );
    EXPECT_EQ( 0, _proc.read( Register::J ) );
}

TEST_F( RegisterTest, ReadWrite ) {
    std::array<Register, 8> registers = {
        Register::A,
        Register::B,
        Register::C,
        Register::X,
        Register::Y,
        Register::Z,
        Register::I,
        Register::J
    };

    auto regGen = makeDiscreteGenerator( std::begin( registers ),
                                         std::end( registers ) );

    NumericGenerator<Word> valueGen;

    // Pick a random register and read and write to it.
    for ( int i = 0; i < 5000; ++i ) {
        auto reg = regGen.next();
        auto val = valueGen.next();

        _proc.write( reg, val );
        EXPECT_EQ( val, _proc.read( reg ) );
    }
}

class SpecialTest : public ::testing::Test {
protected:
    ProcessorState _proc;
public:
    explicit SpecialTest() : _proc { ProcessorState { 0 } } {}
};

TEST_F( SpecialTest, Initialization ) {
    EXPECT_EQ( STACK_BEGIN, _proc.read( Special::Sp ) );
    EXPECT_EQ( 0, _proc.read( Special::Pc ) );
    EXPECT_EQ( 0, _proc.read( Special::Ex ) );
}

TEST_F( SpecialTest, ReadWrite ) {
    std::array<Special, 3> specials = {
        Special::Sp,
        Special::Pc,
        Special::Ex
    };

    auto specGen = makeDiscreteGenerator( std::begin( specials ),
                                          std::end( specials ) );

    NumericGenerator<Word> valueGen;

    // Pick a random special value and read and write to it.
    for ( int i = 0; i < 5000; ++i ) {
        auto spec = specGen.next();
        auto val = valueGen.next();

        _proc.write( spec, val );
        EXPECT_EQ( val, _proc.read( spec ) );
    }
}

// class AddressTest : public ::testing::Test {
// protected:
//     Processor _proc;
// public:
//     explicit AddressTest() : _proc { Processor { 0x10000 } } {}
// };

// TEST_F( AddressTest, LiteralAttr ) {
//     auto regDirect = address::registerDirect( Register::A );
//     auto regIndirect = address::registerIndirect( Register::A );
//     auto regIndirectOffset = address::registerIndirectOffset( Register::A );
//     auto push = address::push();
//     auto pop = address::pop();
//     auto peek = address::peek();
//     auto pick = address::pick();
//     auto sp = address::sp();
//     auto pc = address::pc();
//     auto ex = address::ex();
//     auto indirect = address::indirect();
//     auto direct = address::direct();
//     auto fastDirect = address::fastDirect( 0 );

//     EXPECT_EQ( false, address::isLiteral( regDirect ) );
//     EXPECT_EQ( false, address::isLiteral( regIndirect ) );
//     EXPECT_EQ( false, address::isLiteral( regIndirectOffset ) );
//     EXPECT_EQ( false, address::isLiteral( push ) );
//     EXPECT_EQ( false, address::isLiteral( pop ) );
//     EXPECT_EQ( false, address::isLiteral( peek ) );
//     EXPECT_EQ( false, address::isLiteral( pick ) );
//     EXPECT_EQ( false, address::isLiteral( sp ) );
//     EXPECT_EQ( false, address::isLiteral( pc ) );
//     EXPECT_EQ( false, address::isLiteral( ex ) );
//     EXPECT_EQ( false, address::isLiteral( indirect ) );
//     EXPECT_EQ( true, address::isLiteral( direct ) );
//     EXPECT_EQ( true, address::isLiteral( fastDirect ) );
// }

// TEST_F( AddressTest, RegisterDirect ) {
//     _proc.write( Register::A, 0xdead );

//     auto addr = address::registerDirect( Register::A );
//     EXPECT_EQ( 0xdead, address::read( _proc, addr ) );
//     EXPECT_EQ( 0, _proc.clock() );

//     address::write( _proc, addr, 0xbeef );
//     EXPECT_EQ( 0xbeef, _proc.read( Register::A ) );
//     EXPECT_EQ( 0, _proc.clock() );
// }

// TEST_F( AddressTest, RegisterIndirect ) {
//     _proc.write( Register::A, 0x30 );
//     _proc.write( 0x30, 0xdead );

//     auto addr = address::registerIndirect( Register::A );
//     EXPECT_EQ( 0xdead, address::read( _proc, addr ) );
//     EXPECT_EQ( 0, _proc.clock() );

//     address::write( _proc, addr, 0xbeef );
//     EXPECT_EQ( 0xbeef, _proc.read( 0x30 ) );
//     EXPECT_EQ( 0, _proc.clock() );
// }

// TEST_F( AddressTest, RegisterIndirectOffset ) {
//     _proc.write( Register::A, 20 );
//     _proc.write( 0, 5 );
//     _proc.write( 1, 10 );
//     _proc.write( 25, 0xdead );

//     auto addr = address::registerIndirectOffset( Register::A );
//     EXPECT_EQ( 0xdead, address::read( _proc, addr ) );
//     EXPECT_EQ( 1, _proc.clock() );

//     address::write( _proc, addr, 0xbeef );
//     EXPECT_EQ( 0xbeef, _proc.read( 30 ) );
//     EXPECT_EQ( 2, _proc.clock() );
// }

// TEST_F( AddressTest, StackManip ) {
//     auto push = address::push();
//     auto peek = address::peek();
//     auto pop = address::pop();
//     auto pick = address::pick();

//     address::write( _proc, push, 1 );
//     EXPECT_EQ( 1, address::read( _proc, peek ) );

//     address::write( _proc, push, 2 );
//     EXPECT_EQ( 2, address::read( _proc, peek ) );

//     address::write( _proc, push, 3 );
//     EXPECT_EQ( 3, address::read( _proc, peek ) );

//     EXPECT_EQ( 0, _proc.clock() );

//     _proc.write( 0, 0 );
//     EXPECT_EQ( 3, address::read( _proc, pick ) );
    
//     _proc.write( 1, 1 );
//     EXPECT_EQ( 2, address::read( _proc, pick ) );

//     _proc.write( 2, 2 );
//     EXPECT_EQ( 1, address::read( _proc, pick ) );

//     EXPECT_EQ( 3, _proc.clock() );

//     EXPECT_EQ( 3, address::read( _proc, pop ) );
//     EXPECT_EQ( 2, address::read( _proc, pop ) );
//     EXPECT_EQ( 1, address::read( _proc, pop ) );

//     EXPECT_EQ( 3, _proc.clock() );

//     // Check underflow.
//     EXPECT_THROW( address::read( _proc, pop ), error::StackUnderflow );

//     // Check overflow.
//     for ( Word i = 0; i < STACK_BEGIN; ++i ) {
//         address::write( _proc, push, i );
//     }

//     EXPECT_THROW( address::write( _proc, push, 0xdead ), error::StackOverflow );
// }

// TEST_F( AddressTest, Indirect ) {
//     auto addr = address::indirect();

//     _proc.write( 0, 0x100 );
//     _proc.write( 0x100, 0xdead );

//     EXPECT_EQ( 0xdead, address::read( _proc, addr ) );
//     EXPECT_EQ( 1, _proc.clock() );

//     _proc.write( 1, 0x200 );
//     address::write( _proc, addr, 0xbeef );
//     EXPECT_EQ( 0xbeef, _proc.read( 0x200 ) );
//     EXPECT_EQ( 2, _proc.clock() );
// }

// TEST_F( AddressTest, Direct ) {
//     auto addr = address::direct();

//     _proc.write( 0, 0xdead );
//     EXPECT_EQ( 0xdead, address::read( _proc, addr ) );
//     EXPECT_EQ( 1, _proc.clock() );

//     // Ignored.
//     address::write( _proc, addr, 0xbeef );
//     EXPECT_EQ( 0, _proc.read( 1 ) );
//     EXPECT_EQ( 1, _proc.clock() );
// }

// TEST_F( AddressTest, FastDirect ) {
//     auto addr = address::fastDirect( 0xdead );

//     EXPECT_EQ( 0xdead, address::read( _proc, addr ) );
//     EXPECT_EQ( 0, _proc.clock() );

//     // Ignored.
//     address::write( _proc, addr, 0xbeef );
//     EXPECT_EQ( 0xdead, address::read( _proc, addr ) );
//     EXPECT_EQ( 0, _proc.clock() );
// }
