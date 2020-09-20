/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <cm/filesystem> // IWYU pragma: associated

#if !defined(CMake_HAVE_CXX_FILESYSTEM)

#  include <algorithm>
#  include <cassert>
#  include <cstddef>
#  include <cstdlib>
#  include <functional>
#  include <string>
#  include <utility>
#  include <vector>
#  if defined(_WIN32) && !defined(__CYGWIN__)
#    include <cctype>
#  endif
#  if defined(_WIN32) || defined(__CYGWIN__)
#    include <iterator>
#  endif

#  include <cm/memory>
#  include <cm/string_view>
#  include <cmext/string_view>

namespace cm {
namespace filesystem {
namespace internals {

class path_parser
{
#  if defined(__SUNPRO_CC) && defined(__sparc)
  // Oracle DeveloperStudio C++ compiler generates wrong code if enum size
  // is different than the default.
  using enum_size = int;
#  else
  using enum_size = unsigned char;
#  endif

  enum class state : enum_size
  {
    before_begin,
    in_root_name,
    in_root_dir,
    in_filename,
    trailing_separator,
    at_end
  };

  using pointer = char const*;

public:
  enum class seek_position : enum_size
  {
    root_name = static_cast<enum_size>(state::in_root_name),
    root_directory = static_cast<enum_size>(state::in_root_dir)
  };
  enum class peek_fragment : enum_size
  {
    remainder,
    path
  };

  path_parser(cm::string_view path, bool set_at_end = false)
    : State(set_at_end ? state::at_end : state::before_begin)
    , Path(path)
  {
  }

  path_parser(const path_parser&) = default;

  ~path_parser() = default;

  void reset() noexcept { this->set_state(state::before_begin); }

  void increment() noexcept
  {
    const pointer start = this->next_token();
    const pointer end = this->after_end();

    if (start == end) {
      this->set_state(state::at_end);
      return;
    }

    switch (this->State) {
      case state::before_begin: {
        auto pos = this->consume_root_name(start, end);
        if (pos) {
          this->set_state(state::in_root_name);
        } else {
          pos = this->consume_separator(start, end);
          if (pos) {
            this->set_state(state::in_root_dir);
          } else {
            this->consume_filename(start, end);
            this->set_state(state::in_filename);
          }
        }
        break;
      }
      case state::in_root_name: {
        auto pos = this->consume_separator(start, end);
        if (pos) {
          this->set_state(state::in_root_dir);
        } else {
          this->consume_filename(start, end);
          this->set_state(state::in_filename);
        }
        break;
      }
      case state::in_root_dir: {
        this->consume_filename(start, end);
        this->set_state(state::in_filename);
        break;
      }
      case state::in_filename: {
        auto posSep = this->consume_separator(start, end);
        if (posSep != end) {
          auto pos = this->consume_filename(posSep, end);
          if (pos) {
            return;
          }
        }
        set_state(state::trailing_separator);
        break;
      }
      case state::trailing_separator: {
        this->set_state(state::at_end);
        break;
      }
      case state::at_end:
        // unreachable
        std::abort();
    }
  }

  void decrement() noexcept
  {
    const pointer rstart = this->current_token() - 1;
    const pointer rend = this->before_start();

    if (rstart == rend) {
      this->set_state(state::before_begin);
      return;
    }

    switch (this->State) {
      case state::at_end: {
        auto posSep = this->consume_separator(rstart, rend);
        if (posSep) {
          if (posSep == rend) {
            this->set_state(state::in_root_dir);
          } else {
            auto pos = this->consume_root_name(posSep, rend, true);
            if (pos == rend) {
              this->set_state(state::in_root_dir);
            } else {
              this->set_state(state::trailing_separator);
            }
          }
        } else {
          auto pos = this->consume_root_name(rstart, rend);
          if (pos == rend) {
            this->set_state(state::in_root_name);
          } else {
            this->consume_filename(rstart, rend);
            this->set_state(state::in_filename);
          }
        }
        break;
      }
      case state::trailing_separator: {
        this->consume_filename(rstart, rend);
        this->set_state(state::in_filename);
        break;
      }
      case state::in_filename: {
        auto posSep = this->consume_separator(rstart, rend);
        if (posSep == rend) {
          this->set_state(state::in_root_dir);
        } else {
          auto pos = this->consume_root_name(posSep ? posSep : rstart, rend,
                                             posSep != nullptr);
          if (pos == rend) {
            this->set_state(posSep ? state::in_root_dir : state::in_root_name);
          } else {
            this->consume_filename(posSep, rend);
            this->set_state(state::in_filename);
          }
        }
        break;
      }
      case state::in_root_dir: {
        auto pos = this->consume_root_name(rstart, rend);
        if (pos) {
          this->set_state(state::in_root_name);
        }
        break;
      }
      case state::in_root_name:
      case state::before_begin: {
        // unreachable
        std::abort();
      }
    }
  }

