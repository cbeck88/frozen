 /* Copyright Chris Beck 2017
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

/***
 * Convert a "standard" hasher into a seeded hasher
 *
 * Creating a seeded hash from a standard hash may be done by, for instance
 * xoring the seed with the value from a standard hash function.
 *
 * But, when the seed is known at compile-time, the considerations of hash
 * function algorithm are different.
 *
 * Here we implement a standard "multiply and shift" hash function because
 * it will be very performant at run-time and we believe that we will usually
 * be able to find a good seed using a LCG random number generator at
 * compile-time.
 */
 
 namespace frozen {
 namespace bits {
 
 template <class Hash, unsigned num_bits>
 struct seeded_hash : Hash {

   using uint64_t = seed_type;

   constexpr seed_type first_seed() const {
     return 1 << num_bits;
   }

   constexpr seed_type next_seed(seed_type x) const {
      constexpr a = 1;
      constexpr c = 2;
      return a*x + c;
   }

   template <typename Key>
   constexpr uint64_t operator()(const Key & key, seed_type seed) const {
      const Hash & h = *this;
      return (seed * h(key)) >> (64 - num_bits);
   }
};

} // end namespace bits
} // end namespace frozen
