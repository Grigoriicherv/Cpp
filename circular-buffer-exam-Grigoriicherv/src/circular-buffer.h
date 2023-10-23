#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <utility>

template <typename T>
class circular_buffer {
private:
  template <typename Tp>
  struct template_iterator {

    friend circular_buffer;

    using value_type = T;
    using reference = Tp&;
    using pointer = Tp*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

  private:
    pointer data_;
    std::size_t pos_;
    std::size_t size_;

  private:
    template_iterator(pointer data, std::size_t pos, std::size_t size) : data_(data), pos_(pos), size_(size) {}



  public:
    template_iterator() = default;


    template <typename Q>
    requires std::is_same_v<const Q, Tp>
    template_iterator(template_iterator<Q> const& other)
        : data_(other.data_), pos_(other.pos_), size_(other.size_) {}

    reference operator*() const {
      return data_[pos_ % size_];
    }

    pointer operator->() const {
      return data_ + pos_ % size_;
    }

    template_iterator& operator++() {
      pos_++;
      return *this;
    }

    template_iterator operator++(int) {
      template_iterator temp = *this;
      ++*this;
      return temp;
    }

    template_iterator& operator--() {
      pos_--;
      return *this;
    }

    template_iterator operator--(int) {
      template_iterator temp = *this;
      --*this;
      return temp;
    }

    friend bool operator==(const template_iterator& left, const template_iterator& right) {
      return left.pos_ == right.pos_;
    }

    friend bool operator!=(const template_iterator& left, const template_iterator& right) {
      return !(left == right);
    }

    template_iterator& operator+=(difference_type n) {
      pos_ = static_cast<std::size_t>(static_cast<difference_type>(pos_) + n);
      return *this;
    }

    friend template_iterator operator+(const template_iterator& a, difference_type n) {
      return template_iterator(a.data_, static_cast<std::size_t>(static_cast<difference_type>(a.pos_) + n), a.size_);
    }

    friend template_iterator operator+(difference_type n, const template_iterator& a) {
      return a + n;
    }

    template_iterator& operator-=(difference_type n) {
      pos_ = static_cast<std::size_t>(static_cast<difference_type>(pos_) - n);
      return *this;
    }

    friend template_iterator operator-(const template_iterator& a, difference_type n) {
      return template_iterator(a.data_, static_cast<std::size_t>(static_cast<difference_type>(a.pos_) - n), a.size_);
    }

    friend difference_type operator-(const template_iterator& b, const template_iterator& a) {
      return b.pos_ - a.pos_;
    }

    reference operator[](difference_type n) const {
      return *(*this + n);
    }

    friend bool operator<(const template_iterator& a, const template_iterator& b) {
      return a.data_ == b.data_ && (b - a) > 0;
    }

    friend bool operator>(const template_iterator& a, const template_iterator& b) {
      return b < a;
    }

    friend bool operator>=(const template_iterator& a, const template_iterator& b) {
      return !(a < b);
    }

    friend bool operator<=(const template_iterator& a, const template_iterator& b) {
      return !(a > b);
    }
  };

public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = template_iterator<T>;
  using const_iterator = template_iterator<const T>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  iterator insert_with_copy(const_iterator pos, const T& val) {
    std::size_t idx = pos - begin();
    circular_buffer tmp(capacity() == 0 ? 1 : capacity() * 2);
    std::uninitialized_copy(begin(), begin() + idx, tmp.begin());
    tmp.size_ += idx;
    new (tmp.data_ + ((tmp.begin_ + tmp.size()) % tmp.capacity())) T(val);
    tmp.size_++;
    std::uninitialized_copy(begin() + idx, end(), tmp.end());
    tmp.size_ = size() + 1;
    swap(*this, tmp);
    return begin() + idx;
  }

public:
  // O(1), nothrow
  circular_buffer() noexcept = default;

  circular_buffer(std::size_t capacity)
      : data_(capacity == 0 ? nullptr : static_cast<T*>(operator new(sizeof(T) * capacity))),
        capacity_(capacity) {}


  // O(n), strong
  circular_buffer(const circular_buffer& other) : circular_buffer(other.capacity()) {
    begin_ = other.begin_;
    std::uninitialized_copy(other.begin(), other.end(), begin());
    size_ = other.size();
  }

  // O(n), strong
  circular_buffer& operator=(const circular_buffer& other) {
    if (this != &other) {
      circular_buffer tmp(other);
      swap(tmp, *this);
    }
    return *this;
  }

  // O(n), nothrow
  ~circular_buffer() {
    clear();
    operator delete(data_);
  }

  // O(1), nothrow
  size_t size() const noexcept {
    return size_;
  }

  // O(1), nothrow
  bool empty() const noexcept {
    return size() == 0;
  }

