/**
   \file printable.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
   
   \brief Interface for printable things.
 */

#ifndef NEBULA_PRINTABLE_HPP_
#define NEBULA_PRINTABLE_HPP_

#include <iostream>

namespace nebula {

/**
   \brief Printable classes.
   
   Clases inheriting IPrintable are capable of printing out a human-readable
   representation of themselves to an output stream.
 */
class IPrintable {
 public:
  virtual std::ostream& print(std::ostream& os) const = 0;

  virtual ~IPrintable() = default;
};

/**
   \brief All IPrintable classes can be output via the << operator.
 */
inline std::ostream& operator<<(std::ostream& os, const IPrintable& printable) {
  printable.print(os);
  return os;
}

}  // namespace nebula

#endif  // NEBULA_PRINTABLE_HPP_
