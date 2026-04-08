#pragma once

#include <cstring>

#ifndef SJTU_EXCEPTIONS_HPP
#define SJTU_EXCEPTIONS_HPP

#include <cstddef>
#include <cstring>
#include <string>

namespace sjtu {

class exception {
protected:
        const std::string variant = "";
        std::string detail = "";
public:
        exception() {}
        exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}
        virtual std::string what() {
                return variant + " " + detail;
        }
};

class index_out_of_bound : public exception {
};

class runtime_error : public exception {
};

class invalid_iterator : public exception {
};

class container_is_empty : public exception {
};
}

#endif

namespace sjtu {

using out_of_range = index_out_of_bound;

template <class T>
class vector {
public:
  vector();
  ~vector();
  explicit vector(std::size_t size);
  vector(std::size_t size, const T& t);

  vector(const vector& other);
  vector(vector&& other) noexcept;
  vector& operator=(vector other);

  void swap(vector& other) noexcept;

  void push_back(const T& t);
  void push_back(T&& t);
  void pop_back();

  T& front();
  const T& front() const;
  T& back();
  const T& back() const;

  T& operator[](std::size_t pos) noexcept;
  const T& operator[](std::size_t pos) const noexcept;
  T& at(std::size_t pos);
  const T& at(std::size_t pos) const;

  // void resize(std::size_t size);
  // void resize(std::size_t size, const T& t);
  void reserve(std::size_t capacity);
  void clear();

  [[nodiscard]] std::size_t size() const { return right_ - left_; }
  [[nodiscard]] std::size_t capacity() const { return cap_ - left_; }
  [[nodiscard]] bool empty() const { return right_ == left_; }

  template <bool IsConst>
  class iterator_base;

  using iterator = iterator_base<false>;
  using const_iterator = iterator_base<true>;

  iterator insert(iterator it, const T& t);
  iterator insert(iterator it, T&& t);
  iterator insert(std::size_t pos, const T& t);
  iterator insert(std::size_t pos, T&& t);

  iterator erase(iterator it);
  iterator erase(std::size_t pos);

  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  const_iterator cbegin() const;
  const_iterator cend() const;

private:
  void _construct(T* ptr);
  void _construct(T* ptr, const T& t);
  void _construct(T* ptr, T&& t);
  void _destroy(T* ptr);
  T* _alloc(std::size_t size);
  void _dealloc(T* ptr);
  T* _realloc(T* ptr, std::size_t old_size, std::size_t size);
  /**
   * Supposes that src ~ src+len objects are constructed
   * while dst ~ dst+len objects are not.
   */
  void _memcpy(T* dst, T* src, std::size_t len);
  /**
   * Supposes that src ~ src+len objects are constructed while dst ~ dst+len objects
   * are not RESPECTIVELY at the moment of their VERY move.
   */
  void _memmove(T* dst, T* src, std::size_t len);
  T *left_, *right_, *cap_;
};


template <class T>
template <bool IsConst>
class vector<T>::iterator_base {
public:
  using value_type = std::conditional_t<IsConst, const T, T>;
  using reference = value_type&;
  using pointer = value_type*;

  iterator_base();
  ~iterator_base();

  explicit iterator_base(pointer ptr);
  template <bool OtherIsConst, std::enable_if_t<IsConst || (!OtherIsConst), int> = 0>
  explicit iterator_base(iterator_base<OtherIsConst>& other);

  iterator_base operator+(std::size_t n);
  iterator_base operator-(std::size_t n);
  iterator_base& operator+=(std::size_t n);
  iterator_base& operator-=(std::size_t n);

  std::size_t operator-(const iterator_base& other) const;

  iterator_base operator++(int);
  iterator_base operator--(int);
  iterator_base& operator++();
  iterator_base& operator--();

  reference operator*() const noexcept;
  pointer operator->() const noexcept;

  template <bool OtherIsConst>
  bool operator==(const iterator_base<OtherIsConst>& other) const;
  template <bool OtherIsConst>
  bool operator!=(const iterator_base<OtherIsConst>& other) const;

