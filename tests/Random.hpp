#ifndef __RANDOM_HPP__
#define __RANDOM_HPP__

#include <iterator>
#include <random>
#include <limits>

static std::random_device RANDOM_DEVICE;

template <typename T>
class Generator {
    virtual T next() = 0;
};

template <typename N>
class NumericGenerator : public Generator<N> {
    std::default_random_engine _engine { RANDOM_DEVICE() };
    std::uniform_int_distribution<N> _dist;
public:
    explicit NumericGenerator( N lowerBound, N upperBound ) :
        _dist { lowerBound, upperBound } {
    }

    explicit NumericGenerator() :
        _dist { std::numeric_limits<N>::min(), std::numeric_limits<N>::max() } {
    }

    virtual N next() { return _dist( _engine ); }
};

template <typename RandomAccessIter>
class DiscreteGenerator : public Generator<typename std::iterator_traits<RandomAccessIter>::value_type> {
    using index_type = typename std::iterator_traits<RandomAccessIter>::difference_type;

    NumericGenerator<index_type> _indexGen;
    RandomAccessIter _begin;
public:
    using value_type = typename std::iterator_traits<RandomAccessIter>::value_type;

    explicit DiscreteGenerator( RandomAccessIter begin, RandomAccessIter end ) :
        _indexGen { NumericGenerator<index_type> { 0, end - begin - 1 } },
        _begin { begin } {
    }

    virtual inline value_type next() {
        return _begin[_indexGen.next()];
    }
};

template <typename RandomAccessIter>
DiscreteGenerator<RandomAccessIter> makeDiscreteGenerator( RandomAccessIter begin,
                                                           RandomAccessIter end ) {
    return DiscreteGenerator<RandomAccessIter> { begin, end };
}

#endif // __RANDOM_HPP__
