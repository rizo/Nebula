#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>
#include <glog/logging.h>

namespace nebula {

using boost::format;
using boost::optional;

using Word = std::uint16_t;
using DoubleWord = std::uint32_t;

using SignedWord = std::int16_t;
using SignedDoubleWord = std::uint32_t;

}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique( Args&&... args ) {
    return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
}

#define B_ BOOST_BINARY
