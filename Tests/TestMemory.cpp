// Tests/TestMemory.cpp
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
#include "../Memory.hpp"

#include <thread>

#include <gtest/gtest.h>

using namespace nebula;

const int NUM_OPS = 100000;

class MemoryTest : public ::testing::Test {
protected:
    Memory _memory;
public:
    explicit MemoryTest() : _memory { 0x10000 } {}
};

TEST_F( MemoryTest, Initialization ) {
    for ( int i = 0; i < _memory.size(); ++i ) {
        EXPECT_EQ( 0, _memory.read( i ) );
    }
}

TEST_F( MemoryTest, ReadWrite ) {
    auto maxLocation = static_cast<Word>( _memory.size() - 1 );
    auto locationGen = NumericGenerator<Word> { 0, maxLocation };
    NumericGenerator<Word> valueGen;

    for ( int i = 0; i < NUM_OPS; ++i ) {
        auto loc = locationGen.next();
        auto val = valueGen.next();

        _memory.write( loc, val );
        EXPECT_EQ( val, _memory.read( loc ) );
    }
}

TEST_F( MemoryTest, ConcurrentReadWrite ) {
    constexpr Word MAX_LOCATION = 0x100;
    
    NumericGenerator<Word> locationGen { 0, MAX_LOCATION };

    auto makeWriter = [&] ( Word value ) {
        return [&, value] {
            for ( int i = 0; i < NUM_OPS; ++i ) {
                _memory.write( locationGen.next(), value );
            }
        };
    };

    auto w1 = makeWriter( 0xdead );
    auto w2 = makeWriter( 0xbeef );

    std::thread th1 { w1 };
    std::thread th2 { w2 };

    th1.join();
    th2.join();
}
