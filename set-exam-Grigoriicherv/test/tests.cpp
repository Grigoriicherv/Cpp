#include "element.h"
#include "fault-injection.h"
#include "set.h"
#include "test-utils.h"

#include <gtest/gtest.h>

#include <iterator>
#include <random>
#include <type_traits>

static_assert(!std::is_constructible_v<container::iterator, std::nullptr_t>,
              "iterator should not be constructible from nullptr");
static_assert(!std::is_constructible_v<container::const_iterator, std::nullptr_t>,
              "const_iterator should not be constructible from nullptr");
static_assert(!std::is_constructible_v<container::reverse_iterator, std::nullptr_t>,
              "reverse_iterator should not be constructible from nullptr");
static_assert(!std::is_constructible_v<container::const_reverse_iterator, std::nullptr_t>,
              "const_reverse_iterator should not be constructible from nullptr");

namespace {

class correctness_test : public base_test {};

class exception_safety_test : public base_test {};

class performance_test : public base_test {};

class random_test : public base_test {};

[[maybe_unused]] void magic([[maybe_unused]] element& c) {
  c = 42;
}

void magic(const element&) {}

} // namespace

TEST_F(correctness_test, default_ctor) {
  container c;
  expect_empty(c);
  instances_guard.expect_no_instances();
}

TEST_F(correctness_test, insert_single_element) {
  container c;
  c.insert(42);
  expect_eq(c, {42});
}

TEST_F(correctness_test, insert_ascending) {
  container c;
  mass_insert(c, {1, 2, 3, 4});
  expect_eq(c, {1, 2, 3, 4});
}

TEST_F(correctness_test, insert_descending) {
  container c;
  mass_insert(c, {4, 3, 2, 1});
  expect_eq(c, {1, 2, 3, 4});
}

TEST_F(correctness_test, insert_shuffled_1) {
  container c;
  mass_insert(c, {2, 1, 3, 4});
  expect_eq(c, {1, 2, 3, 4});
}

TEST_F(correctness_test, insert_shuffled_2) {
  container c;
  mass_insert(c, {4, 2, 1, 5, 3});
  expect_eq(c, {1, 2, 3, 4, 5});
}

TEST_F(correctness_test, insert_shuffled_3) {
  container c;
  mass_insert(c, {2, 1, 5, 3, 4});
  expect_eq(c, {1, 2, 3, 4, 5});
}

TEST_F(correctness_test, insert_twice) {
  container c;
  c.insert(42);
  c.insert(42);
  expect_eq(c, {42});
}

TEST_F(correctness_test, insert_duplicates) {
  container c;
  mass_insert(c, {8, 4, 2, 4, 4, 4, 8});
  expect_eq(c, {2, 4, 8});
}

TEST_F(correctness_test, insert_iterators_1) {
  container s;
  container::iterator i = s.end();

  s.insert(42);
  --i;
  EXPECT_EQ(42, *i);
}

TEST_F(correctness_test, insert_iterators_2) {
  container c;
  mass_insert(c, {8, 2, 5, 10, 3, 1, 9});

  container::iterator i = c.find(5);
  container::iterator j = c.find(8);

  c.insert(7);
  EXPECT_EQ(5, *i);
  EXPECT_EQ(8, *j);
  EXPECT_EQ(7, *std::next(i));
  EXPECT_EQ(7, *std::prev(j));
}

TEST_F(correctness_test, insert_return_value) {
  container c;
  mass_insert(c, {8, 2, 5, 10, 3, 1, 9});

  auto [it, ins] = c.insert(7);
  EXPECT_TRUE(ins);
  EXPECT_EQ(7, *it);
  EXPECT_EQ(5, *std::prev(it));
  EXPECT_EQ(8, *std::next(it));
}

TEST_F(correctness_test, insert_duplicate_return_value) {
  container c;
  mass_insert(c, {8, 2, 5, 10, 7, 3, 1, 9});

  auto [it, ins] = c.insert(7);
  EXPECT_FALSE(ins);
  EXPECT_EQ(7, *it);
  EXPECT_EQ(5, *std::prev(it));
  EXPECT_EQ(8, *std::next(it));
}

TEST_F(correctness_test, reinsert) {
  container c;
  mass_insert(c, {6, 2, 3, 1, 9, 8});
  c.erase(c.find(6));
  c.insert(6);
  expect_eq(c, {1, 2, 3, 6, 8, 9});
}

