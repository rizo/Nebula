/**
   \file simulation.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Hardware simulated in its own thread.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   Each hardware is simulated in its own independently-running thread. This is a
   little harder to implement, but makes for a more realistic simulation.
 */

#ifndef NEBULA_SIMULATION_HPP_
#define NEBULA_SIMULATION_HPP_

#pragma once

#include <atomic>
#include <future>

namespace nebula {

enum class SimulationStatus {
  Stopped,
  Running
};

/**
   \brief A simulated hardware device.

   Once started, a Simulation can implement any behavior as long terminates via
   stop() when requested to do so. Upon terminated, an arbitrary type (often the
   internal state of the simulation) is returned for inspection.
 */
template <typename StateType>
class Simulation {
 private:
  std::atomic<SimulationStatus> status_;

 protected:
  void notify() { status_.store(SimulationStatus::Running); }

 public:
  /**
     \brief All simulations start as SimulationStatus::Stopped.
   */
  explicit Simulation() : status_{SimulationStatus::Stopped} {}

  virtual ~Simulation() = default;

  SimulationStatus status() const { return status_.load(); }

  void stop() { status_.store(SimulationStatus::Stopped); }

  virtual unique_ptr<StateType> start() = 0;
};

/**
   \brief Launch a simulation asynchronously in a new thread.
 */
template <typename StateType>
std::future<unique_ptr<StateType>> launch(Simulation<StateType>& simulation) {
  return std::async(std::launch::async,
                    [&simulation] { return simulation.start(); });
}

/**
   \brief Check whether a simulation has terminated.

   Calling this function does _not_ impact the SimulationStatus.
 */
template <typename T>
bool isFinished(std::future<T>& future) {
  return future.wait_for(std::chrono::seconds{0}) == std::future_status::ready;
}

}  // namespace nebula

#endif  // NEBULA_SIMULATION_HPP_
