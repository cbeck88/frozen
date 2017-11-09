#pragma once

namespace frozen {
namespace bits {


/**
 * Get member of array (Work around lack of constexpr in std::array::operator []
 */

template <typename T, std::size_t N>
constexpr decltype(auto) at(std::array<T, N> & a, std::size_t i) {
  auto it * = &std::get<0>(a);
  return it[i];
}

template <typename T, std::size_t N>
constexpr decltype(auto) at(const std::array<T, N> & a, std::size_t i) {
  auto it * = &std::get<0>(a);
  return it[i];
}

/***
 * Constexpr Fold over integer sequence
 * To work around, gcc lack of support for C++14 constexpr for loops
 */

template <typename T, typename V, typename ... Others>
constexpr auto fold_left(T && t, V &&, std::index_sequence<>, Others && ...) {
  return std::forward<T>(t);
}

template <typename T, typename V, std::size_t i, std::size_t... Is, typename ... Others>
constexpr auto fold_left(T && t, V && v, std::index_sequence<i, Is...>, Others && ... os) {
  return fold_left(std::forward<V>(v)(std::forward<T>(t), i, std::forward<Others>(os)...),
                   std::forward<V>(v),
                   std::index_sequence<Is...>(),
                   std::forward<Others>(os)...);
}

// Fold over an array
template <typename V, typename A>
struct array_folder {
  V v;
  A a;

  template <typename T, typename... Others>
  constexpr auto operator()(T && t, std::size_t i, Others && ... os) const {
    return std::forward<V>(v)(std::forward<T>(t), at(std::forward<A>(a), i), std::forward<Others>(os)...); 
  }
};

template <typename T, typename V, typename U, std::size_t N, typename... Others>
constexpr auto fold_left_array(T && t, V && v, const std::array<U, N> & a, Others && ... os) {
  return fold_left(std::forward<T>(t), array_folder<V, decltype(a)>{std::forward<V>(v), a}, std::make_index_sequence<N>(),
                    std::forward<Others>(os)...);
}

// Map over an array
template <typename U, typename V, typename T, std::size_t N, std::size_t ... Is, typename ... Others>
constexpr std::array<U, N> map_array_impl(V && v, const std::array<T, N> & a, std::index_sequence<Is...>, Others && ... os) {
  return {{ (std::forward<V>(v)(at(a, Is), std::forward<Others>(os)...)... }};
}


template <typename U, typename V, typename T, std::size_t N, typename ... Others>
constexpr std::array<U, N> map_array(V && v, const std::array<T, N> & a, Others && ... os) {
  return map_array_impl(std::forward<V>(v), a, std::make_index_sequence<N>(), std::forward<Others>(os)...);
}

// Find first in an array

struct find_first_helper {
  template <typename P, typename A>
  constexpr unsigned operator()(unsigned l, unsigned i, P && p, const A & a) const {
    return l <= i ? l :
                p(at(a, i)) ? i : l;
  }
};

template <typename P, typename T, std::size_t N>
constexpr unsigned find_first(P && pred, const std::array<T, N> & a) {
  return fold_left(N, find_first_helper{}, std::make_index_sequence<N>(), pred, a);
}

// Count number matching a predicate

template <typename P, typename T, std::size_t N>
constexpr unsigned count_with(P && pred, const std::array<T, N> & a) {
  return fold_left_array(0,
                         std::add<unsigned>{},
                         map_array<bool>(std::forward<Pred>(pred), a));
}

} // end namespace bits
} // end namespace frozen
