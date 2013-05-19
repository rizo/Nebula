#include "Random.hpp"
#include "../Memory.hpp"

#include <gtest/gtest.h>

constexpr int NUM_OPS = 5000;

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