  pointer base() const noexcept;

private:
  pointer ptr_;

  template <bool OtherIsConst>
  friend class iterator_base;
};

/*************************************************************************************
 * Implementation
 ************************************************************************************/

template <class T>
vector<T>::vector() {
  left_ = right_ = cap_ = nullptr;
}

template <class T>
vector<T>::~vector() {
  clear();
  _dealloc(left_);
}

template <class T>
vector<T>::vector(std::size_t size) {
  T* buf = _alloc(size);
  left_ = buf;
  right_ = cap_ = left_ + size;
  for (std::size_t i = 0; i < size; ++i) {
    _construct(left_ + i);
  }
}

template <class T>
vector<T>::vector(std::size_t size, const T& t) {
  T* buf = _alloc(size);
  left_ = buf;
  right_ = cap_ = left_ + size;
  for (std::size_t i = 0; i < size; ++i) {
    _construct(left_ + i, t);
  }
}

template <class T>
vector<T>::vector(const vector& other) {
  std::size_t size = other.size();
  T* buf = _alloc(size);
  left_ = buf;
  right_ = cap_ = left_ + size;
  for (std::size_t i = 0; i < size; ++i) {
    _construct(left_ + i, other[i]);
  }
}

template <class T>
vector<T>::vector(vector&& other) noexcept: vector() {
  swap(other);
}

template <class T>
vector<T>& vector<T>::operator=(vector other) {
  swap(other);
  return *this;
}

template <class T>
void vector<T>::swap(vector& other) noexcept {
  std::swap(left_, other.left_);
  std::swap(right_, other.right_);
  std::swap(cap_, other.cap_);
}

template <class T>
void vector<T>::push_back(const T& t) {
  if (right_ == cap_) {
    reserve(std::max(size() + 1, capacity() * 2));
  }
  _construct(right_, t);
  ++right_;
}

template <class T>
void vector<T>::push_back(T&& t) {
  if (right_ == cap_) {
    reserve(std::max(size() + 1, capacity() * 2));
  }
  _construct(right_, std::move(t));
  ++right_;
}

template <class T>
void vector<T>::pop_back() {
  if (empty()) {
    throw out_of_range();
  }
  --right_;
  _destroy(right_);
}

template <class T>
T& vector<T>::front() {
  if (empty()) {
    throw out_of_range();
  }
  return *left_;
}

template <class T>
const T& vector<T>::front() const {
  if (empty()) {
    throw out_of_range();
  }
  return *left_;
}

template <class T>
T& vector<T>::back() {
  if (empty()) {
    throw out_of_range();
  }
  return *(right_ - 1);
}

template <class T>
const T& vector<T>::back() const {
  return *(right_ - 1);
}

template <class T>
T& vector<T>::operator[](std::size_t pos) noexcept {
  return *(left_ + pos);
}

template <class T>
const T& vector<T>::operator[](std::size_t pos) const noexcept {
  if (pos > size()) {
    throw out_of_range();
  }
  return *(left_ + pos);
}

template <class T>
T& vector<T>::at(std::size_t pos) {
  if (pos > size()) {
    throw out_of_range();
  }
  return *(left_ + pos);
}

template <class T>
const T& vector<T>::at(std::size_t pos) const {
  if (pos > size()) {
    throw out_of_range();
  }
  return *(left_ + pos);
}

template <class T>
void vector<T>::reserve(std::size_t capacity) {
  if (capacity <= cap_ - left_) {
    return;
  }
  std::size_t size = right_ - left_;
  left_ = _realloc(left_, cap_ - left_, capacity);
  right_ = left_ + size;
  cap_ = left_ + capacity;
}

template <class T>
void vector<T>::clear() {
  for (T* ptr = left_; ptr != right_; ++ptr) {
    _destroy(ptr);
  }
  right_ = left_;
}

template <class T>
typename vector<T>::iterator vector<T>::insert(iterator it, const T& t) {
  return insert(it.base() - left_, t);
}

template <class T>
typename vector<T>::iterator vector<T>::insert(iterator it, T&& t) {
  return insert(it.base() - left_, t);
}

template <class T>
typename vector<T>::iterator vector<T>::insert(std::size_t pos, const T& t) {
  if (pos >= size()) {
    throw out_of_range();
  }
  if (right_ == cap_) {
    reserve(std::max(size() + 1, capacity() * 2));
  }
  _memmove(left_ + pos + 1, left_ + pos, size() - pos);
  _construct(left_ + pos, t);
  ++right_;
  return iterator(left_ + pos);
}

template <class T>
typename vector<T>::iterator vector<T>::insert(std::size_t pos, T&& t) {
  if (pos >= size()) {
    throw out_of_range();
  }
  if (right_ == cap_) {
    reserve(std::max(size() + 1, capacity() * 2));
  }
  _memmove(left_ + pos + 1, left_ + pos, size() - pos);
  _construct(left_ + pos, std::move(t));
  ++right_;
  return iterator(left_ + pos);
}

template <class T>
typename vector<T>::iterator vector<T>::erase(iterator it) {
  return erase(it.base() - left_);
}

template <class T>
typename vector<T>::iterator vector<T>::erase(std::size_t pos) {
  if (pos > size()) {
    throw out_of_range();
  }
  _destroy(left_ + pos);
  _memmove(left_ + pos, left_ + pos + 1, size() - pos);
  --right_;
  return iterator(left_ + pos);
}

template <class T>
typename vector<T>::iterator vector<T>::begin() {
  return iterator(left_);
}

template <class T>
typename vector<T>::iterator vector<T>::end() {
  return iterator(right_);
}

template <class T>
typename vector<T>::const_iterator vector<T>::begin() const {
  return const_iterator(left_);
}

template <class T>
typename vector<T>::const_iterator vector<T>::end() const {
  return const_iterator(right_);
}

template <class T>
typename vector<T>::const_iterator vector<T>::cbegin() const {
  return const_iterator(left_);
}

template <class T>
typename vector<T>::const_iterator vector<T>::cend() const {
  return const_iterator(right_);
}

template <class T>
void vector<T>::_construct(T* ptr) {
  if constexpr (std::is_trivially_constructible_v<T>) {
    std::memset(ptr, 0, sizeof(T));
  } else {
    new(ptr) T();
  }
}

template <class T>
void vector<T>::_construct(T* ptr, const T& t) {
  if constexpr (std::is_trivially_copy_constructible_v<T>) {
    std::memmove(ptr, &t, sizeof(T));
  } else {
    new(ptr) T(t);
  }
}

template <class T>
void vector<T>::_construct(T* ptr, T&& t) {
  if constexpr (std::is_trivially_move_constructible_v<T>) {
    std::memmove(ptr, &t, sizeof(T));
  } else {
    new(ptr) T(std::move(t));
  }
}

template <class T>
void vector<T>::_destroy(T* ptr) {
  if constexpr (!std::is_trivially_destructible_v<T>) {
    ptr->~T();
  }
}

template <class T>
T* vector<T>::_alloc(std::size_t size) {
  return static_cast<T*>(std::malloc(size * sizeof(T)));
}

template <class T>
void vector<T>::_dealloc(T* ptr) {
  std::free(ptr);
}

template <class T>
T* vector<T>::_realloc(T* ptr, std::size_t old_size, std::size_t size) {
  if constexpr (std::is_trivially_copyable_v<T>) {
    return static_cast<T*>(std::realloc(ptr, size * sizeof(T)));
  } else {
    T* buf = _alloc(size);
    for (std::size_t i = 0; i < std::min(old_size, size); ++i) {
      _construct(buf + i, std::move(*(ptr + i)));
    }
    for (std::size_t i = 0; i < old_size; ++i) {
      _destroy(ptr + i);
    }
    _dealloc(ptr);
    return buf;
  }
}

template <class T>
void vector<T>::_memcpy(T* dst, T* src, std::size_t len) {
  if constexpr (std::is_trivially_copyable_v<T>) {
    std::memcpy(dst, src, len * sizeof(T));
  } else {
    for (std::size_t i = 0; i < len; ++i) {
      new(dst + i) T(std::move(*(src + i)));
    }
    for (std::size_t i = 0; i < len; ++i) {
      (src + i)->~T();
    }
  }
}

template <class T>
void vector<T>::_memmove(T* dst, T* src, std::size_t len) {
  if constexpr (std::is_trivially_copyable_v<T>) {
    std::memmove(dst, src, len * sizeof(T));
  } else {
    if (dst < src) {
      for (std::size_t i = 0; i < len; ++i) {
        new(dst + i) T(std::move(*(src + i)));
        (src + i)->~T();
      }
    } else {
      for (std::size_t i = len; i > 0; --i) {
        std::size_t j = i - 1;
        new(dst + j) T(std::move(*(src + j)));
        (src + j)->~T();
      }
    }
  }
}

template <class T>
template <bool IsConst>
vector<T>::iterator_base<IsConst>::iterator_base() {
  ptr_ = nullptr;
}

template <class T>
template <bool IsConst>
vector<T>::iterator_base<IsConst>::~iterator_base() = default;

template <class T>
template <bool IsConst>
vector<T>::iterator_base<IsConst>::iterator_base(pointer ptr) {
  ptr_ = ptr;
}

template <class T>
template <bool IsConst>
template <bool OtherIsConst, std::enable_if_t<IsConst || (!OtherIsConst), int>>
vector<T>::iterator_base<IsConst>::iterator_base(iterator_base<OtherIsConst>& other) {
  ptr_ = other.ptr_;
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst> vector<T>::iterator_base<IsConst>::operator+(std::size_t n) {
  return iterator_base(ptr_ + n);
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst> vector<T>::iterator_base<IsConst>::operator-(std::size_t n) {
  return iterator_base(ptr_ - n);
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst>& vector<T>::iterator_base<IsConst>::operator+=(std::size_t n) {
  ptr_ += n;
  return *this;
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst>& vector<T>::iterator_base<IsConst>::operator-=(std::size_t n) {
  ptr_ -= n;
  return *this;
}

template <class T>
template <bool IsConst>
std::size_t vector<T>::iterator_base<IsConst>::operator-(const iterator_base& other) const {
  return ptr_ - other.ptr_;
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst> vector<T>::iterator_base<IsConst>::operator++(int) {
  auto tmp = *this;
  ++ptr_;
  return tmp;
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst> vector<T>::iterator_base<IsConst>::operator--(int) {
  auto tmp = *this;
  --ptr_;
  return tmp;
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst>& vector<T>::iterator_base<IsConst>::operator++() {
  ++ptr_;
  return *this;
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst>& vector<T>::iterator_base<IsConst>::operator--() {
  --ptr_;
  return *this;
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst>::reference vector<T>::iterator_base<IsConst>::operator
*() const noexcept {
  return *ptr_;
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst>::pointer vector<T>::iterator_base<IsConst>::operator
->() const noexcept {
  return ptr_;
}

template <class T>
template <bool IsConst>
template <bool OtherIsConst>
bool vector<T>::iterator_base<IsConst>::operator==(const iterator_base<OtherIsConst>& other) const {
  return ptr_ == other.ptr_;
}

template <class T>
template <bool IsConst>
template <bool OtherIsConst>
bool vector<T>::iterator_base<IsConst>::operator!=(const iterator_base<OtherIsConst>& other) const {
  return ptr_ != other.ptr_;
}

template <class T>
template <bool IsConst>
typename vector<T>::template iterator_base<IsConst>::pointer vector<T>::iterator_base<IsConst>::base() const noexcept {
  return ptr_;
}

} // namespace sjtu