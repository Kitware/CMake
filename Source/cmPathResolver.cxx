/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmPathResolver.h"

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <string>
#include <utility>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#ifdef _WIN32
#  include <cctype>

#  include <windows.h>
#endif

#define MAX_SYMBOLIC_LINKS 32

namespace cm {
namespace PathResolver {

namespace {

namespace Options {

enum class ActualCase
{
  No,
  Yes,
};

enum class Symlinks
{
  None,
  Lazy,
  Eager,
};

enum class Existence
{
  Agnostic,
  Required,
};
}

enum class Root
{
  None,
  POSIX,
#ifdef _WIN32
  Drive,
  Network,
#endif
};

struct Control
{
  enum class Tag
  {
    Continue,
    Restart,
    Error,
  };
  Tag tag;
  union
  {
    std::string::size_type slash; // data for Continue
    cmsys::Status error;          // data for Error
  };
  static Control Continue(std::string::size_type s)
  {
    Control c{ Tag::Continue };
    c.slash = s;
    return c;
  }
  static Control Restart() { return Control{ Tag::Restart }; }
  static Control Error(cmsys::Status e)
  {
    Control c{ Tag::Error };
    c.error = e;
    return c;
  }

private:
  Control(Tag t)
    : tag(t)
  {
  }
};

Root ClassifyRoot(cm::string_view p)
{
#ifdef _WIN32
  if (p.size() >= 2 && std::isalpha(p[0]) && p[1] == ':') {
    return Root::Drive;
  }
  if (p.size() >= 3 && p[0] == '/' && p[1] == '/' && p[2] != '/') {
    return Root::Network;
  }
#endif
  if (!p.empty() && p[0] == '/') {
    return Root::POSIX;
  }
  return Root::None;
}

class ImplBase
{
protected:
  ImplBase(System& os)
    : OS(os)
  {
  }

