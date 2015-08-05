/**
   \file monitor-units.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Dimensional units for the virtual monitor.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   \see \p /doc/hw/monitor.txt
   \see monitor.hpp
 */

#ifndef NEBULA_DEVICE_MONITOR_UNITS_HPP_
#define NEBULA_DEVICE_MONITOR_UNITS_HPP_

#pragma once

#include <platform/graphics-units.hpp>

namespace nebula {

namespace units {

namespace simulated {

using namespace boost::units;

struct PixelWidthBaseUnit : base_unit<PixelWidthBaseUnit, LengthDimension, 3> {
};
using SimulatedWidthSystem = make_system<PixelWidthBaseUnit>::type;
/**
   \brief The width of a single virtual pixel.
 */
using Width = unit<LengthDimension, SimulatedWidthSystem>;

BOOST_UNITS_STATIC_CONSTANT(PixelWidth, Width);
BOOST_UNITS_STATIC_CONSTANT(PixelWidths, Width);

struct PixelHeightBaseUnit
    : base_unit<PixelHeightBaseUnit, LengthDimension, 4> {};
using SimulatedHeightSystem = make_system<PixelHeightBaseUnit>::type;
/**
   \brief The height of a single virtual pixel.
 */
using Height = unit<LengthDimension, SimulatedHeightSystem>;

BOOST_UNITS_STATIC_CONSTANT(PixelHeight, Height);
BOOST_UNITS_STATIC_CONSTANT(PixelHeights, Height);

}  // namespace simulated

}  // namespace units

inline units::Quantity<units::simulated::Width> operator"" _spw(
    unsigned long long magnitude) {
  return magnitude * units::simulated::PixelWidth;
}

inline units::Quantity<units::simulated::Height> operator"" _sph(
    unsigned long long magnitude) {
  return magnitude * units::simulated::PixelHeight;
}

const auto kCellWidth = 4_spw;
const auto kCellHeight = 8_sph;

const std::size_t kCellsPerScreenWidth = 32u;
const std::size_t kCellsPerScreenHeight = 12u;

const auto kScreenWidth = kCellWidth * kCellsPerScreenWidth;
const auto kScreenHeight = kCellHeight * kCellsPerScreenHeight;

// This is the width (in real pixels) of the simulated monitor screen.
//
// Must be divisible by the magnitude of 'kScreenWidth'.
const auto kScreenHorizontalResolution = 640_px;

// This is the height (in real pixels) of the simulated monitor screen.
//
// Must be divisible by the magnitude of 'kScreenHeight'.
const auto kScreenVerticalResolution = 480_px;

}  // namespace nebula

/**
   \brief The number of real pixels in a simulated pixel height.
 */
BOOST_UNITS_DEFINE_CONVERSION_FACTOR(
    nebula::units::simulated::PixelHeightBaseUnit,
    nebula::units::real::PixelBaseUnit,
    std::size_t,
    nebula::kScreenVerticalResolution.value() / nebula::kScreenHeight.value());

/**
   \brief The number of real pixels in a simulated pixel width.
 */
BOOST_UNITS_DEFINE_CONVERSION_FACTOR(
    nebula::units::simulated::PixelWidthBaseUnit,
    nebula::units::real::PixelBaseUnit,
    std::size_t,
    nebula::kScreenHorizontalResolution.value() / nebula::kScreenWidth.value());

namespace nebula {

/**
   \brief The size of the monitor border on the sides.
 */
const units::Quantity<units::real::Length> kBorderWidth{2_spw};

/**
   \brief The size of the monitor border on the top and bottom.
 */
const units::Quantity<units::real::Length> kBorderHeight{2_sph};

/**
   \brief The width (in real pixels) of the virtual monitor.
   
   Including the border.
 */
const auto kWindowHorizontalResolution =
    kScreenHorizontalResolution + (kBorderWidth + kBorderWidth);

/**
   \brief The height (in real pixels) of the virtual monitor.

   Including the border.
 */
const auto kWindowVerticalResolution =
    kScreenVerticalResolution + (kBorderHeight + kBorderHeight);

}  // namespace nebula

#endif  // NEBULA_DEVICE_MONITOR_UNITS_HPP_
