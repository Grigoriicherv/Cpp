#pragma once

#include <cstddef>
#include <iostream>
#include <type_traits>

inline constexpr size_t dynamic_extent = -1;

template <typename T, size_t Extent = dynamic_extent>
class contiguous_view {
  template <typename R, bool Dynamic>
  struct Data {};

  template <typename R>
  struct Data<R, true> {
    R* ptr = nullptr;
    size_t size = 0;
  };

  template <typename R>
  struct Data<R, false> {
    R* ptr = nullptr;
  };

public:
  using value_type = std::remove_cv_t<T>;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

  using iterator = pointer;
  using const_iterator = const_pointer;

public:
  contiguous_view() noexcept = default;

  template <typename It>
  explicit(Extent == dynamic_extent) contiguous_view(It first, size_t count) {
    _data.ptr = &(*first);
    if constexpr (dynamic_extent == Extent) {
      _data.size = count;
    }
  }

  template <typename It>
  explicit(Extent == dynamic_extent) contiguous_view(It first, It last) {
    if (last - first == 0) {
      _data.ptr = nullptr;
      if constexpr (dynamic_extent == Extent) {
        _data.size = 0;
      }
    } else {
      _data.ptr = &(*first);
      if constexpr (dynamic_extent == Extent) {
        _data.size = last - first;
      }
    }
  }

  contiguous_view(const contiguous_view& other) noexcept = default;

  template <typename U, size_t N>
  contiguous_view(const contiguous_view<U, N>& other) noexcept = delete;

  template <typename U>
  contiguous_view(const contiguous_view<U, Extent>& other) noexcept {
    _data.ptr = other.data();
    if constexpr (Extent == dynamic_extent) {
      _data.size = other.size();
    }
  }

  template <typename U, size_t N>
    requires(Extent == dynamic_extent)
  contiguous_view(const contiguous_view<U, N>& other) noexcept {
    _data.ptr = other.data();
    _data.size = N;
  }

  template <typename U>
    requires(Extent != dynamic_extent)
  explicit contiguous_view(const contiguous_view<U, dynamic_extent>& other) noexcept {
    _data.ptr = other.data();
  }

  contiguous_view& operator=(const contiguous_view& other) noexcept = default;

  pointer data() const noexcept {
    return _data.ptr;
  }

  size_t size() const noexcept {
    if constexpr (Extent == dynamic_extent) {
      return _data.size;
    } else {
      return Extent;
    }
  }

  size_t size_bytes() const noexcept {
    return size() * sizeof(T);
  }

  bool empty() const noexcept {
    return _data.ptr == nullptr;
  }

  iterator begin() const noexcept {
    return _data.ptr;
  }

  const_iterator cbegin() const noexcept {
    return _data.ptr;
  }

  iterator end() const noexcept {
    if constexpr (dynamic_extent == Extent) {
      return _data.ptr + _data.size;
    } else {
      return _data.ptr + Extent;
    }
  }

  const_iterator cend() const noexcept {
    if constexpr (dynamic_extent == Extent) {
      return _data.ptr + _data.size;
    } else {
      return _data.ptr + Extent;
    }
  }

  reference operator[](size_t idx) const {
    return _data.ptr[idx];
  }

  reference front() const {
    return *begin();
  }

  reference back() const {
    if constexpr (dynamic_extent == Extent) {
      return *(begin() + _data.size - 1);
    } else {
      return *(begin() + Extent - 1);
    }
  }

  contiguous_view<T, dynamic_extent> subview(size_t offset, size_t count = dynamic_extent) const {
    if (count == dynamic_extent) {
      return contiguous_view<T, dynamic_extent>(data() + offset, size() - offset);
    } else {
      return contiguous_view<T, dynamic_extent>(data() + offset, count);
    }
  }

  template <size_t Offset, size_t Count = dynamic_extent>
    requires(Count != dynamic_extent)
  contiguous_view<T, Count> subview() const {
    return contiguous_view<T, Count>(data() + Offset, Count);
  }

  template <size_t Offset, size_t Count = dynamic_extent>
    requires(Count == dynamic_extent)
  auto subview() const {
    return contiguous_view < T,
           (Extent == dynamic_extent) ? dynamic_extent : (Extent - Offset) > (data() + Offset, size() - Offset);
  }

  template <size_t Count>
  contiguous_view<T, Count> first() const {
    return contiguous_view<T, Count>(data(), Count);
  }

  contiguous_view<T, dynamic_extent> first(size_t count) const {
    return contiguous_view<T, dynamic_extent>(data(), count);
  }

  template <size_t Count>
  contiguous_view<T, Count> last() const {
    return contiguous_view<T, Count>(data() + size() - Count, Count);
  }

  contiguous_view<T, dynamic_extent> last(size_t count) const {
    return contiguous_view<T, dynamic_extent>(data() + size() - count, count);
  }

  template <typename R>
  struct bytes {
    using type = std::byte;
  };

  template <typename R>
  struct bytes<const R> {
    using type = const std::byte;
  };

  auto as_bytes() const {
    return contiguous_view < typename bytes<T>::type,
           (Extent == dynamic_extent)
               ? dynamic_extent
               : (Extent * 4) > (reinterpret_cast<typename bytes<T>::type*>(_data.ptr), size() * 4);
  }

private:
  Data<T, Extent == dynamic_extent> _data;
};
