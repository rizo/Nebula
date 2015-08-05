/**
   \file units.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Types for dimensional analysis.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_UNITS_HPP_
#define NEBULA_UNITS_HPP_

#pragma once

#include <boost/units/base_dimension.hpp>
#include <boost/units/base_unit.hpp>
#include <boost/units/io.hpp>
#include <boost/units/make_scaled_unit.hpp>
#include <boost/units/make_system.hpp>
#include <boost/units/static_constant.hpp>
#include <boost/units/unit.hpp>

namespace nebula {

namespace units {

struct LengthBaseDimension
    : boost::units::base_dimension<LengthBaseDimension, 2> {};

using LengthDimension = LengthBaseDimension::dimension_type;

/**
   \brief A positive integral quantity with a unit \p T.
 */
template <typename T>
using Quantity = boost::units::quantity<T, std::size_t>;

}  // namespace units

}  // namespace nebula

#endif  // NEBULA_UNITS_HPP_
