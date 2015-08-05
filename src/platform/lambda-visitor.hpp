/**
   \file lambda-visitor.hpp
   \copyright 2014 Jesse Haber-Kucharsky

   \brief A Boost.Variant visitor with dispatch on lambdas.

   \note Released under the terms of the Apache License Version 2.0. See
   \p /nebula/LICENSE or http://www.apache.org/licenses/

   StackOverflow.com is mostly to thank for this atrocity. It's pretty cool
   though. A good approximation of pattern matching on algabraic
   types.

   See event.hpp and the event-handling code in main.cpp for a good example of
   its use.
 */

#ifndef NEBULA_DETAIL_LAMBDA_VISITOR_HPP_
#define NEBULA_DETAIL_LAMBDA_VISITOR_HPP_

#pragma once

#include <boost/variant.hpp>

namespace nebula {

/**
   \brief Internal implementation details.
 */
namespace detail {

template <typename ReturnType, typename... Lambdas>
struct LambdaVisitor;

template <typename ReturnType, typename FirstLambda, typename... Lambdas>
struct LambdaVisitor<ReturnType,
                     FirstLambda,
                     Lambdas...> : public LambdaVisitor<ReturnType, Lambdas...>,
                                   public FirstLambda {
  using FirstLambda::operator();
  using LambdaVisitor<ReturnType, Lambdas...>::operator();

  LambdaVisitor(FirstLambda first_lambda, Lambdas... lambdas)
      : LambdaVisitor<ReturnType, Lambdas...>(lambdas...),
        FirstLambda(first_lambda) {}
};

template <typename ReturnType, typename FirstLambda>
struct LambdaVisitor<ReturnType,
                     FirstLambda> : public boost::static_visitor<ReturnType>,
                                    public FirstLambda {
  using FirstLambda::operator();

  LambdaVisitor(FirstLambda first_lambda)
      : boost::static_visitor<ReturnType>{}, FirstLambda(first_lambda) {}
};

template <typename ReturnType>
struct LambdaVisitor<ReturnType> : public boost::static_visitor<ReturnType> {
  LambdaVisitor() : boost::static_visitor<ReturnType>{} {};
};

template <typename ReturnType, typename... Lambdas>
LambdaVisitor<ReturnType, Lambdas...> makeLambdaVisitor(Lambdas... lambdas) {
  return {lambdas...};
}

}  // namespace detail

}  // namespace nebula

#endif  // NEBULA_DETAIL_LAMBDA_VISITOR_HPP_
