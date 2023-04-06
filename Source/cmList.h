/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>

#include "cmValue.h"

/**
 * CMake lists management
 *
 * For all operations, input arguments (single value like cm::string_view or
 * multiple values specified through pair of iterators) are, by default,
 * expanded. The expansion can be controlled by the cmList::ExpandElements
 * option.
 *
 * There is an exception to this rule. The following methods do not expand
 * their argument: cmList::push_back, cmList::emplace and cmList::emplace_back.
 */

class cmList
{
public:
  using container_type = std::vector<std::string>;

  using value_type = container_type::value_type;
  using allocator_type = container_type::allocator_type;
  using index_type = int;
  using size_type = container_type::size_type;
  using difference_type = container_type::difference_type;
  using reference = container_type::reference;
  using const_reference = container_type::const_reference;
  using iterator = container_type::iterator;
  using const_iterator = container_type::const_iterator;
  using reverse_iterator = container_type::reverse_iterator;
  using const_reverse_iterator = container_type::const_reverse_iterator;

  static const size_type npos = static_cast<size_type>(-1);

  static cm::string_view element_separator;

  enum class EmptyElements
  {
    No,
    Yes,
  };
  enum class ExpandElements
  {
    No,
    Yes,
  };

  cmList() = default;
  cmList(const cmList&) = default;
  cmList(cmList&&) = default;

  cmList(cm::string_view value,
         ExpandElements expandElements = ExpandElements::Yes,
         EmptyElements emptyElements = EmptyElements::No)
  {
    this->assign(value, expandElements, emptyElements);
  }
  cmList(cm::string_view value, EmptyElements emptyElements)
  {
    this->assign(value, ExpandElements::Yes, emptyElements);
  }
  cmList(cmValue list, ExpandElements expandElements = ExpandElements::Yes,
         EmptyElements emptyElements = EmptyElements::No)
  {
    if (list) {
      this->assign(*list, expandElements, emptyElements);
    }
  }
  cmList(cmValue list, EmptyElements emptyElements)
    : cmList(list, ExpandElements::Yes, emptyElements)
  {
  }
  template <typename InputIterator>
  cmList(InputIterator first, InputIterator last,
         ExpandElements expandElements = ExpandElements::Yes,
         EmptyElements emptyElements = EmptyElements::No)
  {
    this->assign(first, last, expandElements, emptyElements);
  }
  template <typename InputIterator>
  cmList(InputIterator first, InputIterator last, EmptyElements emptyElements)
    : cmList(first, last, ExpandElements::Yes, emptyElements)
  {
  }
  cmList(const container_type& init,
         ExpandElements expandElements = ExpandElements::Yes,
         EmptyElements emptyElements = EmptyElements::No)
    : cmList(init.begin(), init.end(), expandElements, emptyElements)
  {
  }
  cmList(const container_type& init, EmptyElements emptyElements)
    : cmList(init, ExpandElements::Yes, emptyElements)
  {
  }
  cmList(container_type&& init,
         ExpandElements expandElements = ExpandElements::Yes,
         EmptyElements emptyElements = EmptyElements::No)
    : cmList(std::make_move_iterator(init.begin()),
             std::make_move_iterator(init.end()), expandElements,
             emptyElements)
  {
    init.clear();
  }
  cmList(container_type&& init, EmptyElements emptyElements)
    : cmList(std::move(init), ExpandElements::Yes, emptyElements)
  {
  }
  cmList(std::initializer_list<std::string> init) { this->assign(init); }

  ~cmList() = default;

  cmList& operator=(const cmList&) = default;
  cmList& operator=(cmList&&) = default;

  cmList& operator=(cm::string_view value)
  {
    this->assign(value);
    return *this;
  }
  cmList& operator=(cmValue value)
  {
    if (value) {
      this->operator=(*value);
    } else {
      this->clear();
    }

    return *this;
  }

  cmList& operator=(const container_type& init)
  {
    this->assign(init);
    return *this;
  }
  cmList& operator=(container_type&& init)
  {
    this->assign(std::move(init));

    return *this;
  }

  cmList& operator=(std::initializer_list<std::string> init)
  {
    this->assign(init);
    return *this;
  }

