/*
 * Frozen
 * Copyright 2016 QuarksLab
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef FROZEN_LETITGO_ALGORITHMS_H
#define FROZEN_LETITGO_ALGORITHMS_H

#include <array>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

namespace frozen {

namespace bits {

auto constexpr next_highest_power_of_two(std::size_t v) {
  // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
  constexpr auto trip_count = std::numeric_limits<decltype(v)>::digits;
  v--;
  for(std::size_t i = 1; i < trip_count; i <<= 1)
    v |= v >> i;
  v++;
  return v;
}

template <class T, std::size_t N, class Iter, std::size_t... I>
constexpr std::array<T, N> make_unordered_array(Iter &iter,
                                                std::index_sequence<I...>) {
  return {{((void)I, *iter++)...}};
}

template <class T, std::size_t N>
constexpr std::array<T, N>
make_unordered_array(std::initializer_list<T> const values) {
  auto iter = values.begin();
  return make_unordered_array<T, N>(iter, std::make_index_sequence<N>{});
}

// This is std::experimental::to_array
// http://en.cppreference.com/w/cpp/experimental/to_array
template <class T, std::size_t N, std::size_t... I>
constexpr std::array<std::remove_cv_t<T>, N>
    to_array_impl(T (&a)[N], std::index_sequence<I...>)
{
  return { {a[I]...} };
}

template <class T, std::size_t N>
constexpr auto to_array(T (&a)[N]) -> std::array<std::remove_cv_t<T>, N>
{
  return to_array_impl(a, std::make_index_sequence<N>{});
}

template <typename Iter, typename Compare>
constexpr auto min_element(Iter begin, const Iter end,
                           Compare const &compare) {
  auto result = begin;
  while (begin != end) {
    if (compare(*begin, *result)) {
      result = begin;
    }
    ++begin;
  }
  return result;
}

/** Swap **/

template <class T>
constexpr void cswap(T &a, T &b) {
  auto tmp = a;
  a = b;
  b = tmp;
}

template <class... Tys, std::size_t... Is> 
constexpr void cswap(std::tuple<Tys...> &a, std::tuple<Tys...> &b, std::index_sequence<Is...>) {
  using swallow = int[];
  (void) swallow{(cswap(std::get<Is>(a), std::get<Is>(b)), 0)...};
}

template <class... Tys>
constexpr void cswap(std::tuple<Tys...> &a, std::tuple<Tys...> &b) {
  cswap(a, b, std::make_index_sequence<sizeof...(Tys)>());
}

/** Partition **/

template <typename Iterator, class Compare>
constexpr Iterator partition(Iterator left, Iterator right, Compare const &compare) {
  auto pivot = left + (right - left) / 2;
  auto value = *pivot;
  cswap(*right, *pivot);
  for (auto it = left; it < right; ++it) {
    if (compare(*it, value)) {
      cswap(*it, *left);
      left++;
    }
  }
  cswap(*right, *left);
  return left;
}

/** Quicksort **/

/*

template <typename Iterator, class Compare>
constexpr void quicksort(Iterator left, Iterator right, Compare const &compare) {
  if (left < right) {
    auto new_pivot = bits::partition(left, right, compare);
    quicksort(left, new_pivot, compare);
    quicksort(new_pivot + 1, right, compare);
  }
}

template <typename T, std::size_t N, class Compare>
constexpr std::array<T, N> quicksort(std::array<T, N> const &array,
                                     Compare const &compare) {
  std::array<T, N> res = array;
  quicksort(&std::get<0>(res), &std::get<N - 1>(res), compare);
  return res;
}

*/

/** Mergesort **/

// Make an array which is one larger
template <typename T, std::size_t N, std::size_t... Is>
constexpr auto cons_impl(const T & first, const std::array<T, N> & last, std::index_sequence<Is...>)
    -> std::array<T, N+1> {
    return {{ first, last[Is]... }};
}

template <typename T, std::size_t N>
constexpr auto cons(const T & first, const std::array<T, N> & last)
    -> std::array<T, N+1> {
    return cons_impl(first, last, std::make_index_sequence<N>());
}

// Take the trailing part of an array
template <typename T, std::size_t N, std::size_t ... Is>
constexpr auto cdr_impl(const std::array<T, N> & s, std::index_sequence<Is...>)
    -> std::array<T, sizeof...(Is)> {
    return {{s[1 + Is]...}};
}

template <typename T, std::size_t N>
constexpr auto cdr(std::array<T, N> s) -> std::array<T, N - 1> {
    return cdr_impl(s, std::make_index_sequence<N - 1>());
}