  path_parser& operator++() noexcept
  {
    this->increment();
    return *this;
  }

  path_parser& operator--() noexcept
  {
    this->decrement();
    return *this;
  }

  cm::string_view operator*() const noexcept
  {
    switch (this->State) {
      case state::before_begin:
      case state::at_end:
        return cm::string_view();
      case state::trailing_separator:
        return "";
      case state::in_root_dir:
      case state::in_root_name:
      case state::in_filename:
        return this->Entry;
      default:
        // unreachable
        std::abort();
    }
  }

  void seek(seek_position position)
  {
    state s = static_cast<state>(static_cast<int>(position));

    while (this->State <= s) {
      this->increment();
    }
  }

  cm::string_view peek(peek_fragment fragment)
  {
    if (fragment == peek_fragment::remainder) {
      // peek-up remain part of the initial path
      return { this->Entry.data(),
               std::size_t(&this->Path.back() - this->Entry.data() + 1) };
    }
    if (fragment == peek_fragment::path) {
      // peek-up full path until current position
      return { this->Path.data(),
               std::size_t(&this->Entry.back() - this->Path.data() + 1) };
    }
    return {};
  }

  bool in_root_name() const { return this->State == state::in_root_name; }
  bool in_root_directory() const { return this->State == state::in_root_dir; }
  bool at_end() const { return this->State == state::at_end; }

  bool at_start() const { return this->Entry.data() == this->Path.data(); }

private:
  void set_state(state newState) noexcept
  {
    this->State = newState;
    if (newState == state::before_begin || newState == state::at_end) {
      this->Entry = {};
    }
  }

  pointer before_start() const noexcept { return this->Path.data() - 1; }
  pointer after_end() const noexcept
  {
    return this->Path.data() + this->Path.size();
  }

  pointer current_token() const noexcept
  {
    switch (this->State) {
      case state::before_begin:
      case state::in_root_name:
        return &this->Path.front();
      case state::in_root_dir:
      case state::in_filename:
      case state::trailing_separator:
        return &this->Entry.front();
      case state::at_end:
        return &this->Path.back() + 1;
      default:
        // unreachable
        std::abort();
    }
  }
  pointer next_token() const noexcept
  {
    switch (this->State) {
      case state::before_begin:
        return this->Path.data();
      case state::in_root_name:
      case state::in_root_dir:
      case state::in_filename:
        return &this->Entry.back() + 1;
      case state::trailing_separator:
      case state::at_end:
        return after_end();
      default:
        // unreachable
        std::abort();
    }
  }

  pointer consume_separator(pointer ptr, pointer end) noexcept
  {
    if (ptr == end ||
        (*ptr != '/'
#  if defined(_WIN32)
         && *ptr != '\\'
#  endif
         )) {
      return nullptr;
    }
    const auto step = ptr < end ? 1 : -1;
    ptr += step;
    while (ptr != end &&
           (*ptr == '/'
#  if defined(_WIN32)
            || *ptr == '\\'
#  endif
            )) {
      ptr += step;
    }
    if (step == 1) {
      this->Entry = cm::string_view(ptr - 1, 1);
    } else {
      this->Entry = cm::string_view(ptr + 1, 1);
    }

    return ptr;
  }

