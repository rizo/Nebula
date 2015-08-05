/**
   \file cycle-cost.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Operations with simulated latency.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_CYCLE_COST_HPP_
#define NEBULA_CYCLE_COST_HPP_

#pragma once

#include "prelude.hpp"

namespace nebula {

/**
   \brief Simulated latency.

   Classes which implement cycleCost() indicate that they have a latency of some
   amount of clock cycles. These clock cycles are purely logical, but can be
   interpretted in different ways (for example, depending on the simulated
   running frequency of the processor).
 */
class ICycleCost {
 public:
  virtual std::size_t cycleCost() const = 0;

  virtual ~ICycleCost() = default;
};

}  // namespace nebula

#endif  // NEBULA_CYCLE_COST_HPP_