  void assign(cm::string_view value,
              ExpandElements expandElements = ExpandElements::Yes,
              EmptyElements emptyElements = EmptyElements::No)
  {
    this->clear();
    this->append(value, expandElements, emptyElements);
  }
  void assign(cm::string_view value, EmptyElements emptyElements)
  {
    this->assign(value, ExpandElements::Yes, emptyElements);
  }
  void assign(cmValue value,
              ExpandElements expandElements = ExpandElements::Yes,
              EmptyElements emptyElements = EmptyElements::No)
  {
    if (value) {
      this->assign(*value, expandElements, emptyElements);
    } else {
      this->clear();
    }
  }
  void assign(cmValue value, EmptyElements emptyElements)
  {
    this->assign(value, ExpandElements::Yes, emptyElements);
  }
  template <typename InputIterator>
  void assign(InputIterator first, InputIterator last,
              ExpandElements expandElements = ExpandElements::Yes,
              EmptyElements emptyElements = EmptyElements::No)
  {
    this->clear();
    this->append(first, last, expandElements, emptyElements);
  }
  template <typename InputIterator>
  void assign(InputIterator first, InputIterator last,
              EmptyElements emptyElements)
  {
    this->assign(first, last, ExpandElements::Yes, emptyElements);
  }
  void assign(const cmList& init,
              ExpandElements expandElements = ExpandElements::Yes,
              EmptyElements emptyElements = EmptyElements::No)
  {
    this->assign(init.begin(), init.end(), expandElements, emptyElements);
  }
  void assign(const cmList& init, EmptyElements emptyElements)
  {
    this->assign(init, ExpandElements::Yes, emptyElements);
  }
  void assign(cmList&& init,
              ExpandElements expandElements = ExpandElements::Yes,
              EmptyElements emptyElements = EmptyElements::No)
  {
    this->assign(std::make_move_iterator(init.begin()),
                 std::make_move_iterator(init.end()), expandElements,
                 emptyElements);
    init.clear();
  }
  void assign(cmList&& init, EmptyElements emptyElements)
  {
    this->assign(std::move(init), ExpandElements::Yes, emptyElements);
  }
  void assign(const container_type& init,
              ExpandElements expandElements = ExpandElements::Yes,
              EmptyElements emptyElements = EmptyElements::No)
  {
    this->assign(init.begin(), init.end(), expandElements, emptyElements);
  }
  void assign(const container_type& init, EmptyElements emptyElements)
  {
    this->assign(init, ExpandElements::Yes, emptyElements);
  }
  void assign(container_type&& init,
              ExpandElements expandElements = ExpandElements::Yes,
              EmptyElements emptyElements = EmptyElements::No)
  {
    this->assign(std::make_move_iterator(init.begin()),
                 std::make_move_iterator(init.end()), expandElements,
                 emptyElements);
    init.clear();
  }
  void assign(container_type&& init, EmptyElements emptyElements)
  {
    this->assign(std::move(init), ExpandElements::Yes, emptyElements);
  }
  void assign(std::initializer_list<std::string> init)
  {
    this->assign(init.begin(), init.end());
  }

  // Conversions
  std::string to_string() const
  {
    return this->join(cmList::element_separator);
  }

  operator container_type&() & noexcept { return this->Values; }
  operator const container_type&() const& noexcept { return this->Values; }
  operator container_type&&() && noexcept { return std::move(this->Values); }

  // Element access
  reference at(index_type pos)
  {
    return this->Values.at(this->ComputeIndex(pos));
  }
  const_reference at(index_type pos) const
  {
    return this->Values.at(this->ComputeIndex(pos));
  }

  reference operator[](index_type pos)
  {
    return this->Values[this->ComputeIndex(pos, false)];
  }
  const_reference operator[](index_type pos) const
  {
    return this->Values[this->ComputeIndex(pos, false)];
  }

  reference front() { return this->Values.front(); }
  const_reference front() const { return this->Values.front(); }

  reference back() { return this->Values.back(); }
  const_reference back() const { return this->Values.back(); }

  // extract sublist in range [first, last)
  cmList sublist(const_iterator first, const_iterator last) const
  {
    return cmList{ first, last };
  }
  // Extract sublist in range [first, last)
  // Throw std::out_of_range if pos is invalid
  cmList sublist(size_type pos = 0, size_type length = npos) const;

  // Returns the list of elements
  // Throw std::out_of_range if any index is invalid
  cmList get_items(std::initializer_list<index_type> indexes) const
  {
    return this->GetItems(
      std::vector<index_type>{ indexes.begin(), indexes.end() });
  }
  template <typename InputIterator>
  cmList get_items(InputIterator first, InputIterator last) const
  {
    return this->GetItems(std::vector<index_type>{ first, last });
  }

