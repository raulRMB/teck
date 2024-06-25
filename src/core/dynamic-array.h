#ifndef JET_DYNAMIC_ARRAY_H
#define JET_DYNAMIC_ARRAY_H

#include "types.h"

namespace jet
{

template <typename T> class DynamicArray
{
public:
  DynamicArray(size_t size = 1) : mSize(size), mCapacity(size), mData(new T[size])
  {
  }

  ~DynamicArray()
  {
    delete[] mData;
  }

  T &operator[](size_t index)
  {
    return mData[index];
  }

  const T &operator[](size_t index) const
  {
    return mData[index];
  }

  size_t Size() const
  {
    return mSize;
  }

  void PushBack(const T &value)
  {
    if (mSize == mCapacity)
    {
      Resize();
    }
    mData[mSize++] = value;
  }

  T *begin()
  {
    return mData;
  }

  T *end()
  {
    return mData + mSize;
  }

  const T *begin() const
  {
    return mData;
  }

  const T *end() const
  {
    return mData + mSize;
  }

  T *Data()
  {
    return mData;
  }

private:
  void Resize()
  {
    mCapacity = mCapacity * 2;
    T *newData = new T[mCapacity];
    std::copy(mData, mData + mSize, newData);
    delete[] mData;
    mData = newData;
  }

  size_t mSize = 1;
  size_t mCapacity = 1;
  T *mData{};
};

} // namespace jet

#endif // JET_DYNAMIC_ARRAY_H
