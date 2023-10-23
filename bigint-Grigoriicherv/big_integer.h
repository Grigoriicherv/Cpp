#pragma once

#include <functional>
#include <iosfwd>
#include <string>

struct big_integer {
  big_integer();
  big_integer(const big_integer& other);
  big_integer(int a);
  big_integer(long a);
  big_integer(long long a);
  big_integer(unsigned int val);
  big_integer(unsigned long val);
  big_integer(unsigned long long val);
  big_integer(std::vector<uint32_t> vec, bool sig);
  big_integer(const std::string& str);
  ~big_integer();

  big_integer& operator=(const big_integer& other);

  big_integer& operator+=(const big_integer& rhs);
  big_integer& operator-=(const big_integer& rhs);
  big_integer& operator*=(const big_integer& rhs);
  big_integer& operator/=(const big_integer& rhs);
  big_integer& operator%=(const big_integer& rhs);

  big_integer& operator&=(const big_integer& rhs);
  big_integer& operator|=(const big_integer& rhs);
  big_integer& operator^=(const big_integer& rhs);

  big_integer& operator<<=(int rhs);
  big_integer& operator>>=(int rhs);

  big_integer operator+() const;
  big_integer operator-() const;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);

  friend bool operator==(const big_integer& a, const big_integer& b);
  friend bool operator!=(const big_integer& a, const big_integer& b);
  friend bool operator<(const big_integer& a, const big_integer& b);
  friend bool operator>(const big_integer& a, const big_integer& b);
  friend bool operator<=(const big_integer& a, const big_integer& b);
  friend bool operator>=(const big_integer& a, const big_integer& b);
  friend std::string to_string(const big_integer& a);

private:
  big_integer abs() const;
  uint32_t get_if_exist(size_t index, bool for_bitwise) const;
  void shrink();
  void shrink_for_sign_and_not();
  template <class Func>
  big_integer& bitwise_operation_assign(const big_integer& rhs, Func operation);
  big_integer to_bit_op(const big_integer& b);
  uint32_t get_neg(size_t index) const;
  void adding(const big_integer& rhs);
  void subtracting(const big_integer& rhs, bool rhs_bigger);
  bool cmp_abs(const big_integer& b, bool signing) const;
  big_integer& div(const big_integer& rhs, bool mod);
  uint32_t div_short(uint32_t right);
  void sub_short(uint32_t right);
  void mul_short(uint32_t right);
  void add_short(uint32_t right);

  std::vector<uint32_t> data;
  bool sign{};
};

big_integer operator+(const big_integer& a, const big_integer& b);
big_integer operator-(const big_integer& a, const big_integer& b);
big_integer operator*(const big_integer& a, const big_integer& b);
big_integer operator/(const big_integer& a, const big_integer& b);
big_integer operator%(const big_integer& a, const big_integer& b);

big_integer operator&(const big_integer& a, const big_integer& b);
big_integer operator|(const big_integer& a, const big_integer& b);
big_integer operator^(const big_integer& a, const big_integer& b);

big_integer operator<<(const big_integer& a, int b);
big_integer operator>>(const big_integer& a, int b);

bool operator==(const big_integer& a, const big_integer& b);
bool operator!=(const big_integer& a, const big_integer& b);
bool operator<(const big_integer& a, const big_integer& b);
bool operator>(const big_integer& a, const big_integer& b);
bool operator<=(const big_integer& a, const big_integer& b);
bool operator>=(const big_integer& a, const big_integer& b);

std::string to_string(const big_integer& a);
std::ostream& operator<<(std::ostream& out, const big_integer& a);