// Make an array slice
template <typename T, std::size_t N, std::size_t L, std::size_t ... Is>
constexpr auto slice_impl(const std::array<T, N> & a,
                          std::integral_constant<size_t, L>,
                          std::index_sequence<Is...>) -> std::array<T, sizeof...(Is)> {
    return {{ a[L + Is]... }};
}

template <typename T, std::size_t N, std::size_t L, std::size_t R>
constexpr auto slice(const std::array<T, N> & a,
                     std::integral_constant<std::size_t, L> s,
                     std::integral_constant<std::size_t, R>) {
    return slice_impl(a, s, std::make_index_sequence<R - L>());
}

// Merge two sorted std::arrays

template <typename T, class Compare>
constexpr auto merge_impl( const std::array<T, 0> &, const std::array<T, 0> &, Compare const &)
  -> std::array<T, 0> {
    return {};
}

template <typename T, std::size_t N1, class Compare>
constexpr auto merge_impl( const std::array<T, N1> & s, const std::array<T, 0> &, Compare const &)
    -> std::array<T, N1> {
    return s;
}

template <typename T, std::size_t N2, class Compare>
constexpr auto merge_impl( const std::array<T, 0> &, const std::array<T, N2> & s, Compare const &)
    -> std::array<T, N2> {
    return s;
}

template <typename T, std::size_t N1, std::size_t N2, class Compare>
constexpr auto merge_impl( const std::array<T, N1> & a1, const std::array<T, N2> & a2, Compare const & compare) 
  -> std::array<T, N1 + N2> {
    return compare(a1[0], a2[0]) ?
            cons(a1[0], merge_impl(cdr(a1), a2, compare)) :
            cons(a2[0], merge_impl(a1, cdr(a2), compare));
}

template <typename T, std::size_t N1, std::size_t N2, class Compare>
constexpr auto merge(const std::array<T, N1> & a1, const std::array<T, N2> & a2, Compare const & compare)
    -> std::array<T, N1 + N2> {
    return merge_impl(a1, a2, compare/*, std::make_index_sequence<N1 + N2>()*/);
}



template <class T, class Compare>
constexpr auto mergesort(std::array<T, 0>, Compare const &) -> std::array<T, 0> {
    return {};
}

template <class T, class Compare>
constexpr auto mergesort(std::array<T, 1> s, Compare const &) -> std::array<T, 1> {
    return s;
}

template <class T, std::size_t N, class Compare>
constexpr auto mergesort(const std::array<T, N> & s, Compare const & compare)
    -> std::array<T, N> {
    return merge(mergesort(slice(s,
                                 std::integral_constant<std::size_t, 0>{},
                                 std::integral_constant<std::size_t, N/2 + N % 2>{}),
                           compare),
                 mergesort(slice(s,
                                 std::integral_constant<std::size_t, N/2 + N % 2>{},
                                 std::integral_constant<std::size_t, N>{}),
                           compare),
                 compare);
}

/** Search **/

template <class T, class Compare> struct LowerBound {
  T const &value_;
  Compare const &compare_;
  constexpr LowerBound(T const &value, Compare const &compare)
      : value_(value), compare_(compare) {}

  template <class ForwardIt>
  inline constexpr ForwardIt doit(ForwardIt first,
                                  std::integral_constant<std::size_t, 0>) {
    return first;
  }

  template <class ForwardIt, std::size_t N>
  inline constexpr ForwardIt doit(ForwardIt first,
                                  std::integral_constant<std::size_t, N>) {
    auto constexpr step = N / 2;
    auto it = first + step;
    if (compare_(*it, value_)) {
      auto constexpr next_count = N - (step + 1);
      return doit(it + 1, std::integral_constant<std::size_t, next_count>{});
    } else {
      auto constexpr next_count = step;
      return doit(first, std::integral_constant<std::size_t, next_count>{});
    }
  }
};

template <std::size_t N, class ForwardIt, class T, class Compare>
constexpr ForwardIt lower_bound(ForwardIt first, const T &value,
                                Compare const &compare) {
  return LowerBound<T, Compare>{value, compare}.doit(
      first, std::integral_constant<std::size_t, N>{});
}

template <std::size_t N, class Compare, class ForwardIt, class T>
constexpr bool binary_search(ForwardIt first, const T &value,
                             Compare const &compare) {
  ForwardIt where = lower_bound<N>(first, value, compare);
  return (!(where == first + N) && !(compare(value, *where)));
}

} // namespace bits
} // namespace frozen

#endif
