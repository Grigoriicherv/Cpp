#pragma once

#include <algorithm>
#include <memory>
#include <utility>

template <typename T, size_t SMALL_SIZE>
class socow_vector {

  struct dynamic_buffer {
    size_t capacity_;
    size_t ref_count;
    T dynamic_data[0];

    explicit dynamic_buffer(std::size_t capacity) : capacity_(capacity), ref_count(1) {}
  };

  void swap_static_and_static(socow_vector& other) {
    std::uninitialized_copy(other.begin() + size(), other.end(), end());
    std::destroy(other.begin() + size(), other.end());
    std::swap(size_, other.size_);
    std::swap_ranges(begin(), begin() + other.size(), other.begin());
  }

  void swap_static_and_dynamic(socow_vector& stat, socow_vector& dynamic) {
    dynamic_buffer* tmp_dynamic_buffer = dynamic.dynamic_buffer_;
    dynamic.dynamic_buffer_ = nullptr;
    try {
      std::uninitialized_copy_n(stat.static_data, stat.size_, dynamic.static_data);
    } catch (...) {
      dynamic.dynamic_buffer_ = tmp_dynamic_buffer;
      throw;
    }
    std::destroy_n(stat.static_data, stat.size_);
    stat.dynamic_buffer_ = tmp_dynamic_buffer;
  }

  void unshare() {
    if (dynamic_ && dynamic_buffer_->ref_count > 1) {
      if (size_ > SMALL_SIZE) {
        change_capacity(dynamic_buffer_->capacity_);
      } else {
        change_capacity(SMALL_SIZE);
      }
    }
  }

  void change_capacity(size_t new_capacity) {
    if (new_capacity <= SMALL_SIZE && dynamic_) {
      dynamic_buffer* tmp = dynamic_buffer_;
      try {
        std::uninitialized_copy(tmp->dynamic_data, tmp->dynamic_data + size_, static_data);
        if (--tmp->ref_count == 0) {
          for (size_t i = size_; i > 0; --i) {
            tmp->dynamic_data[i - 1].~T();
          }
          operator delete(tmp);
        }
        dynamic_ = false;
      } catch (...) {
        dynamic_buffer_ = tmp;
        throw;
      }
    } else if (new_capacity > SMALL_SIZE) {
      socow_vector tmp(new_capacity);
      tmp.dynamic_ = true;
      if (dynamic_) {
        std::uninitialized_copy(dynamic_buffer_->dynamic_data, dynamic_buffer_->dynamic_data + size_, tmp.data());
      } else {
        std::uninitialized_copy(static_data, static_data + size_, tmp.data());
      }
      tmp.size_ = size_;
      *this = tmp;
    }
  }

  static dynamic_buffer* allocate_buffer(size_t new_cap) {
    auto* new_dynamic_buffer = static_cast<dynamic_buffer*>(operator new(sizeof(dynamic_buffer) + sizeof(T) * new_cap));
    new (new_dynamic_buffer) dynamic_buffer(new_cap);
    return new_dynamic_buffer;
  }

  size_t size_{0};
  bool dynamic_{false};

  union {
    T static_data[SMALL_SIZE];
    dynamic_buffer* dynamic_buffer_;
  };

  void clear_dynamic() {
    if (dynamic_buffer_->ref_count > 1) {
      dynamic_buffer_->ref_count--;
    } else {
      std::destroy_n(dynamic_buffer_->dynamic_data, size_);
      operator delete(dynamic_buffer_);
    }
  }

public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = pointer;
  using const_iterator = const_pointer;

  socow_vector() {}

  socow_vector(const socow_vector& other) : size_(other.size_), dynamic_(other.dynamic_) {
    if (other.dynamic_) {
      dynamic_buffer_ = other.dynamic_buffer_;
      dynamic_buffer_->ref_count++;
    } else {
      std::uninitialized_copy(other.static_data, other.static_data + other.size_, static_data);
    }
  }

  explicit socow_vector(size_t capacity) {
    dynamic_buffer_ = allocate_buffer(capacity);
  }

  socow_vector& operator=(const socow_vector& other) {
    if (this != &other) {
      if (other.dynamic_) {
        clear();
        if (dynamic_) {
          operator delete(dynamic_buffer_);
        }
        dynamic_buffer_ = other.dynamic_buffer_;
        dynamic_buffer_->ref_count++;
        size_ = other.size_;
        dynamic_ = true;
      } else if (!other.dynamic_ && dynamic_) {
        dynamic_buffer* tmp = dynamic_buffer_;
        try {
          std::uninitialized_copy(other.static_data, other.static_data + other.size_, static_data);
          dynamic_ = false;
        } catch (...) {
          dynamic_buffer_ = tmp;
          throw;
        }
        std::destroy_n(tmp->dynamic_data, size_);
        operator delete(tmp);
        size_ = other.size_;
      } else {
        std::size_t min_size = std::min(size(), other.size());
        socow_vector tmp;
        std::uninitialized_copy_n(other.begin(), min_size, tmp.begin());
        tmp.size_ = min_size;
        if (size() < other.size()) {
          std::uninitialized_copy(other.begin() + size(), other.end(), end());
        } else {
          std::destroy(begin() + other.size(), end());
        }
        size_ = other.size();
        std::swap_ranges(begin(), begin() + min_size, tmp.begin());
      }
    }
    return *this;
  }

  ~socow_vector() {
    if (dynamic_) {
      clear_dynamic();
    } else {
      std::destroy_n(static_data, size_);
    }
  }

  T& operator[](size_t i) {
    return data()[i];
  }

  const T& operator[](size_t i) const {
    return data()[i];
  }