TEST_F(correctness_test, copy_ctor_ascending) {
  container c;
  mass_insert(c, {1, 2, 3, 4});

  container c2 = c;
  expect_eq(c2, {1, 2, 3, 4});
}

TEST_F(correctness_test, copy_ctor_descending) {
  container c;
  mass_insert(c, {4, 3, 2, 1});

  container c2 = c;
  expect_eq(c2, {1, 2, 3, 4});
}

TEST_F(correctness_test, copy_ctor_shuffled) {
  container c;
  mass_insert(c, {8, 4, 2, 10, 5});

  container c2 = c;
  expect_eq(c2, {2, 4, 5, 8, 10});
}

TEST_F(correctness_test, copy_ctor_empty) {
  container c;
  container c2 = c;
  expect_empty(c2);
}

TEST_F(correctness_test, copy_assignment) {
  container c;
  mass_insert(c, {1, 2, 3, 4});

  container c2;
  mass_insert(c2, {5, 6, 7, 8});

  c2 = c;
  expect_eq(c2, {1, 2, 3, 4});
}

TEST_F(correctness_test, copy_assignment_empty) {
  container c;

  container c2;
  mass_insert(c2, {1, 2, 3, 4});

  c2 = c;
  expect_empty(c2);
}

TEST_F(correctness_test, copy_assignment_self) {
  container c;
  mass_insert(c, {1, 2, 3, 4});

  c = c;
  expect_eq(c, {1, 2, 3, 4});
}

TEST_F(correctness_test, copy_assignment_self_empty) {
  container c;
  c = c;
  expect_empty(c);
}

TEST_F(correctness_test, swap) {
  container c1, c2;
  mass_insert(c1, {1, 2, 3, 4});
  mass_insert(c2, {5, 6, 7, 8, 9});

  swap(c1, c2);
  expect_eq(c1, {5, 6, 7, 8, 9});
  expect_eq(c2, {1, 2, 3, 4});
}

TEST_F(correctness_test, swap_self) {
  container c1;
  mass_insert(c1, {1, 2, 3, 4});

  swap(c1, c1);
  expect_eq(c1, {1, 2, 3, 4});
}

TEST_F(correctness_test, swap_empty) {
  container c1, c2;
  mass_insert(c1, {1, 2, 3, 4});

  swap(c1, c2);
  expect_empty(c1);
  expect_eq(c2, {1, 2, 3, 4});

  swap(c1, c2);
  expect_eq(c1, {1, 2, 3, 4});
  expect_empty(c2);
}

TEST_F(correctness_test, swap_empty_empty) {
  container c1, c2;
  swap(c1, c2);
  expect_empty(c1);
  expect_empty(c2);
}

TEST_F(correctness_test, swap_empty_self) {
  container c1;
  swap(c1, c1);
  expect_empty(c1);
}

