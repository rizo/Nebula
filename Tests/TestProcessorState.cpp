// Tests/TestProcessorState.cpp
//
// Copyright 2013 Jesse Haber-Kucharsky
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Random.hpp"
#include "../ProcessorState.hpp"

#include <array>
#include <limits>

#include <gtest/gtest.h>

using namespace nebula;

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
    EXPECT_EQ( processor::STACK_BEGIN, _proc.read( Special::Sp ) );
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


