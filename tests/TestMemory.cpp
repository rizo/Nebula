#include "Random.hpp"
#include "../Memory.hpp"

#include <thread>

#include <gtest/gtest.h>

constexpr int NUM_OPS = 100000;

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