  // O(1), nothrow
  size_t capacity() const noexcept {
    return capacity_;
  }

  // O(1), nothrow
  iterator begin() noexcept {
    return {data_, begin_, capacity()};
  }

  // O(1), nothrow
  const_iterator begin() const noexcept {
    return {data_, begin_, capacity()};
  }

  // O(1), nothrow
  iterator end() noexcept {
    return begin() + size();
  }

  // O(1), nothrow
  const_iterator end() const noexcept {
    return begin() + size();
  }

  // O(1), nothrow
  reverse_iterator rbegin() noexcept {
    return std::reverse_iterator<iterator>(end());
  }

  // O(1), nothrow
  const_reverse_iterator rbegin() const noexcept {
    return std::reverse_iterator<const_iterator>(end());
  }

  // O(1), nothrow
  reverse_iterator rend() noexcept {
    return std::reverse_iterator<iterator>(begin());
  }

  // O(1), nothrow
  const_reverse_iterator rend() const noexcept {
    return std::reverse_iterator<const_iterator>(begin());
  }

  // O(1), nothrow
  T& operator[](size_t index) {
    return data_[(begin_ + index) % capacity()];
  }

  // O(1), nothrow
  const T& operator[](size_t index) const {
    return data_[(begin_ + index) % capacity()];
  }

  // O(1), nothrow
  T& back() {
    return data_[(begin_ + size() - 1) % capacity()];
  }

  // O(1), nothrow
  const T& back() const {
    return data_[(begin_ + size() - 1) % capacity()];
  }

  // O(1), nothrow
  T& front() {
    return data_[begin_];
  }

  // O(1), nothrow
  const T& front() const {
    return data_[begin_];
  }

  // O(1), strong
  void push_back(const T& val) {
    if (size() == capacity()) {
      insert_with_copy(end(), val);
      return;
    }
    new (data_ + ((begin_ + size()) % capacity())) T(val);
    ++size_;
  }

  // O(1), strong
  void push_front(const T& val) {
    if (size() == capacity()) {
      insert_with_copy(begin(), val);
      return;
    }
    std::size_t new_begin = (begin_ + capacity() - 1) % capacity();
    new (data_ + new_begin) T(val);
    begin_ = new_begin;
    ++size_;
  }

  // O(1), nothrow
  void pop_back() {
    back().~T();
    --size_;
  }

  // O(1), nothrow
  void pop_front() {
    front().~T();
    begin_ = (begin_ + 1) % capacity();
    --size_;
  }

  // O(n), strong
  void reserve(size_t desired_capacity) {
    if (desired_capacity > capacity()) {
      circular_buffer tmp(desired_capacity);
      std::uninitialized_copy(begin(), end(), tmp.begin());
      tmp.size_ = size();
      swap(*this, tmp);
    }
  }

  // O(n), basic
  iterator insert(const_iterator pos, const T& val) {
    if (size() == capacity()) {
      return insert_with_copy(pos, val);
    }
    std::size_t idx = pos - begin();
    if ((pos - begin()) < (end() - pos)) {
      push_front(val);
      iterator cur = begin();
      for (std::size_t i = 0; i < idx; ++i) {
        std::iter_swap(cur, cur + 1);
        ++cur;
      }
    } else {
      push_back(val);
      iterator cur = end() - 1;
      for (std::size_t i = 0; i < size() - idx - 1; ++i) {
        std::iter_swap(cur, cur - 1);
        --cur;
      }
    }
    return begin() + idx;
  }

  // O(n), basic
  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  // O(n), basic
  iterator erase(const_iterator first, const_iterator last) {
    std::ptrdiff_t diff = first - begin();
    if ((first - begin()) < (end() - last)) {
      while (first > begin()) {
        --first;
        --last;
        std::swap(data_[first.pos_ % capacity()], data_[last.pos_ % capacity()]);
      }
      for (std::ptrdiff_t i = last - first; i > 0; i--) {
        pop_front();
      }
    } else {
      while (last < end()) {
        std::swap(data_[first.pos_ % capacity()], data_[last.pos_ % capacity()]);
        ++first;
        ++last;
      }
      for (std::ptrdiff_t i = last - first; i > 0; i--) {
        pop_back();
      }
    }
    return begin() + diff;
  }

  // O(n), nothrow
  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  }

  // O(1), nothrow
  friend void swap(circular_buffer& left, circular_buffer& right) noexcept {
    std::swap(left.data_, right.data_);
    std::swap(left.size_, right.size_);
    std::swap(left.capacity_, right.capacity_);
    std::swap(left.begin_, right.begin_);
  }

private:
  T* data_ = nullptr;
  std::size_t size_ = 0;
  std::size_t capacity_ = 0;
  std::size_t begin_ = 0;
};