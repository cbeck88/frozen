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

#ifndef FROZEN_LETITGO_BITS_ALGORITHMS_H
#define FROZEN_LETITGO_BITS_ALGORITHMS_H

#include <frozen/bits/basic_types.h>
#include <limits>
#include <tuple>
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

template <class T>
constexpr void cswap(T &a, T &b) {
  auto tmp = a;
  a = b;
  b = tmp;
}

template <class T, class U>
constexpr void cswap(std::pair<T, U> & a, std::pair<T, U> & b) {
  cswap(a.first, b.first);
  cswap(a.second, b.second);
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

template <typename Iterator, class Compare>
constexpr void quicksort(Iterator left, Iterator right, Compare const &compare) {
  if (left < right) {
    auto new_pivot = bits::partition(left, right, compare);
    quicksort(left, new_pivot, compare);
    quicksort(new_pivot + 1, right, compare);
  }
}

template <typename T, std::size_t N, class Compare>
constexpr bits::carray<T, N> quicksort(bits::carray<T, N> const &array,
                                     Compare const &compare) {
  bits::carray<T, N> res = array;
  quicksort(res.begin(), res.end() - 1, compare);
  return res;
}

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
#ifdef BRANCHLESS // the compiler is generally smart enough to deduce it
    bool cmp = compare_(*it, value_);
    auto next_it = cmp ? it + 1 : first;
    auto constexpr next_count = (N&1) ? step : (N - (step + 1));
    return doit(next_it, std::integral_constant<std::size_t, next_count>{});
#else
    auto constexpr next_count = (N&1) ? step : (N - (step + 1));
    if (compare_(*it, value_)) {
      return doit(it + 1, std::integral_constant<std::size_t, next_count>{});
    } else {
      return doit(first, std::integral_constant<std::size_t, next_count>{});
    }
#endif
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