  pointer consume_filename(pointer ptr, pointer end) noexcept
  {
    auto start = ptr;

    if (ptr == end || *ptr == '/'
#  if defined(_WIN32)
        || *ptr == '\\'
#  endif
    ) {
      return nullptr;
    }
    const auto step = ptr < end ? 1 : -1;
    ptr += step;
    while (ptr != end && *ptr != '/'
#  if defined(_WIN32)
           && *ptr != '\\'
#  endif
    ) {
      ptr += step;
    }

#  if defined(_WIN32)
    if (step == -1 && (start - ptr) >= 2 && ptr == end) {
      // rollback drive name consumption, if any
      if (this->is_drive_name(ptr + 1)) {
        ptr += 2;
      }
      if (ptr == start) {
        return nullptr;
      }
    }
#  endif

    if (step == 1) {
      this->Entry = cm::string_view(start, ptr - start);
    } else {
      this->Entry = cm::string_view(ptr + 1, start - ptr);
    }

    return ptr;
  }

#  if defined(_WIN32)
  bool is_drive_name(pointer ptr)
  {
    return std::toupper(ptr[0]) >= 'A' && std::toupper(ptr[0]) <= 'Z' &&
      ptr[1] == ':';
  }
#  endif

  pointer consume_root_name(pointer ptr, pointer end,
                            bool check_only = false) noexcept
  {
#  if defined(_WIN32) && !defined(__CYGWIN__)
    if (ptr < end) {
      if ((end - ptr) >= 2 && this->is_drive_name(ptr)) {
        // Drive letter (X:) is a root name
        if (!check_only) {
          this->Entry = cm::string_view(ptr, 2);
        }
        return ptr + 2;
      }
      if ((end - ptr) > 2 && (ptr[0] == '/' || ptr[0] == '\\') &&
          (ptr[1] == '/' || ptr[1] == '\\') &&
          (ptr[2] != '/' && ptr[2] != '\\')) {
        // server name (//server) is a root name
        auto pos = std::find_if(ptr + 2, end,
                                [](char c) { return c == '/' || c == '\\'; });
        if (!check_only) {
          this->Entry = cm::string_view(ptr, pos - ptr);
        }
        return pos;
      }
    } else {
      if ((ptr - end) >= 2 && this->is_drive_name(ptr - 1)) {
        // Drive letter (X:) is a root name
        if (!check_only) {
          this->Entry = cm::string_view(ptr - 1, 2);
        }
        return ptr - 2;
      }
      if ((ptr - end) > 2 && (ptr[0] != '/' && ptr[0] != '\\')) {
        std::reverse_iterator<pointer> start(ptr);
        std::reverse_iterator<pointer> stop(end);
        auto res = std::find_if(start, stop,
                                [](char c) { return c == '/' || c == '\\'; });
        pointer pos = res.base() - 1;
        if ((pos - 1) > end && (pos[-1] == '/' || pos[-1] == '\\')) {
          // server name (//server) is a root name
          if (!check_only) {
            this->Entry = cm::string_view(pos - 1, ptr - pos + 2);
          }
          return pos - 2;
        }
      }
    }
#  elif defined(__CYGWIN__)
    if (ptr < end) {
      if ((end - ptr) > 2 && ptr[0] == '/' && ptr[1] == '/' && ptr[2] != '/') {
        // server name (//server) is a root name
        auto pos = std::find(ptr + 2, end, '/');
        if (!check_only) {
          this->Entry = cm::string_view(ptr, pos - ptr);
        }
        return pos;
      }
    } else {
      if ((ptr - end) > 2 && ptr[0] != '/') {
        std::reverse_iterator<pointer> start(ptr);
        std::reverse_iterator<pointer> stop(end);
        auto res = std::find(start, stop, '/');
        pointer pos = res.base() - 1;
        if ((pos - 1) > end && pos[-1] == '/') {
          // server name (//server) is a root name
          if (!check_only) {
            this->Entry = cm::string_view(pos - 1, ptr - pos + 2);
          }
          return pos - 2;
        }
      }
    }
#  else
    (void)ptr;
    (void)end;
    (void)check_only;
#  endif
    return nullptr;
  }

