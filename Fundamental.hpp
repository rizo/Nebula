// Fundamental.hpp
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

#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>
#include <boost/log/core.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>

namespace nebula {

using boost::format;
using boost::optional;

using Word = std::uint16_t;
using DoubleWord = std::uint32_t;

using SignedWord = std::int16_t;
using SignedDoubleWord = std::int32_t;

namespace logging {

using namespace boost::log;

using Severity = trivial::severity_level;
using Logger = sources::severity_channel_logger_mt<Severity, std::string>;

void initialize( bool enable, Severity minimumSeverity );

}

}

#define DEFINE_LOGGER( name, str )                                             \
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT( name, nebula::logging::Logger )       \
    {                                                                          \
        return nebula::logging::Logger( boost::log::keywords::channel = str ); \
    }

#define LOG( logger,  severity ) BOOST_LOG_SEV( logger ::get(), logging::Severity:: severity)

template<typename T, typename... Args>
std::unique_ptr<T> make_unique( Args&&... args ) {
    return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
}

#define B_ BOOST_BINARY

#define SAFE_VALUE_WRAPPER( name, type )                \
    struct name {                                       \
        explicit name (const type& v ) : value { v } {} \
        type value;                                     \
    }    
