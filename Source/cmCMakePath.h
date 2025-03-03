/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <string>
#include <utility>

#include <cm/filesystem>
#include <cm/string_view>
#include <cm/type_traits>

namespace cm {
class static_string_view;
}

namespace detail {
#if defined(__SUNPRO_CC) && defined(__sparc)
// Oracle DeveloperStudio C++ compiler on Solaris/Sparc fails to compile
// the full 'is_pathable' and 'is_move_pathable' checks.  We use it only to
// improve error messages via 'enable_if' when calling methods with incorrect
// types. Just pretend all types are allowed so we can at least compile valid
// code.
template <typename T>
struct is_pathable : std::true_type
{
};

template <typename T>
struct is_move_pathable : std::true_type
{
};

#else
template <typename T, typename = void>
struct is_pathable : std::false_type
{
};

template <>
struct is_pathable<cm::filesystem::path> : std::true_type
{
};
template <>
struct is_pathable<std::string> : std::true_type
{
};
template <>
struct is_pathable<cm::string_view> : std::true_type
{
};
template <>
struct is_pathable<cm::static_string_view> : std::true_type
{
};
template <typename T>
struct is_pathable<
  T,
  cm::enable_if_t<std::is_same<char*, typename std::decay<T>::type>::value,
                  void>>
  : cm::bool_constant<std::is_same<char*, typename std::decay<T>::type>::value>
{
};

template <typename T>
struct is_move_pathable : std::false_type
{
};

template <>
struct is_move_pathable<cm::filesystem::path> : std::true_type
{
};
template <>
struct is_move_pathable<std::string> : std::true_type
{
};
#endif
}

class cmCMakePath
{
private:
  template <typename Source>
  using enable_if_move_pathable =
    cm::enable_if_t<detail::is_move_pathable<Source>::value, cmCMakePath&>;

  template <typename Source>
  using enable_if_pathable =
    cm::enable_if_t<detail::is_pathable<Source>::value, cmCMakePath&>;

public:
  using value_type = cm::filesystem::path::value_type;
  using string_type = cm::filesystem::path::string_type;

  enum format : unsigned char
  {
    auto_format =
      static_cast<unsigned char>(cm::filesystem::path::format::auto_format),
    native_format =
      static_cast<unsigned char>(cm::filesystem::path::format::native_format),
    generic_format =
      static_cast<unsigned char>(cm::filesystem::path::format::generic_format)
  };

  class iterator;
  using const_iterator = iterator;

  cmCMakePath() noexcept = default;

  cmCMakePath(cmCMakePath const&) = default;

  cmCMakePath(cmCMakePath&& path) noexcept
    : Path(std::forward<cm::filesystem::path>(path.Path))
  {
  }