  state State;
  const cm::string_view Path;
  cm::string_view Entry;
};

// class unicode_helper
void unicode_helper::append(std::string& str, std::uint32_t codepoint)
{
  if (codepoint <= 0x7f) {
    str.push_back(static_cast<char>(codepoint));
  } else if (codepoint >= 0x80 && codepoint <= 0x7ff) {
    str.push_back(static_cast<char>((codepoint >> 6) + 192));
    str.push_back(static_cast<char>((codepoint & 0x3f) + 128));
  } else if ((codepoint >= 0x800 && codepoint <= 0xd7ff) ||
             (codepoint >= 0xe000 && codepoint <= 0xffff)) {
    str.push_back(static_cast<char>((codepoint >> 12) + 224));
    str.push_back(static_cast<char>(((codepoint & 0xfff) >> 6) + 128));
    str.push_back(static_cast<char>((codepoint & 0x3f) + 128));
  } else if (codepoint >= 0x10000 && codepoint <= 0x10ffff) {
    str.push_back(static_cast<char>((codepoint >> 18) + 240));
    str.push_back(static_cast<char>(((codepoint & 0x3ffff) >> 12) + 128));
    str.push_back(static_cast<char>(((codepoint & 0xfff) >> 6) + 128));
    str.push_back(static_cast<char>((codepoint & 0x3f) + 128));
  } else {
    append(str, 0xfffd);
  }
}

unicode_helper::utf8_state unicode_helper::decode(const utf8_state state,
                                                  const std::uint8_t fragment,
                                                  std::uint32_t& codepoint)
{
  const std::uint32_t utf8_state_info[] = {
    // encoded states
    0x11111111u, 0x11111111u, 0x77777777u, 0x77777777u, 0x88888888u,
    0x88888888u, 0x88888888u, 0x88888888u, 0x22222299u, 0x22222222u,
    0x22222222u, 0x22222222u, 0x3333333au, 0x33433333u, 0x9995666bu,
    0x99999999u, 0x88888880u, 0x22818108u, 0x88888881u, 0x88888882u,
    0x88888884u, 0x88888887u, 0x88888886u, 0x82218108u, 0x82281108u,
    0x88888888u, 0x88888883u, 0x88888885u, 0u,          0u,
    0u,          0u,
  };
  std::uint8_t category = fragment < 128
    ? 0
    : (utf8_state_info[(fragment >> 3) & 0xf] >> ((fragment & 7) << 2)) & 0xf;
  codepoint = (state ? (codepoint << 6) | (fragment & 0x3fu)
                     : (0xffu >> category) & fragment);
  return state == s_reject
    ? s_reject
    : static_cast<utf8_state>(
        (utf8_state_info[category + 16] >> (state << 2)) & 0xf);
}

} // internals

// Class path
path& path::operator/=(const path& p)
{
  if (p.is_absolute() ||
      (p.has_root_name() && p.get_root_name() != this->get_root_name())) {
    this->path_ = p.path_;
    return *this;
  }
  if (p.has_root_directory()) {
    this->path_ = static_cast<std::string>(this->get_root_name());
    this->path_ += static_cast<std::string>(p.get_root_directory());
  } else if (this->has_filename()) {
    this->path_ += this->preferred_separator;
#  if defined(_WIN32) || defined(__CYGWIN__)
    // special case: "//host" / "b" => "//host/b"
  } else if (this->has_root_name() && !this->has_root_directory()) {
    if (this->path_.length() >= 3 &&
        (this->path_[0] == '/'
#    if defined(_WIN32) && !defined(__CYGWIN__)
         || this->path_[0] == '\\'
#    endif
         ) &&
        (this->path_[1] == '/'
#    if defined(_WIN32) && !defined(__CYGWIN__)
         || this->path_[1] == '\\'
#    endif
         ) &&
        (this->path_[2] != '/'
#    if defined(_WIN32) && !defined(__CYGWIN__)
         && this->path_[2] != '\\'
#    endif
         )) {
      this->path_ += this->preferred_separator;
    }
#  endif
  }

  this->path_ += p.get_relative_path();
  return *this;
}

path path::lexically_normal() const
{
  if (this->path_.empty()) {
    return *this;
  }

  const cm::string_view dot = "."_s;
  const cm::string_view dotdot = ".."_s;

  std::vector<cm::string_view> root_parts;
  std::vector<cm::string_view> parts;
  bool root_directory_defined = false;
  bool need_final_separator = false;
  std::size_t path_size = 0;

  internals::path_parser parser(this->path_);
  ++parser;
  while (!parser.at_end()) {
    auto part = *parser;

    if (parser.in_root_name() || parser.in_root_directory()) {
      if (parser.in_root_directory()) {
        root_directory_defined = true;
      }
      root_parts.push_back(part);
      path_size += part.size();
    } else if (part == dotdot) {
      if (!parts.empty() && parts.back() != dotdot) {
        need_final_separator = true;
        path_size -= parts.back().size();
        parts.pop_back();
      } else if ((parts.empty() || parts.back() == dotdot) &&
                 !root_directory_defined) {
        parts.push_back(dotdot);
        path_size += 2;
      }

    } else if (part == dot || part.empty()) {
      need_final_separator = true;
      if (part.empty()) {
        parts.push_back(part);
      }
    } else {
      // filename
      need_final_separator = false;
      parts.push_back(part);
      path_size += part.size();
    }
    ++parser;
  }

  // no final separator if last element of path is ".."
  need_final_separator =
    need_final_separator && !parts.empty() && parts.back() != dotdot;

  // build final path
  //// compute final size of path
  path_size += parts.size() + (need_final_separator ? 1 : 0);

  std::string np;
  np.reserve(path_size);
  for (const auto& p : root_parts) {
    np += p;
  }
  // convert any slash to the preferred_separator
  if (static_cast<std::string::value_type>(this->preferred_separator) != '/') {
    std::replace(
      np.begin(), np.end(), '/',
      static_cast<std::string::value_type>(this->preferred_separator));
  }
  for (const auto& p : parts) {
    if (!p.empty()) {
      np += p;
      np += static_cast<std::string::value_type>(this->preferred_separator);
    }
  }
  if (!parts.empty() && !need_final_separator) {
    // remove extra separator
    np.pop_back();
  }
  if (np.empty()) {
    np.assign(1, '.');
  }

  return path(std::move(np));
}

path path::lexically_relative(const path& base) const
{
  internals::path_parser parser(this->path_);
  ++parser;
  internals::path_parser parserbase(base.path_);
  ++parserbase;
  cm::string_view this_root_name, base_root_name;
  cm::string_view this_root_dir, base_root_dir;

  if (parser.in_root_name()) {
    this_root_name = *parser;
    ++parser;
  }
  if (parser.in_root_directory()) {
    this_root_dir = *parser;
    ++parser;
  }
  if (parserbase.in_root_name()) {
    base_root_name = *parserbase;
    ++parserbase;
  }
  if (parserbase.in_root_directory()) {
    base_root_dir = *parserbase;
    ++parserbase;
  }

  auto is_path_absolute = [](cm::string_view rn, cm::string_view rd) -> bool {
#  if defined(_WIN32) && !defined(__CYGWIN__)
    return !rn.empty() && !rd.empty();
#  else
    (void)rn;
    return !rd.empty();
#  endif
  };

  if (this_root_name != base_root_name ||
      is_path_absolute(this_root_name, this_root_dir) !=
        is_path_absolute(base_root_name, base_root_dir) ||
      (this_root_dir.empty() && !base_root_dir.empty())) {
    return path();
  }

#  if defined(_WIN32) && !defined(__CYGWIN__)
  // LWG3070 handle special case: filename can also be a root-name
  auto is_drive_name = [](cm::string_view item) -> bool {
    return item.length() == 2 && item[1] == ':';
  };
  parser.reset();
  parser.seek(internals::path_parser::seek_position::root_directory);
  while (!parser.at_end()) {
    if (is_drive_name(*parser)) {
      return path();
    }
    ++parser;
  }
  parserbase.reset();
  parserbase.seek(internals::path_parser::seek_position::root_directory);
  while (!parserbase.at_end()) {
    if (is_drive_name(*parserbase)) {
      return path();
    }
    ++parserbase;
  }
#  endif

  const cm::string_view dot = "."_s;
  const cm::string_view dotdot = ".."_s;

  auto a = this->begin(), aend = this->end();
  auto b = base.begin(), bend = base.end();
  while (a != aend && b != bend && a->string() == b->string()) {
    ++a;
    ++b;
  }

  int count = 0;
  for (; b != bend; ++b) {
    auto part = *b;
    if (part == dotdot) {
      --count;
    } else if (part.string() != dot && !part.empty()) {
      ++count;
    }
  }

  if (count == 0 && (a == this->end() || a->empty())) {
    return path(dot);
  }
  if (count >= 0) {
    path result;
    path p_dotdot(dotdot);
    for (int i = 0; i < count; ++i) {
      result /= p_dotdot;
    }
    for (; a != aend; ++a) {
      result /= *a;
    }
    return result;
  }
  // count < 0
  return path();
}

path::path_type path::get_generic() const
{
  auto gen_path = this->path_;
  auto start = gen_path.begin();
#  if defined(_WIN32) && !defined(__CYGWIN__)
  std::replace(gen_path.begin(), gen_path.end(), '\\', '/');
  // preserve special syntax for root_name ('//server' or '//?')
  if (gen_path.length() > 2 && gen_path[2] != '/') {
    start += 2;
  }
#  endif
  // remove duplicate separators
  auto new_end = std::unique(start, gen_path.end(), [](char lhs, char rhs) {
    return lhs == rhs && lhs == '/';
  });
  gen_path.erase(new_end, gen_path.end());
  return gen_path;
}

cm::string_view path::get_root_name() const
{
  internals::path_parser parser(this->path_);
  ++parser;
  if (parser.in_root_name()) {
    return *parser;
  }
  return {};
}

cm::string_view path::get_root_directory() const
{
  internals::path_parser parser(this->path_);
  ++parser;
  if (parser.in_root_name()) {
    ++parser;
  }
  if (parser.in_root_directory()) {
    return *parser;
  }
  return {};
}

cm::string_view path::get_relative_path() const
{
  internals::path_parser parser(this->path_);
  parser.seek(internals::path_parser::seek_position::root_directory);
  if (parser.at_end()) {
    return {};
  }
  return parser.peek(internals::path_parser::peek_fragment::remainder);
}

cm::string_view path::get_parent_path() const
{
  if (!this->has_relative_path()) {
    return this->path_;
  }

  // peek-up full path minus last element
  internals::path_parser parser(this->path_, true);
  --parser;
  if (parser.at_start()) {
    return {};
  }
  --parser;
  return parser.peek(internals::path_parser::peek_fragment::path);
}

cm::string_view path::get_filename() const
{
  {
    internals::path_parser parser(this->path_);
    parser.seek(internals::path_parser::seek_position::root_directory);
    if (parser.at_end()) {
      return {};
    }
  }
  {
    internals::path_parser parser(this->path_, true);
    return *(--parser);
  }
}

cm::string_view path::get_filename_fragment(filename_fragment fragment) const
{
  auto file = this->get_filename();

  if (file.empty() || file == "." || file == "..") {
    return fragment == filename_fragment::stem ? file : cm::string_view{};
  }

  auto pos = file.find_last_of('.');
  if (pos == cm::string_view::npos || pos == 0) {
    return fragment == filename_fragment::stem ? file : cm::string_view{};
  }
  return fragment == filename_fragment::stem ? file.substr(0, pos)
                                             : file.substr(pos);
}

int path::compare_path(cm::string_view str) const
{
  internals::path_parser this_pp(this->path_);
  ++this_pp;
  internals::path_parser other_pp(str);
  ++other_pp;

  // compare root_name part
  {
    bool compare_root_names = false;
    cm::string_view this_root_name, other_root_name;
    int res;

    if (this_pp.in_root_name()) {
      compare_root_names = true;
      this_root_name = *this_pp;
      ++this_pp;
    }
    if (other_pp.in_root_name()) {
      compare_root_names = true;
      other_root_name = *other_pp;
      ++other_pp;
    }
    if (compare_root_names &&
        (res = this_root_name.compare(other_root_name) != 0)) {
      return res;
    }
  }

  // compare root_directory part
  {
    if (!this_pp.in_root_directory() && other_pp.in_root_directory()) {
      return -1;
    } else if (this_pp.in_root_directory() && !other_pp.in_root_directory()) {
      return 1;
    }
    if (this_pp.in_root_directory()) {
      ++this_pp;
    }
    if (other_pp.in_root_directory()) {
      ++other_pp;
    }
  }

  // compare various parts of the paths
  while (!this_pp.at_end() && !other_pp.at_end()) {
    int res;
    if ((res = (*this_pp).compare(*other_pp)) != 0) {
      return res;
    }
    ++this_pp;
    ++other_pp;
  }

  // final step
  if (this_pp.at_end() && !other_pp.at_end()) {
    return -1;
  } else if (!this_pp.at_end() && other_pp.at_end()) {
    return 1;
  }

  return 0;
}

// Class path::iterator
path::iterator::iterator()
  : path_(nullptr)
{
}
path::iterator::iterator(const iterator& other)
{
  this->path_ = other.path_;
  if (other.parser_) {
    this->parser_ = cm::make_unique<internals::path_parser>(*other.parser_);
    this->path_element_ = path(**this->parser_);
  }
}
path::iterator::iterator(const path* p, bool at_end)
  : path_(p)
  , parser_(cm::make_unique<internals::path_parser>(p->path_, at_end))
{
  if (!at_end) {
    ++(*this->parser_);
    this->path_element_ = path(**this->parser_);
  }
}

path::iterator::~iterator() = default;

path::iterator& path::iterator::operator=(const iterator& other)
{
  this->path_ = other.path_;
  if (other.parser_) {
    this->parser_ = cm::make_unique<internals::path_parser>(*other.parser_);
    this->path_element_ = path(**this->parser_);
  }

  return *this;
}

path::iterator& path::iterator::operator++()
{
  assert(this->parser_);

  if (this->parser_) {
    assert(!this->parser_->at_end());

    if (!this->parser_->at_end()) {
      ++(*this->parser_);
      if (this->parser_->at_end()) {
        this->path_element_ = path();
      } else {
        this->path_element_ = path(**this->parser_);
      }
    }
  }

  return *this;
}

path::iterator& path::iterator::operator--()
{
  assert(this->parser_);

  if (this->parser_) {
    assert(!this->parser_->at_start());

    if (!this->parser_->at_start()) {
      --(*this->parser_);
      this->path_element_ = path(**this->parser_);
    }
  }

  return *this;
}

bool operator==(const path::iterator& lhs, const path::iterator& rhs)
{
  return lhs.path_ == rhs.path_ && lhs.parser_ != nullptr &&
    ((lhs.parser_->at_end() && rhs.parser_->at_end()) ||
     (lhs.parser_->at_start() && rhs.parser_->at_start()) ||
     ((**lhs.parser_).data() == (**rhs.parser_).data()));
}

std::size_t hash_value(const path& p) noexcept
{
  internals::path_parser parser(p.path_);
  std::hash<cm::string_view> hasher;
  std::size_t value = 0;

  while (!parser.at_end()) {
    value = hasher(*parser) + 0x9e3779b9 + (value << 6) + (value >> 2);
    ++parser;
  }

  return value;
}
} // filesystem
} // cm

#else

// Avoid empty translation unit.
void cm_filesystem_path_cxx()
{
}

#endif
