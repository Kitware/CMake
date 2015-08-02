/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLinkedTree_h
#define cmLinkedTree_h

#include "cmStandardIncludes.h"

#include <assert.h>

/**
  @brief A adaptor for traversing a tree structure in a vector

  This class is not intended to be wholly generic like a standard library
  container adaptor.  Mostly it exists to facilitate code sharing for the
  needs of the cmState.  For example, the Truncate() method is a specific
  requirement of the cmState.

  An empty cmLinkedTree provides a Root() method, and an Extend() method,
  each of which return iterators.  A Tree can be built up by extending
  from the root, and then extending from any other iterator.

  An iterator resulting from this tree construction can be
  forward-only-iterated toward the root.  Extending the tree never
  invalidates existing iterators.
 */
template<typename T>
class cmLinkedTree
{
  typedef typename std::vector<T>::size_type PositionType;
  typedef T* PointerType;
  typedef T& ReferenceType;
public:
  class iterator : public std::iterator<std::forward_iterator_tag, T>
  {
    friend class cmLinkedTree;
    cmLinkedTree* Tree;

    // The Position is always 'one past the end'.
    PositionType Position;

    iterator(cmLinkedTree* tree, PositionType pos)
      : Tree(tree), Position(pos)
    {

    }

  public:
    iterator()
      : Tree(0), Position(0)
    {

    }

    void operator++()
    {
      assert(this->Tree);
      assert(this->Tree->UpPositions.size() == this->Tree->Data.size());
      assert(this->Position <= this->Tree->Data.size());
      assert(this->Position > 0);
      this->Position = this->Tree->UpPositions[this->Position - 1];
    }

    PointerType operator->() const
    {
      assert(this->Tree);
      assert(this->Tree->UpPositions.size() == this->Tree->Data.size());
      assert(this->Position <= this->Tree->Data.size());
      assert(this->Position > 0);
      return this->Tree->GetPointer(this->Position - 1);
    }

    PointerType operator->()
    {
      assert(this->Tree);
      assert(this->Tree->UpPositions.size() == this->Tree->Data.size());
      assert(this->Position <= this->Tree->Data.size());
      assert(this->Position > 0);
      return this->Tree->GetPointer(this->Position - 1);
    }

    ReferenceType operator*() const
    {
      assert(this->Tree);
      assert(this->Tree->UpPositions.size() == this->Tree->Data.size());
      assert(this->Position <= this->Tree->Data.size());
      assert(this->Position > 0);
      return this->Tree->GetReference(this->Position - 1);
    }

    ReferenceType operator*()
    {
      assert(this->Tree);
      assert(this->Tree->UpPositions.size() == this->Tree->Data.size());
      assert(this->Position <= this->Tree->Data.size());
      assert(this->Position > 0);
      return this->Tree->GetReference(this->Position - 1);
    }

    bool operator==(iterator other) const
    {
      assert(this->Tree);
      assert(this->Tree->UpPositions.size() == this->Tree->Data.size());
      assert(this->Tree == other.Tree);
      return this->Position == other.Position;
    }

    bool operator!=(iterator other) const
    {
      assert(this->Tree);
      assert(this->Tree->UpPositions.size() == this->Tree->Data.size());
      return !(*this == other);
    }

    bool IsValid() const
    {
      if (!this->Tree)
        {
        return false;
        }
      return this->Position <= this->Tree->Data.size();
    }

    bool StrictWeakOrdered(iterator other) const
    {
      assert(this->Tree);
      assert(this->Tree == other.Tree);
      return this->Position < other.Position;
    }
  };

  iterator Root() const
  {
    return iterator(const_cast<cmLinkedTree*>(this), 0);
  }

  iterator Extend(iterator it)
  {
    return Extend_impl(it, T());
  }

  iterator Extend(iterator it, T t)
  {
    return Extend_impl(it, t);
  }

  iterator Truncate()
  {
    assert(this->UpPositions.size() > 0);
    this->UpPositions.erase(this->UpPositions.begin() + 1,
                            this->UpPositions.end());
    assert(this->Data.size() > 0);
    this->Data.erase(this->Data.begin() + 1, this->Data.end());
    return iterator(this, 1);
  }

  void Clear()
  {
    this->UpPositions.clear();
    this->Data.clear();
  }

private:
  T& GetReference(PositionType pos)
  {
    return this->Data[pos];
  }

  T* GetPointer(PositionType pos)
  {
    return &this->Data[pos];
  }

  iterator Extend_impl(iterator it, T t)
  {
    assert(this->UpPositions.size() == this->Data.size());
    assert(it.Position <= this->UpPositions.size());
    this->UpPositions.push_back(it.Position);
    this->Data.push_back(t);
    return iterator(this, this->UpPositions.size());
  }

  std::vector<T> Data;
  std::vector<PositionType> UpPositions;
};

#endif
