#ifndef REF_PTR_HPP
#define REF_PTR_HPP

#include <stddef.h>

/*
Simplified version of boost::intrusive_ptr for C++98.

Use like, ref_ptr<T*>

typename T must have `addRef()`, `release()` and interanl ref counter
*/
template <typename T>
class ref_ptr {
 public:
  ref_ptr(T* p = NULL) : _ptr(p) {
    if (_ptr) _ptr->addRef();
  }
  ~ref_ptr() {
    if (_ptr) _ptr->release();
  }
  ref_ptr(const ref_ptr& other) : _ptr(other._ptr) {
    if (_ptr) _ptr->addRef();
  }

  ref_ptr& operator=(const ref_ptr& other) {
    if (_ptr != other._ptr) {
      if (_ptr) _ptr->release();
      _ptr = other._ptr;
      if (_ptr) _ptr->addRef();
    }
    return *this;
  }
  T* operator->() const { return _ptr; }
  T& operator*() const { return *_ptr; }
  T* get() const { return _ptr; }

 private:
  T* _ptr;
};  // ref_ptr

#endif  // REFPTR_HPP