  cmCMakePath(cm::filesystem::path path) noexcept
    : Path(std::move(path))
  {
  }
  cmCMakePath(cm::string_view source, format fmt = generic_format) noexcept
    : Path(FormatPath(source, fmt))
  {
  }
  cmCMakePath(char const* source, format fmt = generic_format) noexcept
    : Path(FormatPath(cm::string_view{ source }, fmt))
  {
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmCMakePath(std::string const& source, format fmt = generic_format)
    : Path(FormatPath(source, fmt))
  {
  }
  cmCMakePath(std::string&& source, format fmt = generic_format)
    : Path(FormatPath(std::move(source), fmt))
  {
  }
#else
  template <typename Source, typename = enable_if_move_pathable<Source>>
  cmCMakePath(Source source, format fmt = generic_format)
    : Path(FormatPath(std::move(source), fmt))
  {
  }
#endif

  template <typename Source, typename = enable_if_move_pathable<Source>>
  cmCMakePath& Assign(Source&& source)
  {
    this->Path = std::forward<Source>(source);
    return *this;
  }
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath& Assign(Source const& source)
  {
    this->Path = source;
    return *this;
  }

  cmCMakePath& operator=(cmCMakePath const& path)
  {
    if (this != &path) {
      this->Path = path.Path;
    }
    return *this;
  }
  cmCMakePath& operator=(cmCMakePath&& path) noexcept
  {
    if (this != &path) {
      this->Path = std::move(path.Path);
    }
    return *this;
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmCMakePath& operator=(cm::filesystem::path&& source)
  {
    this->Assign(std::forward<cm::filesystem::path>(source));
    return *this;
  }
  cmCMakePath& operator=(std::string&& source)
  {
    this->Assign(std::forward<std::string>(source));
    return *this;
  }
  cmCMakePath& operator=(cm::filesystem::path const& source)
  {
    this->Assign(source);
    return *this;
  }
  cmCMakePath& operator=(std::string const& source)
  {
    this->Assign(source);
    return *this;
  }
  cmCMakePath& operator=(cm::string_view const source)
  {
    this->Assign(source);
    return *this;
  }
  cmCMakePath& operator=(char const* source)
  {
    this->Assign(cm::string_view{ source });
    return *this;
  }
#else
  template <typename Source, typename = enable_if_move_pathable<Source>>
  cmCMakePath& operator=(Source&& source)
  {
    this->Assign(std::forward<Source>(source));
    return *this;
  }
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath& operator=(Source const& source)
  {
    this->Assign(source);
    return *this;
  }
#endif

  // Concatenation
  cmCMakePath& Append(cmCMakePath const& path)
  {
    return this->Append(path.Path);
  }
  cmCMakePath& Append(cm::filesystem::path const& path)
  {
    this->Path /= path;
    // filesystem::path::append use preferred_separator ('\' on Windows)
    // so convert back to '/'
    this->Path = this->Path.generic_string();
    return *this;
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmCMakePath& Append(std::string const& source)
  {
    return this->Append(cm::filesystem::path(source));
  }
  cmCMakePath& Append(cm::string_view source)
  {
    return this->Append(cm::filesystem::path(source));
  }
  cmCMakePath& Append(char const* source)
  {
    return this->Append(cm::filesystem::path(cm::string_view{ source }));
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath& Append(Source const& source)
  {
    return this->Append(cm::filesystem::path(source));
  }
#endif

  cmCMakePath& operator/=(cmCMakePath const& path)
  {
    return this->Append(path);
  }
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath& operator/=(Source const& source)
  {
    return this->Append(source);
  }

  cmCMakePath& Concat(cmCMakePath const& path)
  {
    this->Path += path.Path;
    return *this;
  }
  cmCMakePath& Concat(cm::string_view source)
  {
    this->Path.operator+=(std::string(source));
    return *this;
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmCMakePath& Concat(cm::filesystem::path const& source)
  {
    this->Path.operator+=(source);
    return *this;
  }
  cmCMakePath& Concat(std::string const& source)
  {
    this->Path.operator+=(source);
    return *this;
  }
  cmCMakePath& Concat(char const* source)
  {
    this->Path.operator+=(source);
    return *this;
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath& Concat(Source const& source)
  {
    this->Path.operator+=(source);
    return *this;
  }
#endif

  cmCMakePath& operator+=(cmCMakePath const& path)
  {
    return this->Concat(path);
  }
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath& operator+=(Source const& source)
  {
    return this->Concat(source);
  }

  // Manipulation
  void Clear() noexcept { this->Path.clear(); }

  cmCMakePath& RemoveFileName()
  {
    this->Path.remove_filename();
    return *this;
  }

  cmCMakePath& ReplaceFileName(cmCMakePath const& filename)
  {
    if (this->Path.has_filename()) {
      this->Path.replace_filename(filename.Path);
    }
    return *this;
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmCMakePath& ReplaceFileName(cm::filesystem::path const& filename)
  {
    if (this->Path.has_filename()) {
      this->Path.replace_filename(filename);
    }
    return *this;
  }
  cmCMakePath& ReplaceFileName(std::string const& filename)
  {
    if (this->Path.has_filename()) {
      this->Path.replace_filename(filename);
    }
    return *this;
  }
  cmCMakePath& ReplaceFileName(cm::string_view filename)
  {
    if (this->Path.has_filename()) {
      this->Path.replace_filename(filename);
    }
    return *this;
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath& ReplaceFileName(Source const& filename)
  {
    if (this->Path.has_filename()) {
      this->Path.replace_filename(filename);
    }
    return *this;
  }
#endif

  cmCMakePath& ReplaceExtension(cmCMakePath const& extension = cmCMakePath())
  {
    this->Path.replace_extension(extension.Path);
    return *this;
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmCMakePath& ReplaceExtension(cm::filesystem::path const& extension)
  {
    this->Path.replace_extension(extension);
    return *this;
  }
  cmCMakePath& ReplaceExtension(std::string const& extension)
  {
    this->Path.replace_extension(extension);
    return *this;
  }
  cmCMakePath& ReplaceExtension(cm::string_view const extension)
  {
    this->Path.replace_extension(extension);
    return *this;
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath& ReplaceExtension(Source const& extension)
  {
    this->Path.replace_extension(extension);
    return *this;
  }
#endif

  cmCMakePath& ReplaceWideExtension(
    cmCMakePath const& extension = cmCMakePath())
  {
    return this->ReplaceWideExtension(
      static_cast<cm::string_view>(extension.Path.string()));
  }
  cmCMakePath& ReplaceWideExtension(cm::filesystem::path const& extension)
  {
    return this->ReplaceWideExtension(
      static_cast<cm::string_view>(extension.string()));
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmCMakePath& ReplaceWideExtension(std::string const& extension)
  {
    return this->ReplaceWideExtension(cm::string_view{ extension });
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath& ReplaceWideExtension(Source const& extension)
  {
    return this->ReplaceWideExtension(extension);
  }
#endif
  cmCMakePath& ReplaceWideExtension(cm::string_view extension);

  cmCMakePath& RemoveExtension()
  {
    if (this->Path.has_extension()) {
      this->ReplaceExtension(cm::string_view(""));
    }
    return *this;
  }

  cmCMakePath& RemoveWideExtension()
  {
    if (this->Path.has_extension()) {
      this->ReplaceWideExtension(cm::string_view(""));
    }
    return *this;
  }

  void swap(cmCMakePath& other) noexcept { this->Path.swap(other.Path); }

  // Observers
  std::string String() const { return this->Path.string(); }
  std::wstring WString() const { return this->Path.wstring(); }

  string_type Native() const
  {
    string_type path;
    this->GetNativePath(path);

    return path;
  }
  std::string NativeString() const
  {
    std::string path;
    this->GetNativePath(path);

    return path;
  }
  std::wstring NativeWString() const
  {
    std::wstring path;
    this->GetNativePath(path);

    return path;
  }
  std::string GenericString() const { return this->Path.generic_string(); }
  std::wstring GenericWString() const { return this->Path.generic_wstring(); }

  // Decomposition
  cmCMakePath GetRootName() const { return this->Path.root_name(); }
  cmCMakePath GetRootDirectory() const { return this->Path.root_directory(); }
  cmCMakePath GetRootPath() const { return this->Path.root_path(); }
  cmCMakePath GetFileName() const { return this->Path.filename(); }
  cmCMakePath GetExtension() const { return this->Path.extension(); }
  cmCMakePath GetWideExtension() const;
  cmCMakePath GetStem() const { return this->Path.stem(); }
  cmCMakePath GetNarrowStem() const;

  cmCMakePath GetRelativePath() const { return this->Path.relative_path(); }
  cmCMakePath GetParentPath() const { return this->Path.parent_path(); }

  // Generation
  cmCMakePath Normal() const
  {
    auto path = this->Path.lexically_normal();
    // filesystem::path:lexically_normal use preferred_separator ('\') on
    // Windows) so convert back to '/'
    return path.generic_string();
  }

  cmCMakePath Relative(cmCMakePath const& base) const
  {
    return this->Relative(base.Path);
  }
  cmCMakePath Relative(cm::filesystem::path const& base) const
  {
    auto path = this->Path.lexically_relative(base);
    // filesystem::path:lexically_relative use preferred_separator ('\') on
    // Windows) so convert back to '/'
    return path.generic_string();
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmCMakePath Relative(std::string const& base) const
  {
    return this->Relative(cm::filesystem::path(base));
  }
  cmCMakePath Relative(cm::string_view base) const
  {
    return this->Relative(cm::filesystem::path(base));
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath Relative(Source const& base) const
  {
    return this->Relative(cm::filesystem::path(base));
  }
#endif
  cmCMakePath Proximate(cmCMakePath const& base) const
  {
    return this->Proximate(base.Path);
  }
  cmCMakePath Proximate(cm::filesystem::path const& base) const
  {
    auto path = this->Path.lexically_proximate(base);
    // filesystem::path::lexically_proximate use preferred_separator ('\') on
    // Windows) so convert back to '/'
    return path.generic_string();
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmCMakePath Proximate(std::string const& base) const
  {
    return this->Proximate(cm::filesystem::path(base));
  }
  cmCMakePath Proximate(cm::string_view base) const
  {
    return this->Proximate(cm::filesystem::path(base));
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath Proximate(Source const& base) const
  {
    return this->Proximate(cm::filesystem::path(base));
  }
#endif

  cmCMakePath Absolute(cmCMakePath const& base) const
  {
    return this->Absolute(base.Path);
  }
#if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler on Solaris/Sparc is confused when
  // standard methods and templates use the same name. The template is selected
  // rather than the standard one regardless the arguments of the method.
  cmCMakePath Absolute(std::string const& base) const
  {
    return this->Absolute(cm::filesystem::path(base));
  }
  cmCMakePath Absolute(cm::string_view base) const
  {
    return this->Absolute(cm::filesystem::path(base));
  }
#else
  template <typename Source, typename = enable_if_pathable<Source>>
  cmCMakePath Absolute(Source const& base) const
  {
    return this->Absolute(cm::filesystem::path(base));
  }
#endif
  cmCMakePath Absolute(cm::filesystem::path const& base) const;

  // Comparison
  int Compare(cmCMakePath const& path) const noexcept
  {
    return this->Path.compare(path.Path);
  }

  // Query
  bool IsEmpty() const noexcept { return this->Path.empty(); }

  bool HasRootPath() const { return this->Path.has_root_path(); }
  bool HasRootName() const { return this->Path.has_root_name(); }
  bool HasRootDirectory() const { return this->Path.has_root_directory(); }
  bool HasRelativePath() const { return this->Path.has_relative_path(); }
  bool HasParentPath() const { return this->Path.has_parent_path(); }
  bool HasFileName() const { return this->Path.has_filename(); }
  bool HasStem() const { return this->Path.has_stem(); }
  bool HasExtension() const { return this->Path.has_extension(); }

  bool IsAbsolute() const { return this->Path.is_absolute(); }
  bool IsRelative() const { return this->Path.is_relative(); }
  bool IsPrefix(cmCMakePath const& path) const;

  // Iterators
  // =========
  inline iterator begin() const;
  inline iterator end() const;

  // Non-members
  // ===========
  friend bool operator==(cmCMakePath const& lhs,
                         cmCMakePath const& rhs) noexcept
  {
    return lhs.Compare(rhs) == 0;
  }
  friend bool operator!=(cmCMakePath const& lhs,
                         cmCMakePath const& rhs) noexcept
  {
    return lhs.Compare(rhs) != 0;
  }

  friend cmCMakePath operator/(cmCMakePath const& lhs, cmCMakePath const& rhs)
  {
    cmCMakePath result(lhs);
    result /= rhs;

    return result;
  }

private:
  friend std::size_t hash_value(cmCMakePath const& path) noexcept;

  static std::string FormatPath(std::string path, format fmt = generic_format);
  static std::string FormatPath(cm::string_view path,
                                format fmt = generic_format)
  {
    return FormatPath(std::string(path), fmt);
  }

  void GetNativePath(std::string& path) const;
  void GetNativePath(std::wstring& path) const;

  cm::filesystem::path Path;
};

class cmCMakePath::iterator
{
public:
  using iterator_category = cm::filesystem::path::iterator::iterator_category;

  using value_type = cmCMakePath;
  using difference_type = cm::filesystem::path::iterator::difference_type;
  using pointer = cmCMakePath const*;
  using reference = cmCMakePath const&;

  iterator() = default;

  iterator(iterator const& other)
    : Iterator(other.Iterator)
    , Path(other.Path)
    , PathElement(*this->Iterator)
  {
  }

  ~iterator() = default;

  iterator& operator=(iterator const& other)
  {
    if (this != &other) {
      this->Iterator = other.Iterator;
      this->Path = other.Path;
      this->PathElement = *this->Iterator;
    }

    return *this;
  }

  reference operator*() const { return this->PathElement; }

  pointer operator->() const { return &this->PathElement; }

  iterator& operator++()
  {
    ++this->Iterator;
    this->PathElement = *this->Iterator;

    return *this;
  }

  iterator operator++(int)
  {
    iterator it(*this);
    this->operator++();
    return it;
  }

  iterator& operator--()
  {
    --this->Iterator;
    this->PathElement = *this->Iterator;

    return *this;
  }

  iterator operator--(int)
  {
    iterator it(*this);
    this->operator--();
    return it;
  }

private:
  friend class cmCMakePath;
  friend bool operator==(iterator const&, iterator const&);

  iterator(cmCMakePath const* path, cm::filesystem::path::iterator const& it)
    : Iterator(it)
    , Path(path)
    , PathElement(*this->Iterator)
  {
  }

  cm::filesystem::path::iterator Iterator;
  cmCMakePath const* Path = nullptr;
  cmCMakePath PathElement;
};

inline cmCMakePath::iterator cmCMakePath::begin() const
{
  return iterator(this, this->Path.begin());
}
inline cmCMakePath::iterator cmCMakePath::end() const
{
  return iterator(this, this->Path.end());
}

// Non-member functions
// ====================
inline bool operator==(cmCMakePath::iterator const& lhs,
                       cmCMakePath::iterator const& rhs)
{
  return lhs.Path == rhs.Path && lhs.Path && lhs.Iterator == rhs.Iterator;
}

inline bool operator!=(cmCMakePath::iterator const& lhs,
                       cmCMakePath::iterator const& rhs)
{
  return !(lhs == rhs);
}

inline void swap(cmCMakePath& lhs, cmCMakePath& rhs) noexcept
{
  lhs.swap(rhs);
}

inline std::size_t hash_value(cmCMakePath const& path) noexcept
{
  return cm::filesystem::hash_value(path.Path);
}