  T* data() {
    unshare();
    return dynamic_ ? dynamic_buffer_->dynamic_data : static_data;
  }

  const T* data() const {
    return dynamic_ ? dynamic_buffer_->dynamic_data : static_data;
  }

  size_t size() const {
    return size_;
  }

  T& front() {
    return data()[0];
  }

  const T& front() const {
    return data()[0];
  }

  T& back() {
    return data()[size_ - 1];
  }

  const T& back() const {
    return data()[size_ - 1];
  }

  void push_back(const T& value) {
    if (size() == capacity() || (dynamic_ && dynamic_buffer_->ref_count > 1)) {
      socow_vector tmp(capacity() * 2);
      tmp.dynamic_ = true;
      std::uninitialized_copy_n(std::as_const(*this).data(), size_, tmp.data());
      tmp.size_ = size_;
      new (tmp.data() + size_) T(value);
      tmp.size_++;
      *this = tmp;
    } else {
      new (data() + size_) T(value);
      size_++;
    }
  }

  void pop_back() {
    if (dynamic_ && dynamic_buffer_->ref_count > 1) {
      socow_vector tmp(capacity() * 2);
      tmp.dynamic_ = true;
      std::uninitialized_copy_n(std::as_const(*this).data(), size_ - 1, tmp.data());
      tmp.size_ = size_ - 1;
      swap(tmp);
    } else {
      data()[--size_].~T();
    }
  }

  bool empty() const {
    return size_ == 0;
  }

  size_t capacity() const {
    return dynamic_ ? dynamic_buffer_->capacity_ : SMALL_SIZE;
  }

  void reserve(size_t new_capacity) {
    if (capacity() < new_capacity || (new_capacity >= size_ && (dynamic_ && dynamic_buffer_->ref_count > 1))) {
      change_capacity(new_capacity);
    } else if (new_capacity > size_) {
      unshare();
    }
  }

  void shrink_to_fit() {
    if (size_ != capacity()) {
      change_capacity(size_);
    }
  }

  void clear() {
    if (dynamic_ && dynamic_buffer_->ref_count > 1) {
      dynamic_buffer_->ref_count--;
      dynamic_buffer_ = allocate_buffer(dynamic_buffer_->capacity_);
      size_ = 0;
    }
    if (dynamic_) {
      std::destroy_n(dynamic_buffer_->dynamic_data, size_);
    } else {
      std::destroy_n(static_data, size_);
    }
    size_ = 0;
  }

  void swap(socow_vector& other) {
    if (this == &other) {
      return;
    }
    if (dynamic_ && other.dynamic_) {
      std::swap(dynamic_buffer_, other.dynamic_buffer_);
    } else if (dynamic_ && !other.dynamic_) {
      swap_static_and_dynamic(other, *this);
    } else if (!dynamic_ && other.dynamic_) {
      swap_static_and_dynamic(*this, other);
    } else {
      if (size_ < other.size_) {
        swap_static_and_static(other);
        return;
      } else {
        other.swap_static_and_static(*this);
        return;
      }
    }
    std::swap(size_, other.size_);
    std::swap(dynamic_, other.dynamic_);
  }

  iterator begin() {
    return data();
  }

  iterator end() {
    return data() + size_;
  }

  const_iterator begin() const {
    return data();
  }

  const_iterator end() const {
    return data() + size_;
  }

  iterator insert(socow_vector::const_iterator pos, const T& value) {
    size_t ind = std::distance(std::as_const(*this).data(), pos);
    if ((!dynamic_ && size_ < capacity()) || (dynamic_ && dynamic_buffer_->ref_count == 1 && size_ < capacity())) {
      push_back(value);
      using std::swap;
      for (size_t i = size_ - 1; i > ind; --i) {
        swap(data()[i - 1], data()[i]);
      }
    } else {
      size_t new_cap = size_ == capacity() ? capacity() * 2 : capacity();
      socow_vector tmp(new_cap);
      tmp.dynamic_ = true;
      std::uninitialized_copy_n(std::as_const(*this).data(), ind, tmp.data());
      tmp.size_ = ind;
      new (tmp.data() + ind) T(value);
      tmp.size_++;
      std::uninitialized_copy(std::as_const(*this).data() + ind, std::as_const(*this).data() + size_,
                              tmp.data() + ind + 1);
      tmp.size_ = size_ + 1;
      *this = tmp;
    }
    return data() + ind;
  }

  iterator erase(socow_vector::const_iterator pos) {
    return erase(pos, pos + 1);
  }

  iterator erase(socow_vector::const_iterator first, socow_vector::const_iterator last) {
    size_t ind1 = std::distance(std::as_const(*this).data(), first);
    size_t ind2 = std::distance(std::as_const(*this).data(), last);
    if (((!dynamic_) || (dynamic_ && dynamic_buffer_->ref_count == 1)) && ind1 != ind2) {
      using std::swap;
      for (size_t i = 0; i < size() - ind2; ++i) {
        swap(data()[ind1 + i], data()[ind2 + i]);
      }
      std::destroy(end() - ind2 + ind1, end());
      size_ -= ind2 - ind1;
    } else if (ind1 != ind2) {
      socow_vector tmp(size_ == capacity() ? capacity() * 2 : capacity());
      tmp.dynamic_ = true;
      std::uninitialized_copy_n(std::as_const(*this).data(), ind1, tmp.begin());
      tmp.size_ = ind1;
      std::uninitialized_copy(std::as_const(*this).data() + ind2, std::as_const(*this).data() + size_,
                              tmp.begin() + ind1);
      tmp.size_ = size_ - (ind2 - ind1);
      *this = tmp;
    }
    return data() + ind1;
  }
};
