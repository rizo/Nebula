/**
   \file graphics-units.hpp
   \copyright 2014 Jesse Haber-Kucharsky
   
   \brief Dimensional units for physical displays.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_GRAPHICS_UNITS_HPP_
#define NEBULA_GRAPHICS_UNITS_HPP_

#pragma once

#include "units.hpp"

namespace nebula {

namespace units {

/**
   \brief Units for real physical hardware.
 */
namespace real {

using namespace boost::units;

struct PixelBaseUnit : base_unit<PixelBaseUnit, LengthDimension, 2> {};

using RealSystem = make_system<PixelBaseUnit>::type;

using Dimensionless = unit<dimensionless_type, RealSystem>;
using Length = unit<LengthDimension, RealSystem>;

BOOST_UNITS_STATIC_CONSTANT(Pixel, Length);
BOOST_UNITS_STATIC_CONSTANT(Pixels, Length);

}  // namespace real

}  // namespace units

inline units::Quantity<units::real::Length> operator"" _px(
    unsigned long long magnitude) {
  return magnitude * units::real::Pixel;
}

}  // namespace nebula

#endif  // NEBULA_GRAPHICS_UNITS_HPP_
