/**
   \file concurrent-queue.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief A simple concurrent wrapper over a std::queue.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/
 */

#ifndef NEBULA_CONCURRENT_QUEUE_HPP_
#define NEBULA_CONCURRENT_QUEUE_HPP_

#include "prelude.hpp"

#include <queue>
#include <mutex>

namespace nebula {

namespace detail {

template <typename T>
class ConcurrentQueue {
 private:
  std::queue<T> queue_{};
  std::mutex mutex_{};

 public:
  void push(const T& element) {
    std::lock_guard<std::mutex> lock{mutex_};
    queue_.push(element);
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    std::lock_guard<std::mutex> lock{mutex_};
    queue_.emplace(std::forward<Args>(args)...);
  }

  optional<T> pop() {
    std::lock_guard<std::mutex> lock{mutex_};

    if (queue_.empty()) {
      return {};
    }

    T front_element = std::move(queue_.front());
    queue_.pop();
    return front_element;
  }

  bool isReady() { return !queue_.empty(); }

  std::size_t size() const { return queue_.size(); }

  optional<T&> top() {
    if (!isReady()) {
      return {};
    } else {
      return queue_.front();
    }
  }
};

}  // namespace detail

}  // namespace nebula

#endif  // NEBULA_CONCURRENT_QUEUE_HPP_