  size_type find(cm::string_view value) const;
  size_type find(cmValue value) const
  {
    if (value) {
      return this->find(*value);
    }

    return npos;
  }

  container_type& data() noexcept { return this->Values; }
  const container_type& data() const noexcept { return this->Values; }

  // Iterators
  iterator begin() noexcept { return this->Values.begin(); }
  const_iterator begin() const noexcept { return this->Values.begin(); }
  const_iterator cbegin() const noexcept { return this->Values.cbegin(); }

  iterator end() noexcept { return this->Values.end(); }
  const_iterator end() const noexcept { return this->Values.end(); }
  const_iterator cend() const noexcept { return this->Values.cend(); }

  reverse_iterator rbegin() noexcept { return this->Values.rbegin(); }
  const_reverse_iterator rbegin() const noexcept
  {
    return this->Values.rbegin();
  }
  const_reverse_iterator crbegin() const noexcept
  {
    return this->Values.crbegin();
  }

  reverse_iterator rend() noexcept { return this->Values.rend(); }
  const_reverse_iterator rend() const noexcept { return this->Values.rend(); }
  const_reverse_iterator crend() const noexcept
  {
    return this->Values.crend();
  }

  // Capacity
  bool empty() const noexcept { return this->Values.empty(); }
  size_type size() const noexcept { return this->Values.size(); }

  // Modifiers
  void clear() noexcept { this->Values.clear(); }

  iterator insert(const_iterator pos, cm::string_view value,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    return cmList::Insert(pos, std::string(value), this->Values,
                          expandElements, emptyElements);
  }
  iterator insert(const_iterator pos, cm::string_view value,
                  EmptyElements emptyElements)
  {
    return this->insert(pos, value, ExpandElements::Yes, emptyElements);
  }
  iterator insert(const_iterator pos, cmValue value,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    if (value) {
      return this->insert(pos, *value, expandElements, emptyElements);
    }

    auto delta = std::distance(this->cbegin(), pos);
    return this->begin() + delta;
  }
  iterator insert(const_iterator pos, cmValue value,
                  EmptyElements emptyElements)
  {
    return this->insert(pos, value, ExpandElements::Yes, emptyElements);
  }
  template <typename InputIterator>
  iterator insert(const_iterator pos, InputIterator first, InputIterator last,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    return cmList::Insert(pos, first, last, this->Values, expandElements,
                          emptyElements);
  }
  template <typename InputIterator>
  iterator insert(const_iterator pos, InputIterator first, InputIterator last,
                  EmptyElements emptyElements)
  {
    return this->insert(pos, first, last, ExpandElements::Yes, emptyElements);
  }
  iterator insert(const_iterator pos, const cmList& values,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    return this->insert(pos, values.begin(), values.end(), expandElements,
                        emptyElements);
  }
  iterator insert(const_iterator pos, const cmList& values,
                  EmptyElements emptyElements)
  {
    return this->insert(pos, values, ExpandElements::Yes, emptyElements);
  }
  iterator insert(const_iterator pos, cmList&& values,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    auto result = this->insert(pos, std::make_move_iterator(values.begin()),
                               std::make_move_iterator(values.end()),
                               expandElements, emptyElements);
    values.clear();

    return result;
  }
  iterator insert(const_iterator pos, cmList&& values,
                  EmptyElements emptyElements)
  {
    return this->insert(pos, std::move(values), ExpandElements::Yes,
                        emptyElements);
  }
  iterator insert(const_iterator pos, const container_type& values,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    return this->insert(pos, values.begin(), values.end(), expandElements,
                        emptyElements);
  }
  iterator insert(const_iterator pos, const container_type& values,
                  EmptyElements emptyElements)
  {
    return this->insert(pos, values, ExpandElements::Yes, emptyElements);
  }
  iterator insert(const_iterator pos, container_type&& values,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    auto result = this->insert(pos, std::make_move_iterator(values.begin()),
                               std::make_move_iterator(values.end()),
                               expandElements, emptyElements);
    values.clear();

    return result;
  }
  iterator insert(const_iterator pos, container_type&& values,
                  EmptyElements emptyElements)
  {
    return this->insert(pos, std::move(values), ExpandElements::Yes,
                        emptyElements);
  }
  iterator insert(const_iterator pos, std::initializer_list<std::string> ilist)
  {
    return this->insert(pos, ilist.begin(), ilist.end());
  }

