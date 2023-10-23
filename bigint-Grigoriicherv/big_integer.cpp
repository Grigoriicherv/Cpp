#include "big_integer.h"

#include <charconv>
#include <complex>
#include <cstddef>
#include <cstring>
#include <limits>
#include <ostream>
#include <stdexcept>

// Constructors

big_integer::big_integer() = default;

big_integer::big_integer(const big_integer& other) = default;

big_integer::big_integer(std::vector<uint32_t> vec, bool sig) : data(vec), sign(sig) {}

big_integer::big_integer(int a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(long a) : big_integer(static_cast<long long>(a)) {}

big_integer::big_integer(long long a) {
  *this = (a == std::numeric_limits<long long>::min())
            ? static_cast<unsigned long long>(std::numeric_limits<long long>::max()) + 1
            : static_cast<unsigned long long>(std::abs(a));
  this->sign = a < 0;
}

big_integer::big_integer(unsigned int a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(unsigned long a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::big_integer(unsigned long long a) {
  if (a == 0) {
    data.push_back(0);
  }
  while (a > 0) {
    data.push_back(a % (1ull << 32));
    a >>= 32;
  }
}

big_integer::big_integer(const std::string& str) {
  if (str.size() == 0 || (str.size() == 1 && str[0] == '-')) {
    throw std::invalid_argument("Can't parse empty string or '-'");
  }
  uint32_t value = 0;
  for (int i = (str[0] == '-'); i < str.size(); i += 9) {
    const auto res = std::from_chars(str.data() + i, str.data() + 9 + i, value);
    uint32_t rw = res.ptr - (str.data() + i);
    if (res.ec == std::errc::invalid_argument) {
      throw std::invalid_argument(std::string(&"Expected digit at index "[i]) + ", found " + str[i]);
    }
    if (res.ec == std::errc() && rw < 9) {
      for (size_t j = i; j < i + 9; ++j) {
        if (j >= str.size()) {
          break;
        }
        if (!std::isdigit(str[j])) {
          throw std::invalid_argument(std::string(&"Expected digit at index "[j]) + ", found " + str[j]);
        }
      }
    }
    *this *= static_cast<uint32_t>(std::pow(10, rw));
    *this += value;
  }
  this->sign = (str[0] == '-');
}

big_integer::~big_integer() = default;

// Functions

uint32_t big_integer::div_short(uint32_t right) {
  uint32_t carry = 0;
  for (size_t i = data.size() - 1;; i--) {
    uint64_t tmp = (static_cast<uint64_t>(carry) << 32) + data[i];
    data[i] = tmp / right;
    carry = tmp % right;
    if (i == 0) {
      break;
    }
  }
  shrink();
  return carry;
}

void big_integer::mul_short(uint32_t right) {
  uint64_t carry = 0;
  for (uint32_t& cur : data) {
    uint64_t temp = static_cast<uint64_t>(cur) * static_cast<uint64_t>(right) + carry;
    cur = static_cast<uint32_t>(temp % (1ull << 32));
    carry = static_cast<uint32_t>(temp / (1ull << 32));
  }
  if (carry > 0) {
    data.push_back(carry);
  }
  shrink();
}

void big_integer::add_short(uint32_t right) {
  if (data.empty() || (data.size() == 1 && data[0] == 0)) {
    data.push_back(right);
    sign = false;
    shrink();
    return;
  }
  uint64_t carry = right;
  for (size_t i = 0; i < data.size(); ++i) {
    uint64_t sum = static_cast<uint64_t>(data[i]) + carry;
    data[i] = sum & UINT32_MAX;
    carry = sum >> 32;
  }
  while (carry) {
    data.push_back(carry & UINT32_MAX);
    carry >>= 32;
  }
  shrink();
}

void big_integer::sub_short(uint32_t right) {
  if (data.empty() || (data.size() == 1 && data[0] == 0)) {
    data.push_back(right);
    sign = true;
    shrink();
    return;
  }
  bool carry = false;
  size_t size = this->data.size() + 1;
  data.resize(size);
  size_t index = 0;
  uint64_t sub = static_cast<uint64_t>(right);
  carry = sub > get_if_exist(index, false);
  data[index] = static_cast<uint32_t>(~(sub + ~get_if_exist(index, false)));
  while (carry) {
    sub = carry;
    carry = sub > get_if_exist(index, false);
    data[index] = static_cast<uint32_t>(~(sub + ~get_if_exist(index, false)));
  }
  shrink();
}

uint32_t big_integer::get_neg(size_t index) const {
  return sign ? ~get_if_exist(index, false) : get_if_exist(index, false);
}

uint32_t big_integer::get_if_exist(size_t index, bool for_bitwise) const {
  if (index >= data.size()) {
    if (for_bitwise) {
      if (sign) {
        return UINT32_MAX;
      }
    }
    return 0;
  }
  return data[index];
}

void big_integer::shrink_for_sign_and_not() {
  shrink();
  if (sign) {
    for (size_t i = 0; i < data.size(); i++) {
      data[i] = ~data[i];
    }
    *this -= 1;
  }
}

void big_integer::shrink() {
  for (size_t i = data.size() - 1; !data.empty() && (data[i] == 0 && i > 0); i--) {
    data.pop_back();
  }
}

big_integer big_integer::to_bit_op(const big_integer& b) {
  big_integer new_tmp(b);
  if (b.sign) {
    for (size_t i = 0; i < b.data.size(); i++) {
      new_tmp.data[i] = ~b.data[i];
    }
    new_tmp -= 1;
  }
  shrink();
  return new_tmp;
}

big_integer big_integer::abs() const {
  return sign ? -(*this) : *this;
}

bool big_integer::cmp_abs(const big_integer& b, bool signing) const {
  if (data.size() != b.data.size()) {
    return (signing ? sign ^ (data.size() < b.data.size()) : (data.size() < b.data.size()));
  }
  for (size_t i = data.size() - 1;; i--) {
    if (data[i] != b.data[i]) {
      return (signing ? (data[i] < b.data[i]) ^ sign : (data[i] < b.data[i]));
    }
    if (i == 0) {
      break;
    }
  }
  return false;
}

// Methods

big_integer& big_integer::operator=(const big_integer& other) = default;

void big_integer::adding(const big_integer& rhs) {
  bool carry = false;
  size_t size = std::max(this->data.size() + 1, rhs.data.size() + 1);
  data.resize(size);
  for (size_t index = 0; index < size || carry; index++) {
    uint64_t sum = static_cast<uint64_t>(get_if_exist(index, false)) + rhs.get_if_exist(index, false) + carry;
    data[index] = sum & UINT32_MAX;
    carry = sum >> 32;
  }
  shrink();
}

void big_integer::subtracting(const big_integer& rhs, bool rhs_bigger) {
  bool carry = false;
  std::vector<uint32_t> new_data(std::max(data.size(), rhs.data.size()), 0);
  for (size_t index = 0; index < (rhs_bigger ? rhs.data.size() : data.size()); index++) {
    uint64_t sub =
        static_cast<uint64_t>(rhs_bigger ? get_if_exist(index, false) : rhs.get_if_exist(index, false)) + carry;
    carry = sub > (rhs_bigger ? rhs.get_if_exist(index, false) : get_if_exist(index, false));
    new_data[index] =
        static_cast<uint32_t>(~(sub + (rhs_bigger ? ~rhs.get_if_exist(index, false) : ~get_if_exist(index, false))));
  }
  data = new_data;
  shrink();
}

big_integer& big_integer::operator+=(const big_integer& rhs) {
  if (sign == rhs.sign) {
    adding(rhs);
    return *this;
  } else {
    if (cmp_abs(rhs, false)) {
      subtracting(rhs, true);
      sign = !sign;
    } else {
      subtracting(rhs, false);
    }
    return *this;
  }
}

big_integer& big_integer::operator-=(const big_integer& rhs) {
  if (sign == rhs.sign) {
    if (cmp_abs(rhs, false)) {
      subtracting(rhs, true);
      sign = !rhs.sign;
      return *this;
    }
    subtracting(rhs, false);
    return *this;
  } else {
    adding(rhs);
    return *this;
  }
}

big_integer& big_integer::operator*=(const big_integer& rhs) {
  std::vector<uint32_t> new_data(data.size() + rhs.data.size() + 1);
  for (size_t i = 0; i < data.size(); i++) {
    if (data[i] == 0) {
      continue;
    }
    uint32_t carry = 0;
    for (size_t j = 0; j < rhs.data.size() || carry > 0; j++) {
      uint64_t current = static_cast<uint64_t>(get_if_exist(i, false)) * rhs.get_if_exist(j, false) +
                         static_cast<uint64_t>(new_data[i + j]) + carry;
      uint32_t digit = static_cast<uint32_t>(current & UINT32_MAX);
      new_data[i + j] = digit;
      carry = static_cast<uint32_t>(current >> 32);
    }
  }
  data = new_data;
  sign = sign ^ rhs.sign;
  shrink();
  return *this;
}

big_integer& big_integer::div(const big_integer& rhs, bool mod) {
  if (rhs == 0) {
    throw std::runtime_error("Division by zero");
  }
  big_integer left = abs(), right = rhs.abs();
  if (left < right) {
    left.sign = sign;
    return mod ? *this = left : *this = 0;
  } else if (left == right) {
    return mod ? *this = 0 : *this = 1;
  } else if (right.data.size() == 1) {
    if (mod) {
      this->data.resize(1);
      this->data[0] = div_short(right.data[0]);
    } else {
      div_short(right.data[0]);
      sign = (sign ^ rhs.sign);
    }
    return *this;
  } else {
    auto factor = static_cast<uint32_t>((1LL << 32) / static_cast<uint64_t>(right.data[right.data.size() - 1] + 1));
    left *= factor;
    right *= factor;
    right.shrink();
    size_t n = left.data.size() - right.data.size();
    data.resize(n + 1);
    for (size_t i = n;; i--) {
      uint32_t q_new = ((static_cast<uint64_t>(left.get_if_exist(i + right.data.size(), false)) << 32) +
                        static_cast<uint64_t>(left.get_if_exist(i - 1 + right.data.size(), false))) /
                       static_cast<uint64_t>(right.data.back());
      data[i] = std::min(q_new, UINT32_MAX);
      left -= right * data[i] << ((i)*32);
      while (left < 0) {
        data[i]--;
        left += right << ((i)*32);
      }
      if (i == 0) {
        break;
      }
    }
    if (mod) {
      left.div_short(factor);
      this->data = left.data;
    } else {
      sign ^= rhs.sign;
    }
    shrink();
    return *this;
  }
}

big_integer& big_integer::operator/=(const big_integer& rhs) {
  return div(rhs, false);
}

big_integer& big_integer::operator%=(const big_integer& rhs) {
  return div(rhs, true);
}

template <class Func>
big_integer& big_integer::bitwise_operation_assign(const big_integer& rhs, Func operation) {
  big_integer left = to_bit_op(*this);
  big_integer right = to_bit_op(rhs);
  data.resize(std::max(data.size(), rhs.data.size()));
  for (size_t i = 0; i < data.size(); i++) {
    data[i] = operation(left.get_if_exist(i, true), right.get_if_exist(i, true));
  }
  sign = operation(sign, rhs.sign);
  shrink_for_sign_and_not();
  return *this;
}

big_integer& big_integer::operator&=(const big_integer& rhs) {
  return bitwise_operation_assign(rhs, std::bit_and<uint32_t>{});
}

big_integer& big_integer::operator|=(const big_integer& rhs) {
  return bitwise_operation_assign(rhs, std::bit_or<uint32_t>{});
}

big_integer& big_integer::operator^=(const big_integer& rhs) {
  return bitwise_operation_assign(rhs, std::bit_xor<uint32_t>{});
}

big_integer& big_integer::operator<<=(int rhs) {
  size_t offset = rhs / 32;
  size_t mod = rhs % 32;
  if (sign) {
    sub_short(1);
  }
  std::vector<uint32_t> new_data(data.size() + offset + 1, 0);
  if (sign) {
    new_data.back() ^= (UINT32_MAX << mod);
  }
  if (mod == 0) {
    for (size_t index = data.size();; index--) {
      new_data[index + offset] = get_neg(index);
      if (index == 0) {
        break;
      }
    }
  } else {
    for (size_t index = 0; index < data.size(); index++) {
      uint32_t value = get_neg(index);
      new_data[index + offset] += value << mod;
      new_data[index + offset + 1] += value >> (32 - mod);
    }
  }
  this->data = new_data;
  shrink_for_sign_and_not();
  return *this;
}

big_integer& big_integer::operator>>=(int rhs) {
  size_t offset = rhs / 32;

  std::vector<uint32_t> new_data;
  new_data.reserve(data.size() - offset);
  rhs %= 32;
  for (size_t index = offset; index < data.size(); index++) {
    uint32_t value = get_neg(index);
    if (rhs != 0) {
      value = (value >> rhs) | (get_neg(index + 1) << (32 - rhs));
    }
    new_data.push_back(value);
  }
  this->data = new_data;
  shrink_for_sign_and_not();
  return *this;
}

big_integer big_integer::operator+() const {
  return *this;
}

big_integer big_integer::operator-() const {
  big_integer tmp(*this);
  tmp.sign = !tmp.sign;
  return tmp;
}

big_integer big_integer::operator~() const {
  return -*this -= 1;
}

big_integer& big_integer::operator++() {
  sign ? sub_short(1) : add_short(1);
  return *this;
}

big_integer big_integer::operator++(int) {
  big_integer res(*this);
  sign ? sub_short(1) : add_short(1);
  return res;
}

big_integer& big_integer::operator--() {
  sign ? add_short(1) : sub_short(1);
  return *this;
}

big_integer big_integer::operator--(int) {
  big_integer res(*this);
  sign ? add_short(1) : sub_short(1);
  return res;
}

big_integer operator+(const big_integer& a, const big_integer& b) {
  big_integer tmp(a);
  return tmp += b;
}

big_integer operator-(const big_integer& a, const big_integer& b) {
  big_integer tmp(a);
  return tmp -= b;
}

big_integer operator*(const big_integer& a, const big_integer& b) {
  big_integer tmp(a);
  return tmp *= b;
}

big_integer operator/(const big_integer& a, const big_integer& b) {
  big_integer tmp(a);
  return tmp /= b;
}

big_integer operator%(const big_integer& a, const big_integer& b) {
  big_integer tmp(a);
  return tmp %= b;
}

big_integer operator&(const big_integer& a, const big_integer& b) {
  return big_integer(a) &= b;
}

big_integer operator|(const big_integer& a, const big_integer& b) {
  return big_integer(a) |= b;
}

big_integer operator^(const big_integer& a, const big_integer& b) {
  return big_integer(a) ^= b;
}

big_integer operator<<(const big_integer& a, int b) {
  return big_integer(a) <<= b;
}

big_integer operator>>(const big_integer& a, int b) {
  return big_integer(a) >>= b;
}

bool operator==(const big_integer& a, const big_integer& b) {
  if (a.data.empty() || b.data.empty()) {
    return true;
  }
  if (a.data.size() != b.data.size()) {
    if (a.data.empty() && (b.data.size() == 1 && b.data[0] == 0)) {
      return true;
    }
    if (b.data.empty() && (a.data.size() == 1 && a.data[0] == 0)) {
      return true;
    }
    return false;
  }
  if (a.sign != b.sign && (a.data[0] != 0 && b.data[0] != 0)) {
    return false;
  }
  for (int i = 0; i < a.data.size(); i++) {
    if (a.data[i] != b.data[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const big_integer& a, const big_integer& b) {
  return !(a == b);
}

bool operator<(const big_integer& a, const big_integer& b) {
  if (a.sign != b.sign) {
    return a.sign;
  }
  return a.cmp_abs(b, true);
}

bool operator>(const big_integer& a, const big_integer& b) {
  return b < a;
}

bool operator<=(const big_integer& a, const big_integer& b) {
  return !(a > b);
}

bool operator>=(const big_integer& a, const big_integer& b) {
  return !(a < b);
}

std::string to_string(const big_integer& a) {
  if (a == 0) {
    return "0";
  }
  std::string str;
  big_integer copy = a.abs();
  while (copy != 0) {
    uint32_t digit = copy.div_short(1000000000);
    std::string tmp = std::to_string(digit);
    std::reverse(tmp.begin(), tmp.end());
    str += tmp;
    if (copy != 0) {
      for (size_t i = tmp.size(); i < 9; i++) {
        str += '0';
      }
    }
  }
  if (a.sign) {
    str.push_back('-');
  }
  std::reverse(str.begin(), str.end());
  if (str.empty()) {
    str = "0";
  }
  return str;
}

std::ostream& operator<<(std::ostream& out, const big_integer& a) {
  return out << to_string(a);
}
