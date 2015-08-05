/**
   \file binary-units.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Dimensional units for computer systems.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_BINARY_UNITS_HPP_
#define NEBULA_BINARY_UNITS_HPP_

#include "units.hpp"

namespace nebula {

namespace units {

namespace binary {

using namespace boost::units;

struct InformationBaseDimension : base_dimension<InformationBaseDimension, 1> {
};

using InformationDimension = InformationBaseDimension::dimension_type;

struct WordBaseUnit : base_unit<WordBaseUnit, InformationDimension, 1> {};

/**
   The binary dimensional system consists of a single dimension, information,
   whose base unit is a word (16 B).
 */
using BinarySystem = make_system<WordBaseUnit>::type;

using Dimensionless = unit<dimensionless_type, BinarySystem>;
using Information = unit<InformationDimension, BinarySystem>;

using DoubleWord =
    make_scaled_unit<Information, scale<2, static_rational<1>>>::type;

using Byte = make_scaled_unit<Information, scale<2, static_rational<-1>>>::type;

using Nibble =
    make_scaled_unit<Information, scale<2, static_rational<-2>>>::type;

using Bit = make_scaled_unit<Information, scale<2, static_rational<-4>>>::type;

BOOST_UNITS_STATIC_CONSTANT(Word, Information);
BOOST_UNITS_STATIC_CONSTANT(Words, Information);

}  // namespace binary

}  // namespace units

/**
   \brief A helpful type alias for brevity.
 */
using InformationQuantity = units::Quantity<units::binary::Information>;

}  // namespace nebula

#endif  // NEBULA_BINARY_UNITS_HPP_
