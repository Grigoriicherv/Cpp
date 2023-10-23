#pragma once

#include <cstddef>
#include <memory>

template <typename T, typename Deleter = std::default_delete<T>>
class shared_ptr {
public:
  shared_ptr() noexcept = default;

  ~shared_ptr() {
    reset();
  }

  shared_ptr(std::nullptr_t) noexcept : _data(nullptr) {}

  explicit shared_ptr(T* ptr) : _data(ptr) {
    try {
      counter = new c_struct();
    } catch (...) {
      delete _data;
      _data = nullptr;
      counter = nullptr;
      throw;
    }
  }

  shared_ptr(T* ptr, Deleter deleter) {
    try {
      counter = new c_struct();

    } catch (...) {
      Deleter del;
      swap(deleter, del);
      del(ptr);
      _data = nullptr;
      counter = nullptr;
      throw;
    }
    _data = ptr;
    Deleter tmp_del = deleter;
    swap(counter->del, tmp_del);
  }

  shared_ptr(const shared_ptr& other) noexcept {
    _data = other._data;
    if (other.counter != nullptr) {

      counter = other.counter;
      counter->shared_count++;
    }
  }

  shared_ptr& operator=(const shared_ptr& other) noexcept {
    if (&other != this) {
      reset();
      _data = other.get();
      if (counter != nullptr) {
        counter->shared_count = ++other.counter->shared_count;
      }
    }
    return *this;
  }

  T* get() const noexcept {
    return _data;
  }

  explicit operator bool() const noexcept {
    return _data != nullptr;
  }

  T& operator*() const noexcept {
    return *_data;
  }

  T* operator->() const noexcept {
    return _data;
  }

  std::size_t use_count() const noexcept {
    if (counter == nullptr) {
      return 0;
    }
    return counter->shared_count;
  }

  void reset() noexcept {
    if (counter == nullptr) {
    } else if (counter->shared_count > 1) {
      counter->shared_count--;
    } else if (_data != nullptr) {
      counter->del(_data);
      delete counter;
      _data = nullptr;
      counter = nullptr;
    }
    if (_data == nullptr) {
      delete counter;
    }
  }

  void reset(T* new_ptr) {
    c_struct* tmp;
    try {
      tmp = new c_struct();

    } catch (...) {
      delete new_ptr;
      throw;
    }
    reset();
    _data = new_ptr;
    counter = tmp;
  }

  void reset(T* new_ptr, Deleter deleter) {
    c_struct* tmp;
    try {
      tmp = new c_struct();

    } catch (...) {
      deleter(new_ptr);
      throw;
    }
    deleter(_data);
    if (counter != nullptr) {
      delete counter;
    }
    _data = nullptr;
    counter = nullptr;
    _data = new_ptr;
    counter = tmp;
    Deleter tmp_del = deleter;
    swap(counter->del, tmp_del);
  }

  friend bool operator==(const shared_ptr& lhs, const shared_ptr& rhs) noexcept {
    return lhs._data == rhs._data;
  }

  friend bool operator!=(const shared_ptr& lhs, const shared_ptr& rhs) noexcept {
    return !(lhs == rhs);
  }

  struct c_struct {
    size_t shared_count = 1;
    Deleter del;
  };

private:
  T* _data = nullptr;
  c_struct* counter = nullptr;
};
