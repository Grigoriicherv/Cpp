#pragma once

#include <cassert>
#include <memory>

template <typename It, typename = void>
class counting_iterator_base {};

template <typename It>
class counting_iterator_base<It, std::void_t<typename std::iterator_traits<It>::iterator_concept>> {
public:
  using iterator_concept = std::iterator_traits<It>::iterator_concept;
};

template <typename It>
class counting_iterator : public counting_iterator_base<It> {
public:
  using value_type = std::iterator_traits<It>::value_type;
  using reference = std::iterator_traits<It>::reference;
  using pointer = std::iterator_traits<It>::pointer;
  using difference_type = std::iterator_traits<It>::difference_type;
  using iterator_category = std::iterator_traits<It>::iterator_category;

  counting_iterator() = default;

  explicit counting_iterator(It it, size_t n) : _base(it), _count(n) {}

  decltype(auto) operator*() const {
    assert(_count > 0);
    return *_base;
  }

  decltype(auto) operator->() const {
    return std::to_address(_base);
  }

  decltype(auto) operator[](std::ptrdiff_t n) const {
    assert(n >= 0 && n < _count);
    return _base[n];
  }

  counting_iterator& operator++() {
    assert(_count > 0);
    return ++_base;
  }

  counting_iterator operator++(int) {
    assert(_count > 0);
    return _base++;
  }

  counting_iterator& operator--() {
    return --_base;
  }

  counting_iterator operator--(int) {
    _base--;
    return *this;
  }

  counting_iterator operator+(std::ptrdiff_t n) const {
    assert(_count >= n);
    return counting_iterator(_base + n);
  }

  friend counting_iterator operator+(std::ptrdiff_t n, const counting_iterator& rhs) {
    assert(rhs._count >= n);
    return counting_iterator(n + rhs._base);
  }

  counting_iterator operator-(std::ptrdiff_t n) const {
    return counting_iterator(_base - n);
  }

  friend decltype(auto) operator-(const counting_iterator & lhs, const counting_iterator & rhs) {
    return lhs._base - rhs._base;
  }

  counting_iterator& operator+=(std::ptrdiff_t n) {
    assert(_count >= n);
    _base += n;
    return *this;
  }

  counting_iterator& operator-=(std::ptrdiff_t n) {
    _base -= n;
    return *this;
  }

  friend decltype(auto) operator==(const counting_iterator & lhs, const counting_iterator & rhs) {
    return lhs._base == rhs._base;
  }

  friend decltype(auto) operator!=(const counting_iterator & lhs, const counting_iterator & rhs) {
    return lhs._base != rhs._base;
  }

  friend decltype(auto) operator<(const counting_iterator & lhs, const counting_iterator & rhs) {
    return lhs._base < rhs._base;
  }

  friend decltype(auto) operator<=(const counting_iterator & lhs, const counting_iterator & rhs) {
    return lhs._base <= rhs._base;
  }

  friend decltype(auto) operator>(const counting_iterator & lhs, const counting_iterator & rhs) {
    return lhs._base > rhs._base;
  }

  friend decltype(auto) operator>=(const counting_iterator & lhs, const counting_iterator & rhs) {
    return lhs._base >= rhs._base;
  }

private:
  It _base;
  size_t _count;
};
