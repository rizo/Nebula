/**
   \file prelude.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Fundamental functionality shared by all of Nebula.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   The definitions in this file are used pervasively across Nebula. They include
   fundamental operations such as logging, smart pointers, and some helpful
   convenience functions.
 */

#ifndef NEBULA_PRELUDE_HPP_
#define NEBULA_PRELUDE_HPP_

#pragma once

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/variant.hpp>

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace nebula {

using std::shared_ptr;
using std::unique_ptr;

/**
   \brief Create a std::unique_ptr.

   \note This functionality is a notable omission from the C++11 standard, which
   includes the similar std::make_shared. This is corrected in a later revision
   of the stanard, so this definition will be removed at a later date.
 */
template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

using Word = std::uint16_t;             //!< A single machine-sized word.
using DoubleWord = std::uint32_t;       //!< The size of two ::Word.
using SignedWord = std::int16_t;        //!< A signed machine-sized word.
using SignedDoubleWord = std::int32_t;  //!< The size of two ::SignedWord.

namespace fs = boost::filesystem;

using boost::format;
using boost::optional;

/**
   \brief Execute an action for a variant of a particular type only.

   Invoke the callable object \p action only when the variant \p v is holding an
   object of the instantiated type of this function.

   \code{.cpp}
   boost::variant<int, std::string> v{23};
   executeWhen<int>(v, []() { std::cout << "v is an int!\n"; });
   \endcode
 */
template <typename T, typename Variant>
void executeWhen(const Variant& v, const std::function<void()>& action) {
  if (const T* var = boost::get<T>(&v)) {
    // We get a warning that `var' is unused otherwise.
    (void)var;
    action();
  }
}

/**
   \brief Execute an action with the underlying object of a variant.

   When this function is instantiated with type \p T, it invokes the \p action
   with the underlying object of type \p T stored in the variant \p v.
 */
template <typename T, typename Variant>
void executeWhen(const Variant& v,
                 const std::function<void(T& value)>& action) {
  if (const T* var = boost::get<T>(&v)) {
    T var_copy{*var};
    action(var_copy);
  }
}

/**
   \brief Check if a variant contains a particular type.

   Return \p true if the instantiated type of this function matches the type of
   the object held in the variant \p v.

   \code{.cpp}
   boost::variant<int, std::string> v{"abc"};
   bool b = variantIs<std::string>(v); // b == true
   \endcode
 */
template <typename T, typename Variant>
bool variantIs(const Variant& v) {
  if (const T* var = boost::get<T>(&v)) {
    return true;
  } else {
    return false;
  }
}

/** \brief Logging utilities used across Nebula.
 */
namespace logging {

using namespace boost::log;

using Severity = trivial::severity_level;
using Logger = sources::severity_channel_logger_mt<Severity, std::string>;

/**
   \brief Disable logging.

   \note This function *must* be called even if no logging is desired, since it
   initializes the necessary sub-systems.
 */
void initialize();

/**
   \brief Enable and configure logging.

   \param output_path The path the file where the logs will be written.

   \param minimum_severity Only logging messages greater than \p
   minimum_severity will be written to the log. For instance, if the minimum
   severity is logging::Severity::info, then no logging::Severity::debug
   messages will be included.
 */
void initialize(const fs::path& output_path, Severity minimum_severity);

/**
   \brief Define a logger.

   Once created, a logger should be accessible from any compilation unit which
   includes this definition. In practise, loggers are local to a single
   compilation unit.

   Once defined, loggers can be used to log messages.
   For instance:
   \code{.cpp}
   DEFINE_LOGGER(FOO)
   LOG(FOO, info) << "Hello, from the Foo system!\n";
   \endcode
*/
#define DEFINE_LOGGER(name)                                             \
  BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(name, nebula::logging::Logger) {  \
    return nebula::logging::Logger(nebula::logging::keywords::channel = \
                                       #name);                          \
  }

/**
   \brief Log a message wth a logger.

   Log a message with the \p logger defined previously by DEFINE_LOGGER() where
   \p severity can be one of \p trace, \p debug, \p info, \p warning, \p error,
   \p fatal.

   Logging is thread-safe and should be relatively inexpensive. It is better to
   log more information in the source and disable it at run-time then to be
   conservative.

   When at all possible, prefer log messages that are context-free: messages
   should *not* depend on an understanding of the source code from which they
   originate.
 */
#define LOG(logger, severity) \
  BOOST_LOG_SEV(logger::get(), logging::Severity::severity)

}  // namespace logging

/**
   \brief Do a key-value lookup on a map.

   This is small convenience wrapper which returns an empty result value when
   the key is not found in the map.
 */
template <typename K, typename V>
optional<V> get(const K& key, const std::unordered_map<K, V>& map) {
  auto iter = map.find(key);

  if (iter != map.end()) {
    return iter->second;
  } else {
    return {};
  }
}

}  // namespace nebula

#endif  // NEBULA_PRELUDE_HPP_
