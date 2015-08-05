/**
   \file event.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief Handle user I/O events.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   The word "event" here refers not to an event in the simulated DCPU16
   computer, but an I/O event related to Nebula itself. Events can happen at
   unpredictable times. To check whether an event has occurred, use
   event::poll().
 */

#ifndef NEBULA_EVENT_HPP_
#define NEBULA_EVENT_HPP_

#pragma once

#include "lambda-visitor.hpp"
#include "prelude.hpp"

namespace nebula {

namespace event {

/**
   \brief A request has been made to terminate Nebula.
 */
struct Quit {};

/**
   \brief The user has entered a key with their physical keyboard.
 */
struct KeyInput {
  Word code;
};

using Event = boost::variant<Quit, KeyInput>;

/**
   \brief Poll for the occurance of a new user I/O Event.
 */
optional<Event> poll();

/**
   \brief Dispatch table for events.

   Create an EventHandler via makeHandler() to avoid a long template-y
   signature.
 */
template <typename... Handlers>
using EventHandler = detail::LambdaVisitor<void, Handlers...>;

/**
   \brief Specify how to handle events.

   The signature of this function is scary, but \p handlers is just a
   comma-separated list of callable objects (lambda functions!), which each has
   a single argument of a specific event type. Every event type has to be
   explicitly handled or there will be a (unclear) compilation error.
 */
template <typename... Handlers>
EventHandler<Handlers...> makeHandler(Handlers... handlers) {
  return detail::makeLambdaVisitor<void>(handlers...);
}

/**
   \brief Dispatch on a specific Event given an EventHandler.
 */
template <typename... Handlers>
void handle(const EventHandler<Handlers...>& handler, const Event& event) {
  return boost::apply_visitor(handler, event);
}

}  // namespace event

}  // namespace nebula

#endif  // NEBULA_EVENT_HPP_
