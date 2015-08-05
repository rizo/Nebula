/**
   \file interrupt.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Hardware and software interrupts.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_INTERRUPT_HPP_
#define NEBULA_INTERRUPT_HPP_

#pragma once

#include "concurrent-queue.hpp"
#include "processor-state.hpp"
#include "simulation.hpp"

#include <condition_variable>
#include <mutex>

DEFINE_LOGGER(INTERRUPT)

namespace nebula {

namespace error {

class CaughtFire : std::runtime_error {
 public:
  explicit CaughtFire() : std::runtime_error{"The DCPU-16 caught fire!"} {}
};

}  // namespace error

class InterruptQueue {
 private:
  detail::ConcurrentQueue<Word> queue_{};
  const std::size_t max_queued_interrupts_;

 public:
  explicit InterruptQueue(std::size_t max_queued_interrupts)
      : max_queued_interrupts_{max_queued_interrupts} {}

  void push(Word signal);

  optional<Word> pop();
};

/**
   \brief Hardware and software interrupts.

   Interrupts are the main method by which the processor communicates to
   hardware devices.

   When two devices are connected via an interrupt, communication proceeds in a
   single direction. An interrupt is therefore shared between an
   InterruptSource, which causes sends interrupts, and a InterruptSink, which
   receives them.

   These two parts of the interrupt manage control flow over the ProcessorState.
   Control flow proceeds as follows:

   1. The holder of an InterruptSink invokes InterruptSink::waitForTrigger()
   to wait for an incoming interrupt. Execution will pause until an interrupt is
   recieved.
   2. The holder of an InterruptSource
   triggers an interrupt via InterruptSource::trigger() and passes in exclusive
   ownership of the ProcessorState, effectively yielding control of the
   processor. The source then invokes InterruptSource::waitForResponse() which
   pauses execution.
   3. The sink can now invoke InterruptSink::processorState() to read and modify
   the processor with confidence that it has exclusive access (ownership).
   4. When finished handling the interrupt, the sink invokes
   InterruptSink::respond() to return control to the InterruptSource.
   5. Execution resumes at the source with the new processor state.

   \note This class acts mostly as internal shared state between InterruptSink
   and InterruptSource and should therefore not be instantiated directly.
 */
class Interrupt {
 private:
  unique_ptr<ProcessorState> state_;
  std::atomic<bool> is_active_;
  std::mutex mutex_;
  std::condition_variable condition_;

 public:
  explicit Interrupt()
      : state_{nullptr}, is_active_{false}, mutex_{}, condition_{} {}

  friend class InterruptSource;
  friend class InterruptSink;
};

/**
   \brief A source of new interrupts.
   \see Interrupt
 */
class InterruptSource {
 private:
  shared_ptr<Interrupt> interrupt_;

 public:
  explicit InterruptSource(shared_ptr<Interrupt> interrupt)
      : interrupt_{interrupt} {}

  /**
     \brief Trigger a new interrupt.

     Transfer control of the ProcessorState to a waiting InterruptSink.
     \see waitForResponse()
   */
  void trigger(unique_ptr<ProcessorState> state);

  /**
     \brief Wait for a return of control.

     Invoking waitForResponse() after invoking trigger() will cause execution to
     block until the InterruptSink paired with this InterruptSource has
     relinquished control of the processor.

     \see trigger()
   */
  unique_ptr<ProcessorState> waitForResponse();
};

class InterruptSink {
 private:
  shared_ptr<Interrupt> interrupt_;

 public:
  explicit InterruptSink(shared_ptr<Interrupt> interrupt)
      : interrupt_{interrupt} {}

  /**
     \brief Access the processor state.

     \note If the InterruptSink hasn't yet be given control over the
     ProcessorState via a InterruptSource, then a null pointer dereference will
     take place.
   */
  inline ProcessorState& processorState() { return *interrupt_->state_; }

  /**
     \brief Wait for an interrupt to be triggered.

     Block execution until the InterruptSource paired with this InterruptSink
     has triggered an interrupt.

     \see InterruptSource::trigger()
   */
  void waitForTrigger();

  /**
     \brief Like waitForTrigger(), but non-blocking.
   */
  inline bool isActive() { return interrupt_->is_active_.load(); }

  /**
     \brief Finish handling an interupt.

     After the InterruptSink has dealt with the interrupt appropriately, yield
     control of the ProcessorState back to the InterruptSource.
   */
  void respond();

  /**
     \brief Wait for either a trigger or simulation termination.
   */
  template <typename StateType>
  void waitForTriggerOrDeath(Simulation<StateType>& simulation) {
    std::unique_lock<std::mutex> lock{interrupt_->mutex_};

    while (true) {
      if (interrupt_->condition_.wait_for(lock,
                                          std::chrono::milliseconds{5u},
                                          [&] { return isActive(); })) {
        // Time-out on the condition, and the interrupt is active.
        break;
      } else {
        // Make sure the simulation is still running.
        if (simulation.status() != SimulationStatus::Running) {
          break;
        }
      }
    }
  }
};

}  // namespace nebula

#endif  // NEBULA_INTERRUPT_HPP_
