#ifndef __FUNDAMENTAL_H__
#define __FUNDAMENTAL_H__

#include <cstdint>
#include <functional>
#include <memory>

#include <boost/optional.hpp>

using boost::optional;

using Word = std::uint16_t;
using DoubleWord = std::uint32_t;

using SignedWord = std::int16_t;
using SignedDoubleWord = std::uint32_t;

template<typename T, typename... Args>
std::unique_ptr<T> make_unique( Args&&... args ) {
    return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
}

#endif // __FUNDAMENTAL_H__
