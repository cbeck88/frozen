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

#ifndef FROZEN_LETITGO_UNORDERED_MAP_H
#define FROZEN_LETITGO_UNORDERED_MAP_H
#include <array>
#include <frozen/bits/elsa.h>
#include <frozen/bits/pmh.h>
#include <tuple>
#include <functional>

namespace frozen {

namespace bits {

struct GetKey {
  template <class KV> constexpr auto const &operator()(KV const &kv) const {
    return kv.first;
  }
};

} // namespace bits

template <class Key, class Value, std::size_t N, typename Hash = anna<Key>,
          class KeyEqual = std::equal_to<Key>>
class unordered_map {
  static constexpr std::size_t storage_size =
      bits::next_highest_power_of_two(N) * (N < 32 ? 2 : 1); // size adjustment to prevent high collision rate for small sets
  using container_type = std::array<std::pair<Key, Value>, N>;
  using tables_type = bits::pmh_tables<storage_size, Hash>;

  KeyEqual const equal_;
  container_type items_;
  tables_type tables_;

public:
  /* typedefs */
  using key_type = Key;
  using value_type = Value;
  using size_type = typename container_type::size_type;
  using difference_type = typename container_type::difference_type;
  using hasher = Hash;
  using key_equal = KeyEqual;
  using const_reference = typename container_type::const_reference;
  using reference = const_reference;
  using const_pointer = typename container_type::const_pointer;
  using pointer = const_pointer;
  using const_iterator = const_pointer;
  using iterator = const_iterator;

public:
  /* constructors */
  unordered_map(unordered_map const &) = default;
  constexpr unordered_map(std::initializer_list<std::pair<Key, Value>> items,
                          Hash const &hash, KeyEqual const &equal)
      : equal_{equal}
      , items_{bits::make_unordered_array<std::pair<Key, Value>, N>(items)}
      , tables_{
            bits::make_pmh_tables<std::pair<Key, Value>, N, storage_size>(
                items_,
                hash, bits::GetKey{})} {}
  constexpr unordered_map(std::initializer_list<std::pair<Key, Value>> items)
      : unordered_map{items, Hash{}, KeyEqual{}} {}

  /* iterators */
  const_iterator begin() const { return items_.begin(); }
  const_iterator end() const { return items_.end(); }
  const_iterator cbegin() const { return items_.cbegin(); }
  const_iterator cend() const { return items_.cend(); }

  /* capacity */
  constexpr bool empty() const { return !N; }
  constexpr size_type size() const { return N; }
  constexpr size_type max_size() const { return N; }

  /* lookup */
  constexpr std::size_t count(Key const &key) const {
    auto const &kv = lookup(key);
    return equal_(kv.first, key);
  }

  constexpr Value const &at(Key const &key) const {
    auto const &kv = lookup(key);
    if (equal_(kv.first, key))
      return kv.second;
    else
      throw std::out_of_range("unknown key");
  }

  const_iterator find(Key const &key) const {
    auto const &kv = lookup(key);
    if (equal_(kv.first, key))
      return &kv;
    else
      return items_.end();
  }

  std::pair<const_iterator, const_iterator> equal_range(Key const &key) const {
    auto const &kv = lookup(key);
    if (equal_(kv.first, key))
      return {&kv, &kv + 1};
    else
      return {items_.end(), items_.end()};
  }

  /* bucket interface */
  constexpr std::size_t bucket_count() const { return storage_size; }
  constexpr std::size_t max_bucket_count() const { return storage_size; }

  /* observers*/
  constexpr hasher hash_function() const { return tables_.hash_; }
  constexpr key_equal key_eq() const { return equal_; }

private:
  constexpr auto const &lookup(Key const &key) const {
    return items_[tables_.lookup(key)];
  }
};
} // namespace frozen

#endif