TEST_F(correctness_test, swap_iterators) {
  container c1, c2;
  mass_insert(c1, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
  c2.insert(11);

  container::const_iterator c1_begin = c1.begin();
  container::const_iterator c1_end = c1.end();

  container::const_iterator c2_begin = c2.begin();
  container::const_iterator c2_end = c2.end();

  swap(c1, c2);

  EXPECT_EQ(c1_end, c1.end());
  EXPECT_EQ(c2_end, c2.end());

  EXPECT_EQ(1, *c1_begin++);
  EXPECT_EQ(2, *c1_begin++);
  EXPECT_EQ(3, *c1_begin++);

  std::advance(c1_begin, 7);
  EXPECT_EQ(c2_end, c1_begin);

  EXPECT_EQ(11, *c2_begin++);
  EXPECT_EQ(c1_end, c2_begin);
}

TEST_F(correctness_test, empty) {
  container c;
  expect_empty(c);

  c.insert(1);
  EXPECT_FALSE(c.empty());
  EXPECT_NE(0, c.size());
  EXPECT_NE(c.begin(), c.end());

  c.erase(1);
  expect_empty(c);
}

TEST_F(correctness_test, size) {
  container c;
  EXPECT_EQ(0, c.size());
  c.insert(1);
  EXPECT_EQ(1, c.size());
  c.insert(2);
  EXPECT_EQ(2, c.size());
  c.insert(2);
  EXPECT_EQ(2, c.size());
  c.erase(1);
  EXPECT_EQ(1, c.size());
  c.erase(1);
  EXPECT_EQ(1, c.size());
  c.erase(2);
  EXPECT_EQ(0, c.size());
}

TEST_F(correctness_test, iterator_conversions) {
  container c;
  container::const_iterator i1 = c.begin();
  container::iterator i2 = c.end();

  EXPECT_TRUE(i1 == i1);
  EXPECT_TRUE(i1 == i2);
  EXPECT_TRUE(i2 == i1);
  EXPECT_TRUE(i2 == i2);
  EXPECT_FALSE(i1 != i1);
  EXPECT_FALSE(i1 != i2);
  EXPECT_FALSE(i2 != i1);
  EXPECT_FALSE(i2 != i2);

  EXPECT_TRUE(std::as_const(i1) == i1);
  EXPECT_TRUE(std::as_const(i1) == i2);
  EXPECT_TRUE(std::as_const(i2) == i1);
  EXPECT_TRUE(std::as_const(i2) == i2);
  EXPECT_FALSE(std::as_const(i1) != i1);
  EXPECT_FALSE(std::as_const(i1) != i2);
  EXPECT_FALSE(std::as_const(i2) != i1);
  EXPECT_FALSE(std::as_const(i2) != i2);

  EXPECT_TRUE(i1 == std::as_const(i1));
  EXPECT_TRUE(i1 == std::as_const(i2));
  EXPECT_TRUE(i2 == std::as_const(i1));
  EXPECT_TRUE(i2 == std::as_const(i2));
  EXPECT_FALSE(i1 != std::as_const(i1));
  EXPECT_FALSE(i1 != std::as_const(i2));
  EXPECT_FALSE(i2 != std::as_const(i1));
  EXPECT_FALSE(i2 != std::as_const(i2));

  EXPECT_TRUE(std::as_const(i1) == std::as_const(i1));
  EXPECT_TRUE(std::as_const(i1) == std::as_const(i2));
  EXPECT_TRUE(std::as_const(i2) == std::as_const(i1));
  EXPECT_TRUE(std::as_const(i2) == std::as_const(i2));
  EXPECT_FALSE(std::as_const(i1) != std::as_const(i1));
  EXPECT_FALSE(std::as_const(i1) != std::as_const(i2));
  EXPECT_FALSE(std::as_const(i2) != std::as_const(i1));
  EXPECT_FALSE(std::as_const(i2) != std::as_const(i2));
}

TEST_F(correctness_test, iterator_increment_1) {
  container c;
  mass_insert(c, {5, 3, 8, 1, 2, 6, 7, 10});

  container::iterator i = c.begin();
  EXPECT_EQ(1, *i);
  EXPECT_EQ(2, *++i);
  EXPECT_EQ(3, *++i);
  EXPECT_EQ(5, *++i);
  EXPECT_EQ(6, *++i);
  EXPECT_EQ(7, *++i);
  EXPECT_EQ(8, *++i);
  EXPECT_EQ(10, *++i);
  EXPECT_EQ(c.end(), ++i);
}

TEST_F(correctness_test, iterator_increment_2) {
  container c;
  mass_insert(c, {5, 2, 10, 9, 12, 7});

  container::iterator i = c.begin();
  EXPECT_EQ(2, *i);
  EXPECT_EQ(5, *++i);
  EXPECT_EQ(7, *++i);
  EXPECT_EQ(9, *++i);
  EXPECT_EQ(10, *++i);
  EXPECT_EQ(12, *++i);
  EXPECT_EQ(c.end(), ++i);
}

TEST_F(correctness_test, iterator_increment_3) {
  container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  container::iterator i = std::next(c.begin(), 3);
  ++ ++i;
  EXPECT_EQ(6, *i);
}

TEST_F(correctness_test, iterator_increment_3c) {
  container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  container::const_iterator i = std::next(c.begin(), 3);
  ++ ++i;
  EXPECT_EQ(6, *i);
}

TEST_F(correctness_test, iterator_decrement_1) {
  container s;
  mass_insert(s, {5, 3, 8, 1, 2, 6, 7, 10});

  container::iterator i = s.end();
  EXPECT_EQ(10, *--i);
  EXPECT_EQ(8, *--i);
  EXPECT_EQ(7, *--i);
  EXPECT_EQ(6, *--i);
  EXPECT_EQ(5, *--i);
  EXPECT_EQ(3, *--i);
  EXPECT_EQ(2, *--i);
  EXPECT_EQ(1, *--i);
  EXPECT_EQ(s.begin(), i);
}

TEST_F(correctness_test, iterator_decrement_2) {
  container s;
  mass_insert(s, {5, 2, 10, 9, 12, 7});

  container::iterator i = s.end();
  EXPECT_EQ(12, *--i);
  EXPECT_EQ(10, *--i);
  EXPECT_EQ(9, *--i);
  EXPECT_EQ(7, *--i);
  EXPECT_EQ(5, *--i);
  EXPECT_EQ(2, *--i);
  EXPECT_EQ(s.begin(), i);
}

TEST_F(correctness_test, iterator_postfix) {
  container c;
  mass_insert(c, {1, 2, 3});

  container::iterator i = c.begin();
  EXPECT_EQ(1, *i);
  container::iterator j = i++;
  EXPECT_EQ(2, *i);
  EXPECT_EQ(1, *j);
  j = i++;
  EXPECT_EQ(3, *i);
  EXPECT_EQ(2, *j);
  j = i++;
  EXPECT_EQ(c.end(), i);
  EXPECT_EQ(3, *j);
  j = i--;
  EXPECT_EQ(3, *i);
  EXPECT_EQ(c.end(), j);
}

TEST_F(correctness_test, const_iterator_postfix) {
  using container = set<const element>;

  container c;
  mass_insert(c, {1, 2, 3});

  container::const_iterator i = c.begin();
  EXPECT_EQ(1, *i);
  container::const_iterator j = i++;
  EXPECT_EQ(2, *i);
  EXPECT_EQ(1, *j);
  j = i++;
  EXPECT_EQ(3, *i);
  EXPECT_EQ(2, *j);
  j = i++;
  EXPECT_EQ(c.end(), i);
  EXPECT_EQ(3, *j);
  j = i--;
  EXPECT_EQ(3, *i);
  EXPECT_EQ(c.end(), j);
}

TEST_F(correctness_test, iterator_deref_1) {
  container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  container::iterator i = c.find(4);
  EXPECT_EQ(4, *i);
  magic(*i);
  expect_eq(c, {1, 2, 3, 4, 5, 6});

  container::const_iterator j = c.find(3);
  EXPECT_EQ(3, *j);
  magic(*j);
  expect_eq(c, {1, 2, 3, 4, 5, 6});
}

TEST_F(correctness_test, iterator_deref_1c) {
  container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  const container::iterator i = c.find(4);
  EXPECT_EQ(4, *i);
  magic(*i);
  expect_eq(c, {1, 2, 3, 4, 5, 6});

  const container::const_iterator j = c.find(3);
  EXPECT_EQ(3, *j);
  magic(*j);
  expect_eq(c, {1, 2, 3, 4, 5, 6});
}

TEST_F(correctness_test, iterator_deref_2) {
  container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  container::iterator i = c.find(4);
  EXPECT_EQ(4, *i);
  magic(*i.operator->());
  expect_eq(c, {1, 2, 3, 4, 5, 6});

  container::const_iterator j = c.find(3);
  EXPECT_EQ(3, *j);
  magic(*j.operator->());
  expect_eq(c, {1, 2, 3, 4, 5, 6});
}

TEST_F(correctness_test, iterator_deref_2c) {
  container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  const container::iterator i = c.find(4);
  EXPECT_EQ(4, *i);
  magic(*i.operator->());
  expect_eq(c, {1, 2, 3, 4, 5, 6});

  const container::const_iterator j = c.find(3);
  EXPECT_EQ(3, *j);
  magic(*j.operator->());
  expect_eq(c, {1, 2, 3, 4, 5, 6});
}

TEST_F(correctness_test, iterator_default_ctor) {
  container::iterator i;
  container::const_iterator j;
  container s;
  mass_insert(s, {4, 1, 8, 6, 3, 2, 6});

  i = s.begin();
  j = s.begin();
  EXPECT_EQ(1, *i);
  EXPECT_EQ(1, *j);
}

TEST_F(correctness_test, iterator_swap) {
  container c1;
  mass_insert(c1, {1, 2, 3});

  container c2;
  mass_insert(c2, {4, 5, 6});

  container::iterator i = c1.find(2);
  container::iterator j = c2.find(5);

  {
    using std::swap;
    swap(i, j);
  }

  c1.erase(j);
  c2.erase(i);
  expect_eq(c1, {1, 3});
  expect_eq(c2, {4, 6});
}

TEST_F(correctness_test, reverse_iterator) {
  container c;
  mass_insert(c, {3, 1, 2, 4});
  expect_eq(reverse_view(c), {4, 3, 2, 1});

  EXPECT_EQ(4, *c.rbegin());
  EXPECT_EQ(3, *std::next(c.rbegin()));
  EXPECT_EQ(1, *std::prev(c.rend()));
}

TEST_F(correctness_test, iterator_constness) {
  container c;
  mass_insert(c, {1, 2, 3});

  magic(*std::as_const(c).begin());
  magic(*std::prev(std::as_const(c).end()));
  expect_eq(c, {1, 2, 3});
}

TEST_F(correctness_test, reverse_iterator_constness) {
  container c;
  mass_insert(c, {1, 2, 3});

  magic(*std::as_const(c).rbegin());
  magic(*std::prev(std::as_const(c).rend()));
  expect_eq(c, {1, 2, 3});
}

TEST_F(correctness_test, iterator_value_type) {
  container c;
  mass_insert(c, {1, 2, 3});

  container::iterator::value_type e = *c.begin();
  e = 42;
  expect_eq(c, {1, 2, 3});
}

TEST_F(correctness_test, iterator_value_type_const) {
  container c;
  mass_insert(c, {1, 2, 3});

  container::const_iterator::value_type e = *std::as_const(c).begin();
  e = 42;
  expect_eq(c, {1, 2, 3});
}

TEST_F(correctness_test, clear) {
  container c;
  mass_insert(c, {1, 2, 3, 4, 5, 6});

  c.clear();
  expect_empty(c);

  mass_insert(c, {5, 6, 7, 8});
  expect_eq(c, {5, 6, 7, 8});
}

TEST_F(correctness_test, erase_begin) {
  container c;
  mass_insert(c, {1, 2, 3, 4});

  c.erase(c.begin());
  expect_eq(c, {2, 3, 4});
}

TEST_F(correctness_test, erase_middle) {
  container c;
  mass_insert(c, {1, 2, 3, 4});

  c.erase(std::next(c.begin(), 2));
  expect_eq(c, {1, 2, 4});
}

TEST_F(correctness_test, erase_close_to_end) {
  container c;
  mass_insert(c, {6, 1, 4, 3, 2, 5});

  c.erase(std::next(c.begin(), 4));
  expect_eq(c, {1, 2, 3, 4, 6});
}

TEST_F(correctness_test, erase_end) {
  container c;
  mass_insert(c, {1, 2, 3, 4});

  c.erase(std::prev(c.end()));
  expect_eq(c, {1, 2, 3});
}

TEST_F(correctness_test, erase_root) {
  container c;
  mass_insert(c, {5, 3, 8, 1, 2});

  c.erase(5);
  expect_eq(c, {1, 2, 3, 8});
}

TEST_F(correctness_test, erase_1) {
  container c;
  mass_insert(c, {5, 3, 8, 1, 2, 7, 9, 10, 11, 12});

  c.erase(8);
  expect_eq(c, {1, 2, 3, 5, 7, 9, 10, 11, 12});
}

TEST_F(correctness_test, erase_2) {
  container c;
  mass_insert(c, {5, 3, 17, 15, 20, 19, 18});

  c.erase(17);
  expect_eq(c, {3, 5, 15, 18, 19, 20});
}

TEST_F(correctness_test, erase_3) {
  container c;
  mass_insert(c, {10, 5, 15, 14, 13});

  c.erase(15);
  expect_eq(c, {5, 10, 13, 14});
}

TEST_F(correctness_test, erase_4) {
  container c;
  mass_insert(c, {10, 5, 15, 3, 4});

  c.erase(5);
  expect_eq(c, {3, 4, 10, 15});
}

TEST_F(correctness_test, erase_5) {
  container c;
  mass_insert(c, {5, 2, 10, 6, 14, 7, 8});

  c.erase(5);
  expect_eq(c, {2, 6, 7, 8, 10, 14});
}

TEST_F(correctness_test, erase_6) {
  container c;
  mass_insert(c, {7, 3, 2, 6, 10, 9});

  c.erase(3);
  c.erase(6);
  c.erase(7);
  c.erase(10);
  c.erase(2);
  c.erase(9);
  expect_empty(c);
}

TEST_F(correctness_test, erase_7) {
  container c;
  mass_insert(c, {5, 3, 8});

  c.erase(5);
  expect_eq(c, {3, 8});
  EXPECT_FALSE(c.empty());
}

TEST_F(correctness_test, erase_8) {
  container c;
  mass_insert(c, {5, 3});

  c.erase(5);
  expect_eq(c, {3});
  EXPECT_FALSE(c.empty());
}

TEST_F(correctness_test, erase_it_return_value_1) {
  container c;
  mass_insert(c, {5, 2, 1, 3, 4});

  container::iterator i = c.erase(c.find(3));
  EXPECT_EQ(4, *i);
  i = c.erase(i);
  EXPECT_EQ(5, *i);
}

TEST_F(correctness_test, erase_it_return_value_2) {
  container c;
  mass_insert(c, {1, 4, 3, 2, 5});

  container::iterator i = c.erase(c.find(3));
  EXPECT_EQ(4, *i);
  i = c.erase(i);
  EXPECT_EQ(5, *i);
}

TEST_F(correctness_test, erase_it_return_value_3) {
  container c;
  mass_insert(c, {7, 4, 10, 1, 8, 7, 12});

  container::iterator i = c.erase(c.find(7));
  EXPECT_EQ(8, *i);
  i = c.erase(i);
  EXPECT_EQ(10, *i);
}

TEST_F(correctness_test, erase_val_return_value_1) {
  container c;
  mass_insert(c, {7, 4, 10, 1, 8, 7, 12});

  size_t i = c.erase(7);
  EXPECT_EQ(1, i);
}

TEST_F(correctness_test, erase_val_return_value_2) {
  container c;
  mass_insert(c, {7, 4, 10, 1, 8, 7, 12});

  size_t i = c.erase(6);
  EXPECT_EQ(0, i);
}

TEST_F(correctness_test, erase_iterators) {
  container c;
  mass_insert(c, {8, 2, 6, 10, 3, 1, 9, 7});

  container::iterator i = c.find(8);
  container::iterator prev = std::prev(i);
  container::iterator next = std::next(i);

  c.erase(i);
  EXPECT_EQ(7, *prev);
  EXPECT_EQ(9, *next);
  EXPECT_EQ(next, std::next(prev));
  EXPECT_EQ(prev, std::prev(next));
}

TEST_F(correctness_test, find_in_empty) {
  container c;

  EXPECT_EQ(c.end(), c.find(0));
  EXPECT_EQ(c.end(), c.find(5));
  EXPECT_EQ(c.end(), c.find(42));
}

TEST_F(correctness_test, finds) {
  container c;
  mass_insert(c, {8, 3, 5, 4, 3, 1, 8, 8, 10, 9});

  EXPECT_EQ(c.end(), c.find(0));
  EXPECT_EQ(c.begin(), c.find(1));
  EXPECT_EQ(c.end(), c.find(2));
  EXPECT_EQ(std::next(c.begin(), 1), c.find(3));
  EXPECT_EQ(std::next(c.begin(), 2), c.find(4));
  EXPECT_EQ(std::next(c.begin(), 3), c.find(5));
  EXPECT_EQ(c.end(), c.find(6));
  EXPECT_EQ(c.end(), c.find(7));
  EXPECT_EQ(std::next(c.begin(), 4), c.find(8));
  EXPECT_EQ(std::next(c.begin(), 5), c.find(9));
  EXPECT_EQ(std::next(c.begin(), 6), c.find(10));
  EXPECT_EQ(c.end(), c.find(11));
}

TEST_F(correctness_test, lower_bound_empty) {
  container c;
  EXPECT_EQ(c.end(), c.lower_bound(5));
}

TEST_F(correctness_test, lower_bounds) {
  container c;
  mass_insert(c, {8, 3, 5, 4, 3, 1, 8, 8, 10, 9});

  EXPECT_EQ(c.begin(), c.lower_bound(0));
  EXPECT_EQ(c.begin(), c.lower_bound(1));
  EXPECT_EQ(std::next(c.begin()), c.lower_bound(2));
  EXPECT_EQ(std::next(c.begin()), c.lower_bound(3));
  EXPECT_EQ(std::next(c.begin(), 2), c.lower_bound(4));
  EXPECT_EQ(std::next(c.begin(), 3), c.lower_bound(5));
  EXPECT_EQ(std::next(c.begin(), 4), c.lower_bound(6));
  EXPECT_EQ(std::next(c.begin(), 4), c.lower_bound(7));
  EXPECT_EQ(std::next(c.begin(), 4), c.lower_bound(8));
  EXPECT_EQ(std::next(c.begin(), 5), c.lower_bound(9));
  EXPECT_EQ(std::next(c.begin(), 6), c.lower_bound(10));
  EXPECT_EQ(std::next(c.begin(), 7), c.lower_bound(11));
}

TEST_F(correctness_test, upper_bound_empty) {
  container c;
  EXPECT_EQ(c.end(), c.upper_bound(5));
}

TEST_F(correctness_test, upper_bounds) {
  container c;
  mass_insert(c, {8, 3, 5, 4, 3, 1, 8, 8, 10, 9});

  EXPECT_EQ(c.begin(), c.upper_bound(0));
  EXPECT_EQ(std::next(c.begin()), c.upper_bound(1));
  EXPECT_EQ(std::next(c.begin()), c.upper_bound(2));
  EXPECT_EQ(std::next(c.begin(), 2), c.upper_bound(3));
  EXPECT_EQ(std::next(c.begin(), 3), c.upper_bound(4));
  EXPECT_EQ(std::next(c.begin(), 4), c.upper_bound(5));
  EXPECT_EQ(std::next(c.begin(), 4), c.upper_bound(6));
  EXPECT_EQ(std::next(c.begin(), 4), c.upper_bound(7));
  EXPECT_EQ(std::next(c.begin(), 5), c.upper_bound(8));
  EXPECT_EQ(std::next(c.begin(), 6), c.upper_bound(9));
  EXPECT_EQ(std::next(c.begin(), 7), c.upper_bound(10));
  EXPECT_EQ(std::next(c.begin(), 7), c.upper_bound(11));
}

TEST_F(exception_safety_test, non_throwing_default_ctor) {
  faulty_run([] {
    try {
      container c;
    } catch (...) {
      fault_injection_disable dg;
      ADD_FAILURE() << "default constructor should not throw";
      throw;
    }
  });
}

TEST_F(exception_safety_test, copy_ctor) {
  faulty_run([] {
    container c;
    mass_insert(c, {3, 2, 4, 1});

    container c2 = c;
    expect_eq(c, {1, 2, 3, 4});
  });
}

TEST_F(exception_safety_test, non_throwing_clear) {
  faulty_run([] {
    container c;
    mass_insert(c, {3, 2, 4, 1});
    try {
      c.clear();
    } catch (...) {
      fault_injection_disable dg;
      ADD_FAILURE() << "clear() should not throw";
      throw;
    }
  });
}

TEST_F(exception_safety_test, copy_assignment) {
  faulty_run([] {
    container c;
    mass_insert(c, {3, 2, 4, 1});

    container c2;
    mass_insert(c2, {8, 7, 2, 14});

    strong_exception_safety_guard sg(c);
    c = c2;
    expect_eq(c, {2, 7, 8, 14});
  });
}

TEST_F(exception_safety_test, insert) {
  faulty_run([] {
    container c;
    mass_insert(c, {3, 2, 4, 1});

    strong_exception_safety_guard sg(c);
    c.insert(5);
    expect_eq(c, {1, 2, 3, 4, 5});
  });
}

TEST_F(exception_safety_test, erase_1) {
  faulty_run([] {
    container c;
    mass_insert(c, {6, 3, 8, 2, 5, 7, 10});

    strong_exception_safety_guard sg(c);
    element val = 6;
    c.erase(c.find(val));
    expect_eq(c, {2, 3, 5, 7, 8, 10});
  });
}

TEST_F(exception_safety_test, erase_2) {
  faulty_run([] {
    container c;
    mass_insert(c, {6, 3, 8, 2, 5, 7, 10});

    strong_exception_safety_guard sg(c);
    element val = 5;
    c.erase(c.find(val));
    expect_eq(c, {2, 3, 6, 7, 8, 10});
  });
}

TEST_F(performance_test, size) {
  constexpr size_t N = 100'000;
  constexpr size_t K = 1'000'000;

  container c;
  mass_insert_balanced(c, N);

  for (size_t i = 0; i < K; ++i) {
    EXPECT_EQ(N, c.size());
  }
}

TEST_F(performance_test, iteration) {
  constexpr size_t N = 100'000;
  constexpr size_t K = 5;

  container c;
  mass_insert_balanced(c, N);

  for (size_t i = 0; i < K; ++i) {
    for (container::iterator j = c.begin(); j != c.end(); ++j) {
      [[maybe_unused]] volatile int escape = *j;
    }
  }
}

TEST_F(performance_test, lower_bound) {
  constexpr size_t N = 100'000;
  constexpr size_t K = 200'000;

  container c;
  mass_insert_balanced(c, N);

  for (size_t i = 0; i < K; ++i) {
    constexpr int n = N;
    EXPECT_EQ(c.begin(), c.lower_bound(1));
    EXPECT_EQ(std::prev(c.end()), c.lower_bound(n));
  }
}

TEST_F(performance_test, swap) {
  constexpr size_t N = 100'000;
  constexpr size_t K = 1'000'000;

  container c1;
  container c2;

  mass_insert_balanced(c1, N);
  mass_insert_balanced(c2, N, -1);

  for (size_t i = 0; i < K; ++i) {
    swap(c1, c2);
  }
}

namespace {

struct random_test_config {
  std::mt19937::result_type seed = std::mt19937::default_seed;
  std::uniform_int_distribution<int> value_dist;
  size_t iterations;
  double p_insert;
  double p_erase;
  double p_compare = .1;
};

void run_random_test(random_test_config cfg) {
  std::mt19937 rng(cfg.seed);

  std::uniform_real_distribution real_dist;

  std::set<int> std_set;
  container my_set;

  for (size_t i = 0; i < cfg.iterations; ++i) {
    double op = real_dist(rng);
    int e = cfg.value_dist(rng);

    if (op < cfg.p_insert) {
      auto [std_it, std_ins] = std_set.insert(e);
      auto [my_it, my_ins] = my_set.insert(e);
      ASSERT_EQ(std_ins, my_ins);
      ASSERT_EQ(*std_it, *my_it);
    } else if (op < cfg.p_insert + cfg.p_erase) {
      auto std_erase_result = std_set.erase(e);
      auto my_erase_result = my_set.erase(e);
      ASSERT_EQ(std_erase_result, my_erase_result);
    } else {
      auto std_it = std_set.find(e);
      auto my_it = my_set.find(e);
      ASSERT_EQ(std_it == std_set.end(), my_it == my_set.end());
    }

    ASSERT_EQ(std_set.empty(), my_set.empty());
    ASSERT_EQ(std_set.size(), my_set.size());

    if (real_dist(rng) < cfg.p_compare) {
      ASSERT_TRUE(std::equal(std_set.begin(), std_set.end(), my_set.begin()));
    }
  }
}

} // namespace

TEST_F(random_test, insert_find_scattered) {
  random_test_config cfg;
  cfg.seed = 1337;
  cfg.value_dist = std::uniform_int_distribution(1, 10'000);
  cfg.iterations = 10'000;
  cfg.p_insert = .5;
  cfg.p_erase = 0;

  run_random_test(cfg);
}

TEST_F(random_test, insert_find_dense) {
  random_test_config cfg;
  cfg.seed = 1338;
  cfg.value_dist = std::uniform_int_distribution(1, 500);
  cfg.iterations = 100'000;
  cfg.p_insert = .5;
  cfg.p_erase = 0;

  run_random_test(cfg);
}

TEST_F(random_test, insert_erase_find_scattered_1) {
  random_test_config cfg;
  cfg.seed = 1339;
  cfg.value_dist = std::uniform_int_distribution(1, 10'000);
  cfg.iterations = 10'000;
  cfg.p_insert = .4;
  cfg.p_erase = .2;

  run_random_test(cfg);
}

TEST_F(random_test, insert_erase_find_dense_1) {
  random_test_config cfg;
  cfg.seed = 1340;
  cfg.value_dist = std::uniform_int_distribution(1, 500);
  cfg.iterations = 100'000;
  cfg.p_insert = .4;
  cfg.p_erase = .2;

  run_random_test(cfg);
}

TEST_F(random_test, insert_erase_find_scattered_2) {
  random_test_config cfg;
  cfg.seed = 1341;
  cfg.value_dist = std::uniform_int_distribution(1, 10'000);
  cfg.iterations = 10'000;
  cfg.p_insert = .01;
  cfg.p_erase = .7;

  run_random_test(cfg);
}

TEST_F(random_test, insert_erase_find_dense_2) {
  random_test_config cfg;
  cfg.seed = 1342;
  cfg.value_dist = std::uniform_int_distribution(1, 500);
  cfg.iterations = 100'000;
  cfg.p_insert = .01;
  cfg.p_erase = .7;

  run_random_test(cfg);
}