  iterator append(cm::string_view value,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    return this->insert(this->cend(), value, expandElements, emptyElements);
  }
  iterator append(cm::string_view value, EmptyElements emptyElements)
  {
    return this->append(value, ExpandElements::Yes, emptyElements);
  }
  iterator append(cmValue value,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    if (value) {
      return this->append(*value, expandElements, emptyElements);
    }

    return this->end();
  }
  iterator append(cmValue value, EmptyElements emptyElements)
  {
    return this->append(value, ExpandElements::Yes, emptyElements);
  }
  template <typename InputIterator>
  iterator append(InputIterator first, InputIterator last,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    return this->insert(this->cend(), first, last, expandElements,
                        emptyElements);
  }
  template <typename InputIterator>
  iterator append(InputIterator first, InputIterator last,
                  EmptyElements emptyElements)
  {
    return this->append(first, last, ExpandElements::Yes, emptyElements);
  }
  iterator append(const cmList& values,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    return this->append(values.begin(), values.end(), expandElements,
                        emptyElements);
  }
  iterator append(const cmList& values, EmptyElements emptyElements)
  {
    return this->append(values, ExpandElements::Yes, emptyElements);
  }
  iterator append(cmList&& values,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    auto result = this->append(std::make_move_iterator(values.begin()),
                               std::make_move_iterator(values.end()),
                               expandElements, emptyElements);
    values.clear();

    return result;
  }
  iterator append(cmList&& values, EmptyElements emptyElements)
  {
    return this->append(std::move(values), ExpandElements::Yes, emptyElements);
  }
  iterator append(const container_type& values,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    return this->append(values.begin(), values.end(), expandElements,
                        emptyElements);
  }
  iterator append(const container_type& values, EmptyElements emptyElements)
  {
    return this->append(values, ExpandElements::Yes, emptyElements);
  }
  iterator append(container_type&& values,
                  ExpandElements expandElements = ExpandElements::Yes,
                  EmptyElements emptyElements = EmptyElements::No)
  {
    auto result = this->append(std::make_move_iterator(values.begin()),
                               std::make_move_iterator(values.end()),
                               expandElements, emptyElements);
    values.clear();

    return result;
  }
  iterator append(container_type&& values, EmptyElements emptyElements)
  {
    return this->append(std::move(values), ExpandElements::Yes, emptyElements);
  }
  iterator append(std::initializer_list<std::string> ilist)
  {
    return this->insert(this->cend(), ilist);
  }

  iterator prepend(cm::string_view value,
                   ExpandElements expandElements = ExpandElements::Yes,
                   EmptyElements emptyElements = EmptyElements::No)
  {
    return this->insert(this->cbegin(), value, expandElements, emptyElements);
  }
  iterator prepend(cm::string_view value, EmptyElements emptyElements)
  {
    return this->prepend(value, ExpandElements::Yes, emptyElements);
  }
  iterator prepend(cmValue value,
                   ExpandElements expandElements = ExpandElements::Yes,
                   EmptyElements emptyElements = EmptyElements::No)
  {
    if (value) {
      return this->prepend(*value, expandElements, emptyElements);
    }

    return this->begin();
  }
  iterator prepend(cmValue value, EmptyElements emptyElements)
  {
    return this->prepend(value, ExpandElements::Yes, emptyElements);
  }
  template <typename InputIterator>
  iterator prepend(InputIterator first, InputIterator last,
                   ExpandElements expandElements = ExpandElements::Yes,
                   EmptyElements emptyElements = EmptyElements::No)
  {
    return this->insert(this->cbegin(), first, last, expandElements,
                        emptyElements);
  }
  template <typename InputIterator>
  iterator prepend(InputIterator first, InputIterator last,
                   EmptyElements emptyElements)
  {
    return this->prepend(first, last, ExpandElements::Yes, emptyElements);
  }
  iterator prepend(const cmList& values,
                   ExpandElements expandElements = ExpandElements::Yes,
                   EmptyElements emptyElements = EmptyElements::No)
  {
    return this->prepend(values.begin(), values.end(), expandElements,
                         emptyElements);
  }
  iterator prepend(const cmList& values, EmptyElements emptyElements)
  {
    return this->prepend(values, ExpandElements::Yes, emptyElements);
  }
  iterator prepend(cmList&& values,
                   ExpandElements expandElements = ExpandElements::Yes,
                   EmptyElements emptyElements = EmptyElements::No)
  {
    auto result = this->prepend(std::make_move_iterator(values.begin()),
                                std::make_move_iterator(values.end()),
                                expandElements, emptyElements);
    values.clear();

    return result;
  }
  iterator prepend(cmList&& values, EmptyElements emptyElements)
  {
    return this->prepend(std::move(values), ExpandElements::Yes,
                         emptyElements);
  }
  iterator prepend(const container_type& values,
                   ExpandElements expandElements = ExpandElements::Yes,
                   EmptyElements emptyElements = EmptyElements::No)
  {
    return this->prepend(values.begin(), values.end(), expandElements,
                         emptyElements);
  }
  iterator prepend(const container_type& values, EmptyElements emptyElements)
  {
    return this->prepend(values, ExpandElements::Yes, emptyElements);
  }
  iterator prepend(container_type&& values,
                   ExpandElements expandElements = ExpandElements::Yes,
                   EmptyElements emptyElements = EmptyElements::No)
  {
    auto result = this->prepend(std::make_move_iterator(values.begin()),
                                std::make_move_iterator(values.end()),
                                expandElements, emptyElements);
    values.clear();

    return result;
  }
  iterator prepend(container_type&& values, EmptyElements emptyElements)
  {
    return this->prepend(std::move(values), ExpandElements::Yes,
                         emptyElements);
  }
  iterator prepend(std::initializer_list<std::string> ilist)
  {
    return this->insert(this->cbegin(), ilist);
  }

