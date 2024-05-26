#include <vector>
#include "def.h"

namespace jet
{

template <typename T>
class Iterator
{
public:
  
  Iterator& operator++()
  {
    ++ptr;
    return *this;
  }

  T& operator*() const 
  {
    return *ptr;
  }

  bool operator != (const Iterator& other) const
  {
    return ptr != other.ptr;
  }

private:
  T* ptr;
};

template <typename T>
class dArray
{
public:
  dArray() {}
  dArray(u32 size) : vec(size) {}
  
  void PushBack(T& obj)
  {
    vec.push_back(obj);
  }

  void PushBack(T&& obj)
  {
    vec.push_back(std::move(obj));
  }

  void PopBack()
  {
    vec.pop_back();
  }

  void Reserve(size_t size)
  {
    vec.reserve(size);
  }

  void Resize(size_t size)
  {
    vec.resize(size);
  }

  void Clear()
  {
    vec.clear();
  }

  T& Back()
  {
    return vec.back();
  }

  size_t Size()
  {
    return vec.size();
  }

  T* Data()
  {
    return vec.data();
  }

  Iterator<T> Begin()
  {
    return Iterator(vec.begin());
  }

  Iterator<T> End()
  {
    return Iterator(vec.end());
  }
  
private:
  std::vector<T> vec;
};
  
}
