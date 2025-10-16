/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <cm/type_traits>

#include "cmSbomSerializer.h"

class cmSbomObject;

template <typename T>
inline void SerializeDispatch(T const& t, cmSbomSerializer& serializer)
{
  serializer.BeginObject();
  t.Serialize(serializer);
  serializer.EndObject();
}

template <typename T>
inline void SerializeDispatch(T* p, cmSbomSerializer& serializer)
{
  if (p->SpdxId) {
    serializer.AddReference(*p->SpdxId);
  }
}

inline void SerializeDispatch(std::nullptr_t, cmSbomSerializer&)
{
}

struct ObjectInterface
{
  virtual ObjectInterface* Copy(void* storage) const = 0;
  virtual ObjectInterface* Move(void* storage) = 0;
  virtual void* Addr() noexcept = 0;
  virtual void const* Addr() const noexcept = 0;
  virtual void SerializeImpl(cmSbomSerializer& os) const = 0;
  virtual ~ObjectInterface() = default;

  ObjectInterface() = default;
  ObjectInterface(ObjectInterface const&) = default;
  ObjectInterface& operator=(ObjectInterface const&) = default;
  ObjectInterface(ObjectInterface&&) noexcept = default;
  ObjectInterface& operator=(ObjectInterface&&) noexcept = default;
};

namespace impl {

#if defined(__GLIBCXX__) && __GLIBCXX__ <= 20150623
using max_align_t = ::max_align_t;
#else
using max_align_t = std::max_align_t;
#endif

enum class StorageKind
{
  Stack,
  Heap
};

template <typename, StorageKind>
struct Storage
{
};

template <typename T>
struct Storage<T, StorageKind::Stack> : ObjectInterface
{
  using ValueType = cm::decay_t<T>;

  explicit Storage(T x)
    : Value(std::move(x))
  {
  }

  Storage(Storage&&) noexcept(
    std::is_nothrow_move_constructible<ValueType>::value) = default;
  ~Storage() override = default;

  void* Addr() noexcept override { return std::addressof(Value); }
  void const* Addr() const noexcept override { return std::addressof(Value); }

  ValueType& get() { return Value; }
  ValueType const& get() const { return Value; }

private:
  ValueType Value;
};

template <typename T>
struct Storage<T, StorageKind::Heap> : ObjectInterface
{
  using ValueType = cm::decay_t<T>;

  explicit Storage(T x)
    : Ptr(new T(std::move(x)))
  {
  }

  Storage(Storage&&) noexcept = default;
  ~Storage() override = default;

  void* Addr() noexcept override { return Ptr.get(); }
  void const* Addr() const noexcept override { return Ptr.get(); }

  ValueType& get() { return *Ptr; }
  ValueType const& get() const { return *Ptr; }

private:
  std::unique_ptr<ValueType> Ptr;
};

template <>
struct Storage<std::nullptr_t, StorageKind::Stack> : ObjectInterface
{
  explicit Storage(std::nullptr_t) {}

  Storage(Storage&&) noexcept = default;
  ~Storage() override = default;

  void* Addr() noexcept override { return nullptr; }
  void const* Addr() const noexcept override { return nullptr; }

  void SerializeImpl(cmSbomSerializer&) const override {}

  std::nullptr_t get() { return nullptr; }
  std::nullptr_t get() const { return nullptr; }
};

}

struct ObjectStorage
{
  template <typename T>
  using Stack = impl::Storage<T, impl::StorageKind::Stack>;
  template <typename T>
  using Heap = impl::Storage<T, impl::StorageKind::Heap>;

  static constexpr std::size_t BufferSize = 128u;

  static constexpr std::size_t Size = sizeof(Heap<std::nullptr_t>) > BufferSize
    ? sizeof(Heap<std::nullptr_t>)
    : BufferSize;
  static constexpr std::size_t Align = alignof(impl::max_align_t);

  struct Buffer
  {
    alignas(Align) unsigned char Data[Size];
  };

  template <typename Concrete>
  using Model = cm::conditional_t<sizeof(Stack<Concrete>) <= Size &&
                                    alignof(Stack<Concrete>) <= Align,
                                  Stack<Concrete>, Heap<Concrete>>;
};

class cmSbomObject
{
public:
  template <typename T>
  struct Instance : ObjectStorage::Model<T>
  {
    using Base = ObjectStorage::Model<T>;
    using Base::Base;
    Instance(Instance&&) noexcept = default;

    ObjectInterface* Copy(void* storage) const override
    {
      return ::new (storage) Instance(this->get());
    }

    ObjectInterface* Move(void* storage) override
    {
      return ::new (storage) Instance(std::move(*this));
    }

    void SerializeImpl(cmSbomSerializer& os) const override
    {
      SerializeDispatch(this->get(), os);
    }
  };

  cmSbomObject() { ::new (Storage()) Instance<std::nullptr_t>(nullptr); }
  cmSbomObject(std::nullptr_t)
    : cmSbomObject()
  {
  }

  template <
    typename T, typename Decayed = cm::remove_cv_t<cm::remove_reference_t<T>>,
    typename = cm::enable_if_t<!std::is_same<Decayed, cmSbomObject>::value>>
  cmSbomObject(T&& x)
  {
    ::new (Storage()) Instance<Decayed>(std::forward<T>(x));
  }

  cmSbomObject(cmSbomObject const& other)
  {
    other.Interface().Copy(Storage());
  }
  cmSbomObject(cmSbomObject&& other) noexcept
  {
    other.Interface().Move(Storage());
  }

  ~cmSbomObject() noexcept { Interface().~ObjectInterface(); }

  cmSbomObject& operator=(cmSbomObject rhs)
  {
    Interface().~ObjectInterface();
    rhs.Interface().Move(Storage());
    return *this;
  }

  bool IsNull() const noexcept { return Interface().Addr() == nullptr; }
  explicit operator bool() const noexcept { return !IsNull(); }

  void Serialize(cmSbomSerializer& os) const { Interface().SerializeImpl(os); }

  template <typename T>
  T& CastUnchecked()
  {
    return *static_cast<T*>(Interface().Addr());
  }

  template <typename T>
  T const& CastUnchecked() const
  {
    return *static_cast<T const*>(Interface().Addr());
  }

  ObjectInterface& Interface()
  {
    return *static_cast<ObjectInterface*>(Storage());
  }
  ObjectInterface const& Interface() const
  {
    return *static_cast<ObjectInterface const*>(Storage());
  }

  void* Storage() { return &Data.Data; }
  void const* Storage() const { return &Data.Data; }

  template <typename U>
  U* ptr() noexcept
  {
    return std::addressof(this->template CastUnchecked<U>());
  }

  template <typename U>
  U const* ptr() const noexcept
  {
    return std::addressof(this->template CastUnchecked<U>());
  }

private:
  ObjectStorage::Buffer Data{};
};

template <typename T, typename U = cm::decay_t<T>>
U* insert_back(std::vector<cmSbomObject>& vec, T&& obj) noexcept
{
  vec.emplace_back(std::forward<U>(obj));
  return std::addressof(vec.back().CastUnchecked<U>());
}