  System& OS;
  std::string P;
  std::size_t SymlinkDepth = 0;

#ifdef _WIN32
  std::string GetWorkingDirectoryOnDrive(char letter);
  Control ResolveRootRelative();
#endif
  cm::optional<std::string> ReadSymlink(std::string const& path,
                                        cmsys::Status& status);
  Control ResolveSymlink(Root root, std::string::size_type slash,
                         std::string::size_type next_slash,
                         std::string symlink_target);
};

template <class Policy>
class Impl : public ImplBase
{
  Control ResolveRelativePath();
  Control ResolveRoot(Root root);
  Control ResolveComponent(Root root, std::string::size_type root_slash,
                           std::string::size_type slash);
  Control ResolvePath();

public:
  Impl(System& os)
    : ImplBase(os)
  {
  }
  cmsys::Status Resolve(std::string in, std::string& out);
};

template <class Policy>
Control Impl<Policy>::ResolveRelativePath()
{
  // This is a relative path.  Convert it to absolute and restart.
  std::string p = this->OS.GetWorkingDirectory();
  std::replace(p.begin(), p.end(), '\\', '/');
  if (ClassifyRoot(p) == Root::None) {
    p.insert(0, 1, '/');
  }
  if (p.back() != '/') {
    p.push_back('/');
  }
  P.insert(0, p);
  return Control::Restart();
}

#ifdef _WIN32
std::string ImplBase::GetWorkingDirectoryOnDrive(char letter)
{
  // Use the drive's working directory, if any.
  std::string d = this->OS.GetWorkingDirectoryOnDrive(letter);
  std::replace(d.begin(), d.end(), '\\', '/');
  if (d.size() >= 3 && std::toupper(d[0]) == std::toupper(letter) &&
      d[1] == ':' && d[2] == '/') {
    d[0] = letter;
    d.push_back('/');
    return d;
  }

  // Use the current working directory if the drive matches.
  d = this->OS.GetWorkingDirectory();
  if (d.size() >= 3 && std::toupper(d[0]) == std::toupper(letter) &&
      d[1] == ':' && d[2] == '/') {
    d[0] = letter;
    d.push_back('/');
    return d;
  }

  // Fall back to the root directory on the drive.
  d = "_:/";
  d[0] = letter;
  return d;
}

Control ImplBase::ResolveRootRelative()
{
  // This is a root-relative path.  Resolve the root drive and restart.
  P.replace(0, 2, this->GetWorkingDirectoryOnDrive(P[0]));
  return Control::Restart();
}
#endif

cm::optional<std::string> ImplBase::ReadSymlink(std::string const& path,
                                                cmsys::Status& status)
{
  cm::optional<std::string> result;
  std::string target;
  status = this->OS.ReadSymlink(path, target);
  if (status && ++this->SymlinkDepth >= MAX_SYMBOLIC_LINKS) {
    status = cmsys::Status::POSIX(ELOOP);
  }
  if (status) {
    if (!target.empty()) {
      result = std::move(target);
    }
  } else if (status.GetPOSIX() == EINVAL
#ifdef _WIN32
             || status.GetWindows() == ERROR_NOT_A_REPARSE_POINT
#endif
  ) {
    // The path was not a symlink.
    status = cmsys::Status::Success();
  }
  return result;
}

Control ImplBase::ResolveSymlink(Root root, std::string::size_type slash,
                                 std::string::size_type next_slash,
                                 std::string symlink_target)
{
  std::replace(symlink_target.begin(), symlink_target.end(), '\\', '/');
  Root const symlink_target_root = ClassifyRoot(symlink_target);
  if (symlink_target_root == Root::None) {
    // This is a symlink to a relative path.
    // Resolve the symlink, while preserving the leading and
    // trailing (if any) slash:
    //   "*/link/" => "*/dest/"
    //     ^slash       ^slash
    P.replace(slash + 1, next_slash - slash - 1, symlink_target);
    return Control::Continue(slash);
  }

#ifdef _WIN32
  if (root == Root::Drive && symlink_target_root == Root::POSIX) {
    // This is a symlink to a POSIX absolute path,
    // but the current path is on a drive letter.  Resolve the
    // symlink while preserving the drive letter, and start over:
    //   "C:/*/link/" => "C:/dest/"
    //        ^slash      (restart)
    P.replace(2, next_slash - 2, symlink_target);
    return Control::Restart();
  }
#else
  static_cast<void>(root);
#endif

  // This is a symlink to an absolute path.
  // Resolve it and start over:
  //   "*/link/" => "/dest/"
  //     ^slash      (restart)
  P.replace(0, next_slash, symlink_target);
  return Control::Restart();
}

template <class Policy>
Control Impl<Policy>::ResolveRoot(Root root)
{
  if (root == Root::None) {
    return this->ResolveRelativePath();
  }

  // POSIX absolute paths always start with a '/'.
  std::string::size_type root_slash = 0;

#ifdef _WIN32
  if (root == Root::Drive) {
    if (P.size() == 2 || P[2] != '/') {
      return this->ResolveRootRelative();
    }

    if (Policy::ActualCase == Options::ActualCase::Yes) {
      // Normalize the drive letter to upper-case.
      P[0] = static_cast<char>(std::toupper(P[0]));
    }

    // The root is a drive letter.  The root '/' immediately follows.
    root_slash = 2;
  } else if (root == Root::Network) {
    // The root is a network name.  Find the root '/' after it.
    root_slash = P.find('/', 2);
    if (root_slash == std::string::npos) {
      root_slash = P.size();
      P.push_back('/');
    }
  }
#endif

  if (Policy::Existence == Options::Existence::Required
#ifdef _WIN32
      && root != Root::Network
#endif
  ) {
    std::string path = P.substr(0, root_slash + 1);
    if (!this->OS.PathExists(path)) {
      P = std::move(path);
      return Control::Error(cmsys::Status::POSIX(ENOENT));
    }
  }

  return Control::Continue(root_slash);
}

template <class Policy>
Control Impl<Policy>::ResolveComponent(Root root,
                                       std::string::size_type root_slash,
                                       std::string::size_type slash)
{
  // Look for the '/' or end-of-input that ends this component.
  // The sample paths in comments below show the trailing slash
  // even if it is actually beyond the end of the path.
  std::string::size_type next_slash = P.find('/', slash + 1);
  if (next_slash == std::string::npos) {
    next_slash = P.size();
  }
  cm::string_view c =
    cm::string_view(P).substr(slash + 1, next_slash - (slash + 1));

  if (slash == root_slash) {
    if (c.empty() || c == "."_s || c == ".."_s) {
      // This is an empty, '.', or '..' component at the root.
      // Drop the component and its trailing slash, if any,
      // while preserving the root slash:
      //   "//"   => "/"
      //   "/./"  => "/"
      //   "/../" => "/"
      //    ^slash    ^slash
      P.erase(slash + 1, next_slash - slash);
      return Control::Continue(slash);
    }
  } else {
    if (c.empty() || c == "."_s) {
      // This is an empty or '.' component not at the root.
      // Drop the component and its leading slash:
      //   "*//"  => "*/"
      //   "*/./" => "*/"
      //     ^slash    ^slash
      P.erase(slash, next_slash - slash);
      return Control::Continue(slash);
    }

    if (c == ".."_s) {
      // This is a '..' component not at the root.
      // Rewind to the previous component:
      //   "*/prev/../" => "*/prev/../"
      //          ^slash     ^slash
      next_slash = slash;
      slash = P.rfind('/', slash - 1);

      if (Policy::Symlinks == Options::Symlinks::Lazy) {
        cmsys::Status status;
        std::string path = P.substr(0, next_slash);
        if (cm::optional<std::string> maybe_symlink_target =
              this->ReadSymlink(path, status)) {
          return this->ResolveSymlink(root, slash, next_slash,
                                      std::move(*maybe_symlink_target));
        }
        if (!status && Policy::Existence == Options::Existence::Required) {
          P = std::move(path);
          return Control::Error(status);
        }
      }

      // This is not a symlink.
      // Drop the component, the following '..', and its trailing slash,
      // if any, while preserving the (possibly root) leading slash:
      //   "*/dir/../" => "*/"
      //     ^slash         ^slash
      P.erase(slash + 1, next_slash + 3 - slash);
      return Control::Continue(slash);
    }
  }

  // This is a named component.

  if (Policy::Symlinks == Options::Symlinks::Eager) {
    cmsys::Status status;
    std::string path = P.substr(0, next_slash);
    if (cm::optional<std::string> maybe_symlink_target =
          this->ReadSymlink(path, status)) {
      return this->ResolveSymlink(root, slash, next_slash,
                                  std::move(*maybe_symlink_target));
    }
    if (!status && Policy::Existence == Options::Existence::Required) {
      P = std::move(path);
      return Control::Error(status);
    }
  }

#if defined(_WIN32) || defined(__APPLE__)
  bool exists = false;
  if (Policy::ActualCase == Options::ActualCase::Yes) {
    std::string name;
    std::string path = P.substr(0, next_slash);
    if (cmsys::Status status = this->OS.ReadName(path, name)) {
      exists = true;
      if (!name.empty()) {
        // Rename this component:
        //   "*/name/" => "*/Name/"
        //     ^slash       ^slash
        P.replace(slash + 1, next_slash - slash - 1, name);
        next_slash = slash + 1 + name.length();
      }
    } else if (Policy::Existence == Options::Existence::Required) {
      P = std::move(path);
      return Control::Error(status);
    }
  }
#endif

  if (Policy::Existence == Options::Existence::Required
#if defined(_WIN32) || defined(__APPLE__)
      && !exists
#endif
  ) {
    std::string path = P.substr(0, next_slash);
    if (!this->OS.PathExists(path)) {
      P = std::move(path);
      return Control::Error(cmsys::Status::POSIX(ENOENT));
    }
  }

  // Keep this component:
  //   "*/name/" => "*/name/"
  //     ^slash            ^slash
  return Control::Continue(next_slash);
}

template <class Policy>
Control Impl<Policy>::ResolvePath()
{
  Root const root = ClassifyRoot(P);

  // Resolve the root component.  It always ends in a slash.
  Control control = this->ResolveRoot(root);
  if (control.tag != Control::Tag::Continue) {
    return control;
  }
  std::string::size_type const root_slash = control.slash;

  // Resolve later components.  Every iteration that finishes
  // the loop body makes progress either by removing a component
  // or advancing the slash past it.
  for (std::string::size_type slash = root_slash;
       P.size() > root_slash + 1 && slash < P.size();) {
    control = this->ResolveComponent(root, root_slash, slash);
    if (control.tag != Control::Tag::Continue) {
      return control;
    }
    slash = control.slash;
  }
  return Control::Continue(P.size());
}

template <class Policy>
cmsys::Status Impl<Policy>::Resolve(std::string in, std::string& out)
{
  P = std::move(in);
  std::replace(P.begin(), P.end(), '\\', '/');
  for (;;) {
    Control control = this->ResolvePath();
    switch (control.tag) {
      case Control::Tag::Continue:
        out = std::move(P);
        return cmsys::Status::Success();
      case Control::Tag::Restart:
        continue;
      case Control::Tag::Error:
        out = std::move(P);
        return control.error;
    };
  }
}

}

namespace Policies {
struct NaivePath
{
#if defined(_WIN32) || defined(__APPLE__)
  static constexpr Options::ActualCase ActualCase = Options::ActualCase::No;
#endif
  static constexpr Options::Symlinks Symlinks = Options::Symlinks::None;
  static constexpr Options::Existence Existence = Options::Existence::Agnostic;
};
struct CasePath
{
#if defined(_WIN32) || defined(__APPLE__)
  static constexpr Options::ActualCase ActualCase = Options::ActualCase::Yes;
#endif
  static constexpr Options::Symlinks Symlinks = Options::Symlinks::None;
  static constexpr Options::Existence Existence = Options::Existence::Agnostic;
};
struct RealPath
{
#if defined(_WIN32) || defined(__APPLE__)
  static constexpr Options::ActualCase ActualCase = Options::ActualCase::Yes;
#endif
  static constexpr Options::Symlinks Symlinks = Options::Symlinks::Eager;
  static constexpr Options::Existence Existence = Options::Existence::Required;
};
struct LogicalPath
{
#if defined(_WIN32) || defined(__APPLE__)
  static constexpr Options::ActualCase ActualCase = Options::ActualCase::Yes;
#endif
  static constexpr Options::Symlinks Symlinks = Options::Symlinks::Lazy;
  static constexpr Options::Existence Existence = Options::Existence::Agnostic;
};

#if defined(__SUNPRO_CC)
constexpr Options::Symlinks NaivePath::Symlinks;
constexpr Options::Existence NaivePath::Existence;
constexpr Options::Symlinks CasePath::Symlinks;
constexpr Options::Existence CasePath::Existence;
constexpr Options::Symlinks RealPath::Symlinks;
constexpr Options::Existence RealPath::Existence;
constexpr Options::Symlinks LogicalPath::Symlinks;
constexpr Options::Existence LogicalPath::Existence;
#endif
}

template <class Policy>
Resolver<Policy>::Resolver(System& os)
  : OS(os)
{
}
template <class Policy>
cmsys::Status Resolver<Policy>::Resolve(std::string in, std::string& out) const
{
  return Impl<Policy>(OS).Resolve(std::move(in), out);
}

System::System() = default;
System::~System() = default;

template class Resolver<Policies::LogicalPath>;
template class Resolver<Policies::RealPath>;
template class Resolver<Policies::CasePath>;
template class Resolver<Policies::NaivePath>;

}
}
