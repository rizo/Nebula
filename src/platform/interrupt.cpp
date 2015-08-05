/**
   \file interrupt.cpp
   \copyright 2014 Jesse Haber-Kucharsky

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#include "interrupt.hpp"

namespace nebula {

//
// InterruptQueue
//

void InterruptQueue::push(Word signal) {
  if (queue_.size() >= max_queued_interrupts_) {
    LOG(INTERRUPT, error) << "Exceeded maximum queued interrupts.";
    throw error::CaughtFire{};
  }

  queue_.push(signal);

  LOG(INTERRUPT, info)
      << format("Added interrupt with signal 0x%04x to the queue.") % signal;
}

optional<Word> InterruptQueue::pop() {
  LOG(INTERRUPT, debug) << "Popped interrupt from queue.";
  return queue_.pop();
}

//
// InterruptSource
//

void InterruptSource::trigger(unique_ptr<ProcessorState> state) {
  interrupt_->is_active_.store(true);
  interrupt_->state_ = std::move(state);
  interrupt_->condition_.notify_one();
}

unique_ptr<ProcessorState> InterruptSource::waitForResponse() {
  std::unique_lock<std::mutex> lock{interrupt_->mutex_};

  while (!((interrupt_->state_ != nullptr) && !interrupt_->is_active_.load())) {
    interrupt_->condition_.wait(lock);
  }

  return std::move(interrupt_->state_);
}

//
// InterruptSink
//

void InterruptSink::waitForTrigger() {
  std::unique_lock<std::mutex> lock{interrupt_->mutex_};

  while (!isActive()) {
    interrupt_->condition_.wait(lock);
  }
}

void InterruptSink::respond() {
  interrupt_->is_active_.store(false);
  interrupt_->condition_.notify_one();
}

}  // namespace nebula
