#pragma once

#include <cstddef>
#include <memory>

class linked_ptr_internal {
public:
  explicit operator bool() const noexcept {
    return next_ != nullptr;
  }

  void nulling() {
    next_ = nullptr;
    prev_ = nullptr;
  }

  void join_new() {
    next_ = this;
    prev_ = this;
  }

  void join(const linked_ptr_internal* ptr) {
    prev_ = ptr;
    next_ = ptr->next_;
    next_->prev_ = this;
    ptr->next_ = this;
  }

  bool depart() {
    if (next_ == this) {
      return true;
    }
    prev_->next_ = next_;
    next_->prev_ = prev_;
    return false;
  }

  size_t size() const {
    const linked_ptr_internal* p = next_;
    size_t res = 1;
    while (p != this) {
      p = p->next_;
      res++;
    }
    return res;
  }

private:
  mutable const linked_ptr_internal* next_ = nullptr;
  mutable const linked_ptr_internal* prev_ = nullptr;
};

template <typename T, typename Deleter = std::default_delete<T>>
class linked_ptr {
public:

public:
  linked_ptr() noexcept = default;

  ~linked_ptr() {
    reset();
  }

  linked_ptr(std::nullptr_t) noexcept : value_(nullptr) {}

  explicit linked_ptr(T* ptr) {
    capture(ptr);
  }

  linked_ptr(T* ptr, Deleter deleter) : del(deleter) {
    capture(ptr);
  }

  linked_ptr(const linked_ptr& other) noexcept {
    copy(&other);
  }

  template <typename Y, typename D, typename = std::enable_if_t<std::is_base_of_v<T, Y>>,
            typename = std::enable_if_t<std::is_constructible_v<Deleter, D>>>
  linked_ptr(const linked_ptr<Y, D>& other) noexcept : del(other.del) {
    copy(&other);
  }

  template <typename Y, typename D, typename = std::enable_if_t<std::is_same_v<T, const Y>>>
  linked_ptr(const linked_ptr<Y, D>& other) noexcept : del(other.del) {
    copy(&other);
  }

  template <typename Y, typename = std::enable_if_t<std::is_same_v<const Y, T>>>
  linked_ptr(const linked_ptr<Y, Deleter>& other) noexcept : del(other.del) {
    copy(&other);
  }

  linked_ptr& operator=(const linked_ptr& other) noexcept {
    if (&other != this) {
      depart();
      copy(&other); // del = deleter;
    }
    return *this;
  }

  template <typename Y, typename D>
    requires((std::is_base_of_v<T, Y> && std::is_constructible_v<Deleter, D>) ||
             (std::is_same_v<Deleter, D> && std::is_same_v<const Y, T>))
  linked_ptr& operator=(const linked_ptr<Y, D>& other) noexcept {
    depart();
    copy(&other);
    return *this;
  }

  T* get() const noexcept {
    return value_;
  }

  explicit operator bool() const noexcept {
    return value_ != nullptr;
  }

  T& operator*() const noexcept {
    return *value_;
  }

  T* operator->() const noexcept {
    return value_;
  }

  std::size_t use_count() const noexcept {
    return size();
  }

  void reset() noexcept {
    depart();
  }

  void reset(T* new_ptr) {
    depart();
    capture(new_ptr);
  }

  void reset(T* new_ptr, Deleter deleter) {
    link_.nulling();
    deleter(value_);
    capture(new_ptr);
    Deleter tmp = deleter;
    swap(del, tmp);
  }

  friend bool operator==(const linked_ptr& lhs, const linked_ptr& rhs) noexcept {
    return lhs.value_ == rhs.value_;
  }

  friend bool operator!=(const linked_ptr& lhs, const linked_ptr& rhs) noexcept {
    return !(lhs == rhs);
  }

private:
  template <typename U, typename D>
  friend class linked_ptr;

  T* value_ = nullptr;
  linked_ptr_internal link_;
  Deleter del;

  void depart() {
    if (value_ && link_.depart()) {
      link_.nulling();
      del(value_);
      value_ = nullptr;
    }
  }

  void capture(T* ptr) {
    value_ = ptr;
    link_.join_new();
  }

  size_t size() const {
    if (!value_ && !link_) {
      return 0;
    }
    return link_.size();
  }

  template <typename U, typename D>
  void copy(const linked_ptr<U, D>* ptr) {
    value_ = ptr->get();
    if (value_) {
      link_.join(&ptr->link_);
    } else {
      link_.join_new();
    }
  }
};