  void push_back(cm::string_view value)
  {
    this->Values.push_back(std::string{ value });
  }
  void push_back(std::string&& value)
  {
    this->Values.push_back(std::move(value));
  }

  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args)
  {
    return this->Values.emplace(pos, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void emplace_back(Args&&... args)
  {
    this->Values.emplace_back(std::forward<Args>(args)...);
  }

  // Inserts elements in the list
  // Throw std::out_of_range if index is invalid
  template <typename InputIterator>
  cmList& insert_items(index_type index, InputIterator first,
                       InputIterator last)
  {
    this->insert(this->begin() + this->ComputeInsertIndex(index), first, last);
    return *this;
  }
  cmList& insert_items(index_type index,
                       std::initializer_list<std::string> values)
  {
    return this->insert_items(index, values.begin(), values.end());
  }

  iterator erase(const_iterator pos)
  {
    // convert const_iterator in iterator to please non standard c++11
    // compilers (gcc 4.8 for example)
    auto pos2 =
      this->Values.begin() + std::distance(this->Values.cbegin(), pos);
    return this->Values.erase(pos2);
  }
  iterator erase(const_iterator first, const_iterator last)
  {
    // convert const_iterator in iterator to please non standard c++11
    // compilers (gcc 4.8 for example)
    auto first2 =
      this->Values.begin() + std::distance(this->Values.cbegin(), first);
    auto last2 =
      this->Values.begin() + std::distance(this->Values.cbegin(), last);
    return this->Values.erase(first2, last2);
  }

  void pop_back() { this->Values.pop_back(); }
  void pop_front() { this->Values.erase(this->begin()); }

  // Removes elements from the list
  // Throw std::out_of_range if any index is invalid
  cmList& remove_items(std::initializer_list<index_type> indexes)
  {
    return this->RemoveItems(
      std::vector<index_type>{ indexes.begin(), indexes.end() });
  }
  cmList& remove_items(std::initializer_list<std::string> values)
  {
    return this->RemoveItems(
      std::vector<std::string>{ values.begin(), values.end() });
  }
  template <typename InputIterator>
  cmList& remove_items(InputIterator first, InputIterator last)
  {
    return this->RemoveItems(
      std::vector<typename InputIterator::value_type>{ first, last });
  }

  cmList& remove_duplicates();

  enum class FilterMode
  {
    INCLUDE,
    EXCLUDE
  };
  // Includes or removes items from the list
  // Throw std::invalid_argument if regular expression is invalid
  cmList& filter(cm::string_view regex, FilterMode mode);

  cmList& reverse()
  {
    std::reverse(this->Values.begin(), this->Values.end());
    return *this;
  }

  struct SortConfiguration
  {
    enum class OrderMode
    {
      DEFAULT,
      ASCENDING,
      DESCENDING,
    } Order = OrderMode::DEFAULT;

    enum class CompareMethod
    {
      DEFAULT,
      STRING,
      FILE_BASENAME,
      NATURAL,
    } Compare = CompareMethod::DEFAULT;

    enum class CaseSensitivity
    {
      DEFAULT,
      SENSITIVE,
      INSENSITIVE,
    } Case = CaseSensitivity::DEFAULT;

    // declare the default constructor to work-around clang bug
    SortConfiguration();

    SortConfiguration(OrderMode order, CompareMethod compare,
                      CaseSensitivity caseSensitivity)
      : Order(order)
      , Compare(compare)
      , Case(caseSensitivity)
    {
    }
  };
  cmList& sort(const SortConfiguration& config = SortConfiguration{});

  // exception raised on error during transform operations
  class transform_error : public std::runtime_error
  {
  public:
    transform_error(const std::string& error)
      : std::runtime_error(error)
    {
    }
  };

  class TransformSelector
  {
  public:
    using index_type = cmList::index_type;

    // define some structs used as template selector
    struct AT;
    struct FOR;
    struct REGEX;

    virtual ~TransformSelector() = default;

    virtual const std::string& GetTag() = 0;

    // method NEW is used to allocate a selector of the needed type.
    // For example:
    // cmList::TransformSelector::New<AT>({1, 2, 5, 6});
    //  or
    // cmList::TransformSelector::New<REGEX>("^XX.*");
    template <typename Type>
    static std::unique_ptr<TransformSelector> New(
      std::initializer_list<index_type>);
    template <typename Type>
    static std::unique_ptr<TransformSelector> New(
      std::vector<index_type> const&);
    template <typename Type>
    static std::unique_ptr<TransformSelector> New(std::vector<index_type>&&);

    template <typename Type>
    static std::unique_ptr<TransformSelector> New(std::string const&);
    template <typename Type>
    static std::unique_ptr<TransformSelector> New(std::string&&);

  private:
    static std::unique_ptr<TransformSelector> NewAT(
      std::initializer_list<index_type> init);
    static std::unique_ptr<TransformSelector> NewAT(
      std::vector<index_type> const& init);
    static std::unique_ptr<TransformSelector> NewAT(
      std::vector<index_type>&& init);

    static std::unique_ptr<TransformSelector> NewFOR(
      std::initializer_list<index_type> init);
    static std::unique_ptr<TransformSelector> NewFOR(
      std::vector<index_type> const& init);
    static std::unique_ptr<TransformSelector> NewFOR(
      std::vector<index_type>&& init);

    static std::unique_ptr<TransformSelector> NewREGEX(
      std::string const& init);
    static std::unique_ptr<TransformSelector> NewREGEX(std::string&& init);
  };

  enum class TransformAction
  {
    APPEND,
    PREPEND,
    TOLOWER,
    TOUPPER,
    STRIP,
    GENEX_STRIP,
    REPLACE
  };

  // Transforms the list by applying an action
  // Throw std::transform_error is any of arguments specified are invalid
  cmList& transform(TransformAction action,
                    std::unique_ptr<TransformSelector> = {});
  cmList& transform(TransformAction action, std::string const& arg,
                    std::unique_ptr<TransformSelector> = {});
  cmList& transform(TransformAction action, std::string const& arg1,
                    std::string const& arg2,
                    std::unique_ptr<TransformSelector> = {});
  cmList& transform(TransformAction action,
                    std::vector<std::string> const& args,
                    std::unique_ptr<TransformSelector> = {});

  std::string join(cm::string_view glue) const;

  void swap(cmList& other) noexcept { this->Values.swap(other.Values); }

  // static members
  // ==============
  // these methods can be used to store CMake list expansion directly in a
  // std::vector.
  static void assign(cm::string_view value,
                     std::vector<std::string>& container,
                     EmptyElements emptyElements = EmptyElements::No)
  {
    container.clear();
    cmList::append(value, container, emptyElements);
  }
  static void assign(cmValue value, std::vector<std::string>& container,
                     EmptyElements emptyElements = EmptyElements::No)
  {
    if (value) {
      cmList::assign(*value, container, emptyElements);
    } else {
      container.clear();
    }
  }
  template <typename InputIterator>
  static void assign(InputIterator first, InputIterator last,
                     std::vector<std::string>& container,
                     EmptyElements emptyElements = EmptyElements::No)
  {
    container.clear();
    cmList::append(first, last, container, emptyElements);
  }

  static std::vector<std::string>::iterator insert(
    std::vector<std::string>::const_iterator pos, cm::string_view value,
    std::vector<std::string>& container,
    EmptyElements emptyElements = EmptyElements::No)
  {
    return cmList::Insert(pos, std::string(value), container,
                          ExpandElements::Yes, emptyElements);
  }
  static std::vector<std::string>::iterator insert(
    std::vector<std::string>::const_iterator pos, cmValue value,
    std::vector<std::string>& container,
    EmptyElements emptyElements = EmptyElements::No)
  {
    if (value) {
      return cmList::insert(pos, *value, container, emptyElements);
    }

    auto delta = std::distance(container.cbegin(), pos);
    return container.begin() + delta;
  }
  template <typename InputIterator>
  static std::vector<std::string>::iterator insert(
    std::vector<std::string>::const_iterator pos, InputIterator first,
    InputIterator last, std::vector<std::string>& container,
    EmptyElements emptyElements = EmptyElements::No)
  {
    return cmList::Insert(pos, first, last, container, ExpandElements::Yes,
                          emptyElements);
  }

  static std::vector<std::string>::iterator append(
    cm::string_view value, std::vector<std::string>& container,
    EmptyElements emptyElements = EmptyElements::No)
  {
    return cmList::insert(container.cend(), value, container, emptyElements);
  }
  static std::vector<std::string>::iterator append(
    cmValue value, std::vector<std::string>& container,
    EmptyElements emptyElements = EmptyElements::No)
  {
    if (value) {
      return cmList::append(*value, container, emptyElements);
    }

    return container.end();
  }
  template <typename InputIterator>
  static std::vector<std::string>::iterator append(
    InputIterator first, InputIterator last,
    std::vector<std::string>& container,
    EmptyElements emptyElements = EmptyElements::No)
  {
    return cmList::insert(container.cend(), first, last, container,
                          emptyElements);
  }

  static std::vector<std::string>::iterator prepend(
    cm::string_view value, std::vector<std::string>& container,
    EmptyElements emptyElements = EmptyElements::No)
  {
    return cmList::insert(container.cbegin(), value, container, emptyElements);
  }
  static std::vector<std::string>::iterator prepend(
    cmValue value, std::vector<std::string>& container,
    EmptyElements emptyElements = EmptyElements::No)
  {
    if (value) {
      return cmList::prepend(*value, container, emptyElements);
    }

    return container.begin();
  }
  template <typename InputIterator>
  static std::vector<std::string>::iterator prepend(
    InputIterator first, InputIterator last,
    std::vector<std::string>& container,
    EmptyElements emptyElements = EmptyElements::No)
  {
    return cmList::insert(container.cbegin(), first, last, container,
                          emptyElements);
  }

  // The following methods offer the possibility to extend a CMake list
  // but without any intermediate expansion. So the operation is simply a
  // string concatenation with special handling for the CMake list item
  // separator
  static std::string& append(cm::string_view value, std::string& list);
  template <typename InputIterator>
  static std::string& append(InputIterator first, InputIterator last,
                             std::string& list)
  {
    if (first == last) {
      return list;
    }

    return cmList::append(cm::string_view{ std::accumulate(
                            std::next(first), last, *first,
                            [](std::string a, const std::string& b) {
                              return std::move(a) +
                                std::string(cmList::element_separator) + b;
                            }) },
                          list);
  }

  static std::string& prepend(cm::string_view value, std::string& list);
  template <typename InputIterator>
  static std::string& prepend(InputIterator first, InputIterator last,
                              std::string& list)
  {
    if (first == last) {
      return list;
    }

    return cmList::prepend(cm::string_view{ std::accumulate(
                             std::next(first), last, *first,
                             [](std::string a, const std::string& b) {
                               return std::move(a) +
                                 std::string(cmList::element_separator) + b;
                             }) },
                           list);
  }

  // Non-members
  // ===========
  friend inline bool operator==(const cmList& lhs, const cmList& rhs) noexcept
  {
    return lhs.Values == rhs.Values;
  }
  friend inline bool operator!=(const cmList& lhs, const cmList& rhs) noexcept
  {
    return lhs.Values != rhs.Values;
  }

private:
  size_type ComputeIndex(index_type pos, bool boundCheck = true) const;
  size_type ComputeInsertIndex(index_type pos, bool boundCheck = true) const;

  cmList GetItems(std::vector<index_type>&& indexes) const;

  cmList& RemoveItems(std::vector<index_type>&& indexes);
  cmList& RemoveItems(std::vector<std::string>&& items);

  static container_type::iterator Insert(container_type::const_iterator pos,
                                         std::string&& value,
                                         container_type& container,
                                         ExpandElements expandElements,
                                         EmptyElements emptyElements);
  static container_type::iterator Insert(container_type::const_iterator pos,
                                         const std::string& value,
                                         container_type& container,
                                         ExpandElements expandElements,
                                         EmptyElements emptyElements)
  {
    auto tmp = value;
    return cmList::Insert(pos, std::move(tmp), container, expandElements,
                          emptyElements);
  }
  template <typename InputIterator>
  static container_type::iterator Insert(container_type::const_iterator pos,
                                         InputIterator first,
                                         InputIterator last,
                                         container_type& container,
                                         ExpandElements expandElements,
                                         EmptyElements emptyElements)
  {
    auto delta = std::distance(container.cbegin(), pos);

    if (first == last) {
      return container.begin() + delta;
    }

    auto insertPos = container.begin() + delta;
    if (expandElements == ExpandElements::Yes) {
      for (; first != last; ++first) {
        auto size = container.size();
        insertPos = cmList::Insert(insertPos, *first, container,
                                   expandElements, emptyElements);
        insertPos += container.size() - size;
      }
    } else {
      for (; first != last; ++first) {
        if (!first->empty() || emptyElements == EmptyElements::Yes) {
          insertPos = container.insert(insertPos, *first);
          insertPos++;
        }
      }
    }

    return container.begin() + delta;
  }

  container_type Values;
};

// specializations for cmList::TransformSelector allocators
// ========================================================
template <>
inline std::unique_ptr<cmList::TransformSelector>
cmList::TransformSelector::New<cmList::TransformSelector::AT>(
  std::initializer_list<index_type> init)
{
  return cmList::TransformSelector::NewAT(init);
}
template <>
inline std::unique_ptr<cmList::TransformSelector>
cmList::TransformSelector::New<cmList::TransformSelector::AT>(
  std::vector<index_type> const& init)
{
  return cmList::TransformSelector::NewAT(init);
}
template <>
inline std::unique_ptr<cmList::TransformSelector>
cmList::TransformSelector::New<cmList::TransformSelector::AT>(
  std::vector<index_type>&& init)
{
  return cmList::TransformSelector::NewAT(std::move(init));
}

template <>
inline std::unique_ptr<cmList::TransformSelector>
cmList::TransformSelector::New<cmList::TransformSelector::FOR>(
  std::initializer_list<index_type> init)
{
  return cmList::TransformSelector::NewFOR(init);
}
template <>
inline std::unique_ptr<cmList::TransformSelector>
cmList::TransformSelector::New<cmList::TransformSelector::FOR>(
  std::vector<index_type> const& init)
{
  return cmList::TransformSelector::NewFOR(init);
}
template <>
inline std::unique_ptr<cmList::TransformSelector>
cmList::TransformSelector::New<cmList::TransformSelector::FOR>(
  std::vector<index_type>&& init)
{
  return cmList::TransformSelector::NewFOR(std::move(init));
}

template <>
inline std::unique_ptr<cmList::TransformSelector>
cmList::TransformSelector::New<cmList::TransformSelector::REGEX>(
  std::string const& init)
{
  return cmList::TransformSelector::NewREGEX(init);
}
template <>
inline std::unique_ptr<cmList::TransformSelector>
cmList::TransformSelector::New<cmList::TransformSelector::REGEX>(
  std::string&& init)
{
  return cmList::TransformSelector::NewREGEX(std::move(init));
}

// Non-member functions
// ====================
inline std::vector<std::string>& operator+=(std::vector<std::string>& l,
                                            const cmList& r)
{
  l.insert(l.end(), r.begin(), r.end());
  return l;
}
inline std::vector<std::string>& operator+=(std::vector<std::string>& l,
                                            cmList&& r)
{
  std::move(r.begin(), r.end(), std::back_inserter(l));
  r.clear();

  return l;
}

namespace cm {
inline void erase(cmList& list, const std::string& value)
{
  list.erase(std::remove(list.begin(), list.end(), value), list.end());
}

template <typename Predicate>
inline void erase_if(cmList& list, Predicate pred)
{
  list.erase(std::remove_if(list.begin(), list.end(), pred), list.end());
}
}

namespace srd {
inline void swap(cmList& lhs, cmList& rhs) noexcept
{
  lhs.swap(rhs);
}
}
