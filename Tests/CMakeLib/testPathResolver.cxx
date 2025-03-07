/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <cmConfigure.h> // IWYU pragma: keep

#include <cerrno>
#include <map>
#include <string>
#include <utility>

#ifdef _WIN32
#  include <cctype>
#endif

#include <cmsys/Status.hxx>

#include "cmPathResolver.h"

#if defined(_WIN32) || defined(__APPLE__)
#  include "cmSystemTools.h"
#endif

#include "testCommon.h"

// IWYU pragma: no_forward_declare cm::PathResolver::Policies::LogicalPath
// IWYU pragma: no_forward_declare cm::PathResolver::Policies::NaivePath
// IWYU pragma: no_forward_declare cm::PathResolver::Policies::CasePath
// IWYU pragma: no_forward_declare cm::PathResolver::Policies::RealPath

namespace {

class MockSystem : public cm::PathResolver::System
{
public:
  ~MockSystem() override = default;

  struct Path
  {
    std::string Name;
    std::string Link;
  };

  std::map<std::string, Path> Paths;

  void SetPaths(std::map<std::string, Path> paths)
  {
    this->Paths = std::move(paths);
  }

  static std::string AdjustCase(std::string const& path)
  {
#if defined(_WIN32) || defined(__APPLE__)
    return cmSystemTools::LowerCase(path);
#else
    return path;
#endif
  }

  cmsys::Status ReadSymlink(std::string const& path,
                            std::string& link) override
  {
    auto i = this->Paths.find(AdjustCase(path));
    if (i == this->Paths.end()) {
      return cmsys::Status::POSIX(ENOENT);
    }
    if (i->second.Link.empty()) {
      return cmsys::Status::POSIX(EINVAL);
    }
    link = i->second.Link;
    return cmsys::Status::Success();
  }

  bool PathExists(std::string const& path) override
  {
    return this->Paths.find(AdjustCase(path)) != this->Paths.end();
  }

  std::string WorkDir;

  void SetWorkDir(std::string wd) { this->WorkDir = std::move(wd); }

  std::string GetWorkingDirectory() override { return this->WorkDir; }

#ifdef _WIN32
  std::map<char, std::string> WorkDirOnDrive;

  void SetWorkDirOnDrive(std::map<char, std::string> wd)
  {
    this->WorkDirOnDrive = std::move(wd);
  }

  std::string GetWorkingDirectoryOnDrive(char letter) override
  {
    std::string result;
    auto i = this->WorkDirOnDrive.find(std::tolower(letter));
    if (i != this->WorkDirOnDrive.end()) {
      result = i->second;
    }
    return result;
  }
#endif

#if defined(_WIN32) || defined(__APPLE__)
  cmsys::Status ReadName(std::string const& path, std::string& name) override
  {
    auto i = this->Paths.find(AdjustCase(path));
    if (i == this->Paths.end()) {
      return cmsys::Status::POSIX(ENOENT);
    }
    name = i->second.Name;
    return cmsys::Status::Success();
  }
#endif
};

#define EXPECT_RESOLVE(_in, _expect)                                          \
  do {                                                                        \
    std::string out;                                                          \
    ASSERT_TRUE(r.Resolve(_in, out));                                         \
    ASSERT_EQUAL(out, _expect);                                               \
  } while (false)

#define EXPECT_ENOENT(_in, _expect)                                           \
  do {                                                                        \
    std::string out;                                                          \
    ASSERT_EQUAL(r.Resolve(_in, out).GetPOSIX(), ENOENT);                     \
    ASSERT_EQUAL(out, _expect);                                               \
  } while (false)

using namespace cm::PathResolver;

bool posixRoot()
{
  std::cout << "posixRoot()\n";
  MockSystem os;
  os.SetPaths({
    { "/", { {}, {} } },
  });
  Resolver<Policies::RealPath> const r(os);
  EXPECT_RESOLVE("/", "/");
  EXPECT_RESOLVE("//", "/");
  EXPECT_RESOLVE("/.", "/");
  EXPECT_RESOLVE("/./", "/");
  EXPECT_RESOLVE("/..", "/");
  EXPECT_RESOLVE("/../", "/");
  return true;
}

bool posixAbsolutePath()
{
  std::cout << "posixAbsolutePath()\n";
  MockSystem os;
  os.SetPaths({
    { "/", { {}, {} } },
    { "/a", { {}, {} } },
  });
  Resolver<Policies::RealPath> const r(os);
  EXPECT_RESOLVE("/a", "/a");
  EXPECT_RESOLVE("/a/", "/a");
  EXPECT_RESOLVE("/a//", "/a");
  EXPECT_RESOLVE("/a/.", "/a");
  EXPECT_RESOLVE("/a/./", "/a");
  EXPECT_RESOLVE("/a/..", "/");
  EXPECT_RESOLVE("/a/../", "/");
  EXPECT_RESOLVE("/a/../..", "/");
#ifndef _WIN32
  EXPECT_RESOLVE("//a", "/a");
#endif
  return true;
}

bool posixWorkingDirectory()
{
  std::cout << "posixWorkingDirectory()\n";
  MockSystem os;
  os.SetPaths({
    { "/", { {}, {} } },
    { "/a", { {}, {} } },
    { "/cwd", { {}, {} } },
    { "/cwd/a", { {}, {} } },
  });
  Resolver<Policies::RealPath> const r(os);
  EXPECT_RESOLVE("", "/");
  EXPECT_RESOLVE(".", "/");
  EXPECT_RESOLVE("..", "/");
  EXPECT_RESOLVE("a", "/a");
  os.SetWorkDir("/cwd");
  EXPECT_RESOLVE("", "/cwd");
  EXPECT_RESOLVE(".", "/cwd");
  EXPECT_RESOLVE("..", "/");
  EXPECT_RESOLVE("a", "/cwd/a");
  return true;
}

bool posixSymlink()
{
  std::cout << "posixSymlink()\n";
  MockSystem os;
  os.SetPaths({
    { "/", { {}, {} } },
    { "/link-a", { {}, "a" } },
    { "/link-a-excess", { {}, "a//." } },
    { "/link-broken", { {}, "link-broken-dest" } },
    { "/a", { {}, {} } },
    { "/a/b", { {}, {} } },
    { "/a/link-b", { {}, "b" } },
    { "/a/b/link-c", { {}, "c" } },
    { "/a/b/c", { {}, {} } },
    { "/a/b/c/link-..|..", { {}, "../.." } },
    { "/a/link-|1|2", { {}, "/1/2" } },
    { "/1", { {}, {} } },
    { "/1/2", { {}, {} } },
    { "/1/2/3", { {}, {} } },
  });

  {
    Resolver<Policies::CasePath> const r(os);
    EXPECT_RESOLVE("/link-a", "/link-a");
    EXPECT_RESOLVE("/link-a-excess", "/link-a-excess");
    EXPECT_RESOLVE("/link-a-excess/b", "/link-a-excess/b");
    EXPECT_RESOLVE("/link-broken", "/link-broken");
    EXPECT_RESOLVE("/link-a/../missing", "/missing");
    EXPECT_RESOLVE("/a/b/link-c", "/a/b/link-c");
    EXPECT_RESOLVE("/a/link-b/c", "/a/link-b/c");
    EXPECT_RESOLVE("/a/link-b/link-c/..", "/a/link-b");
    EXPECT_RESOLVE("/a/b/c/link-..|..", "/a/b/c/link-..|..");
    EXPECT_RESOLVE("/a/b/c/link-..|../link-b", "/a/b/c/link-..|../link-b");
    EXPECT_RESOLVE("/a/link-|1|2/3", "/a/link-|1|2/3");
    EXPECT_RESOLVE("/a/link-|1|2/../2/3", "/a/2/3");
  }

  {
    Resolver<Policies::LogicalPath> const r(os);
    EXPECT_RESOLVE("/link-a", "/link-a");
    EXPECT_RESOLVE("/link-a-excess", "/link-a-excess");
    EXPECT_RESOLVE("/link-a-excess/b", "/link-a-excess/b");
    EXPECT_RESOLVE("/link-broken", "/link-broken");
    EXPECT_RESOLVE("/link-a/../missing", "/missing");
    EXPECT_RESOLVE("/a/b/link-c", "/a/b/link-c");
    EXPECT_RESOLVE("/a/link-b/c", "/a/link-b/c");
    EXPECT_RESOLVE("/a/link-b/link-c/..", "/a/link-b");
    EXPECT_RESOLVE("/a/b/c/link-..|..", "/a/b/c/link-..|..");
    EXPECT_RESOLVE("/a/b/c/link-..|../link-b", "/a/b/c/link-..|../link-b");
    EXPECT_RESOLVE("/a/link-|1|2/3", "/a/link-|1|2/3");
    EXPECT_RESOLVE("/a/link-|1|2/../2/3", "/1/2/3");
  }

  {
    Resolver<Policies::RealPath> const r(os);
    EXPECT_RESOLVE("/link-a", "/a");
    EXPECT_RESOLVE("/link-a-excess", "/a");
    EXPECT_RESOLVE("/link-a-excess/b", "/a/b");
    EXPECT_ENOENT("/link-broken", "/link-broken-dest");
    EXPECT_ENOENT("/link-a/../missing", "/missing");
    EXPECT_RESOLVE("/a/b/link-c", "/a/b/c");
    EXPECT_RESOLVE("/a/link-b/c", "/a/b/c");
    EXPECT_RESOLVE("/a/link-b/link-c/..", "/a/b");
    EXPECT_RESOLVE("/a/b/c/link-..|..", "/a");
    EXPECT_RESOLVE("/a/b/c/link-..|../link-b", "/a/b");
    EXPECT_RESOLVE("/a/link-|1|2/3", "/1/2/3");
  }

  return true;
}

#ifdef __APPLE__
bool macosActualCase()
{
  std::cout << "macosActualCase()\n";
  MockSystem os;
  os.SetPaths({
    { "/", { {}, {} } },
    { "/mixed", { "MiXeD", {} } },
    { "/mixed/link-mixed", { "LiNk-MiXeD", "mixed" } },
    { "/mixed/mixed", { "MiXeD", {} } },
    { "/mixed/link-c-mixed", { "LiNk-C-MiXeD", "/mIxEd" } },
    { "/upper", { "UPPER", {} } },
    { "/upper/link-upper", { "LINK-UPPER", "upper" } },
    { "/upper/upper", { "UPPER", {} } },
    { "/upper/link-c-upper", { "LINK-C-UPPER", "/upper" } },
  });

  {
    Resolver<Policies::CasePath> const r(os);
    EXPECT_RESOLVE("/mIxEd/MiSsInG", "/MiXeD/MiSsInG");
    EXPECT_RESOLVE("/mIxEd/link-MiXeD", "/MiXeD/LiNk-MiXeD");
    EXPECT_RESOLVE("/mIxEd/link-c-MiXeD", "/MiXeD/LiNk-C-MiXeD");
    EXPECT_RESOLVE("/upper/mIsSiNg", "/UPPER/mIsSiNg");
    EXPECT_RESOLVE("/upper/link-upper", "/UPPER/LINK-UPPER");
    EXPECT_RESOLVE("/upper/link-c-upper", "/UPPER/LINK-C-UPPER");
  }

  {
    Resolver<Policies::LogicalPath> const r(os);
    EXPECT_RESOLVE("/mIxEd/MiSsInG", "/MiXeD/MiSsInG");
    EXPECT_RESOLVE("/mIxEd/link-MiXeD", "/MiXeD/LiNk-MiXeD");
    EXPECT_RESOLVE("/mIxEd/link-c-MiXeD", "/MiXeD/LiNk-C-MiXeD");
    EXPECT_RESOLVE("/upper/mIsSiNg", "/UPPER/mIsSiNg");
    EXPECT_RESOLVE("/upper/link-upper", "/UPPER/LINK-UPPER");
    EXPECT_RESOLVE("/upper/link-c-upper", "/UPPER/LINK-C-UPPER");
  }

  {
    Resolver<Policies::RealPath> const r(os);
    EXPECT_ENOENT("/mIxEd/MiSsInG", "/MiXeD/MiSsInG");
    EXPECT_RESOLVE("/mIxEd/link-MiXeD", "/MiXeD/MiXeD");
    EXPECT_RESOLVE("/mIxEd/link-c-MiXeD", "/MiXeD");
    EXPECT_ENOENT("/upper/mIsSiNg", "/UPPER/mIsSiNg");
    EXPECT_RESOLVE("/upper/link-upper", "/UPPER/UPPER");
    EXPECT_RESOLVE("/upper/link-c-upper", "/UPPER");
  }

  return true;
}
#endif

#ifdef _WIN32
bool windowsRoot()
{
  std::cout << "windowsRoot()\n";
  MockSystem os;
  {
    Resolver<Policies::NaivePath> const r(os);
    EXPECT_RESOLVE("c:/", "c:/");
    EXPECT_RESOLVE("C:/", "C:/");
    EXPECT_RESOLVE("c://", "c:/");
    EXPECT_RESOLVE("C:/.", "C:/");
    EXPECT_RESOLVE("c:/./", "c:/");
    EXPECT_RESOLVE("C:/..", "C:/");
    EXPECT_RESOLVE("c:/../", "c:/");
  }
  {
    Resolver<Policies::CasePath> const r(os);
    EXPECT_RESOLVE("c:/", "C:/");
    EXPECT_RESOLVE("C:/", "C:/");
    EXPECT_RESOLVE("c://", "C:/");
    EXPECT_RESOLVE("C:/.", "C:/");
    EXPECT_RESOLVE("c:/./", "C:/");
    EXPECT_RESOLVE("C:/..", "C:/");
    EXPECT_RESOLVE("c:/../", "C:/");
  }
  os.SetPaths({
    { "c:/", { {}, {} } },
    { "//host/", { {}, {} } },
  });
  {
    Resolver<Policies::RealPath> const r(os);
    EXPECT_RESOLVE("c:/", "C:/");
    EXPECT_RESOLVE("C:/", "C:/");
    EXPECT_RESOLVE("c://", "C:/");
    EXPECT_RESOLVE("C:/.", "C:/");
    EXPECT_RESOLVE("c:/./", "C:/");
    EXPECT_RESOLVE("C:/..", "C:/");
    EXPECT_RESOLVE("c:/../", "C:/");
    EXPECT_RESOLVE("//host", "//host/");
    EXPECT_RESOLVE("//host/.", "//host/");
    EXPECT_RESOLVE("//host/./", "//host/");
    EXPECT_RESOLVE("//host/..", "//host/");
    EXPECT_RESOLVE("//host/../", "//host/");
  }
  return true;
}

bool windowsAbsolutePath()
{
  std::cout << "windowsAbsolutePath()\n";
  MockSystem os;
  os.SetPaths({
    { "c:/", { {}, {} } },
    { "c:/a", { {}, {} } },
  });
  Resolver<Policies::RealPath> const r(os);
  EXPECT_RESOLVE("c:/a", "C:/a");
  EXPECT_RESOLVE("c:/a/", "C:/a");
  EXPECT_RESOLVE("c:/a//", "C:/a");
  EXPECT_RESOLVE("c:/a/.", "C:/a");
  EXPECT_RESOLVE("c:/a/./", "C:/a");
  EXPECT_RESOLVE("c:/a/..", "C:/");
  EXPECT_RESOLVE("c:/a/../", "C:/");
  EXPECT_RESOLVE("c:/a/../..", "C:/");
  return true;
}

bool windowsActualCase()
{
  std::cout << "windowsActualCase()\n";
  MockSystem os;
  os.SetPaths({
    { "c:/", { {}, {} } },
    { "c:/mixed", { "MiXeD", {} } },
    { "c:/mixed/link-mixed", { "LiNk-MiXeD", "mixed" } },
    { "c:/mixed/mixed", { "MiXeD", {} } },
    { "c:/mixed/link-c-mixed", { "LiNk-C-MiXeD", "C:/mIxEd" } },
    { "c:/upper", { "UPPER", {} } },
    { "c:/upper/link-upper", { "LINK-UPPER", "upper" } },
    { "c:/upper/upper", { "UPPER", {} } },
    { "c:/upper/link-c-upper", { "LINK-C-UPPER", "c:/upper" } },
  });

  {
    Resolver<Policies::CasePath> const r(os);
    EXPECT_RESOLVE("c:/mIxEd/MiSsInG", "C:/MiXeD/MiSsInG");
    EXPECT_RESOLVE("c:/mIxEd/link-MiXeD", "C:/MiXeD/LiNk-MiXeD");
    EXPECT_RESOLVE("c:/mIxEd/link-c-MiXeD", "C:/MiXeD/LiNk-C-MiXeD");
    EXPECT_RESOLVE("c:/upper/mIsSiNg", "C:/UPPER/mIsSiNg");
    EXPECT_RESOLVE("c:/upper/link-upper", "C:/UPPER/LINK-UPPER");
    EXPECT_RESOLVE("c:/upper/link-c-upper", "C:/UPPER/LINK-C-UPPER");
  }

  {
    Resolver<Policies::LogicalPath> const r(os);
    EXPECT_RESOLVE("c:/mIxEd/MiSsInG", "C:/MiXeD/MiSsInG");
    EXPECT_RESOLVE("c:/mIxEd/link-MiXeD", "C:/MiXeD/LiNk-MiXeD");
    EXPECT_RESOLVE("c:/mIxEd/link-c-MiXeD", "C:/MiXeD/LiNk-C-MiXeD");
    EXPECT_RESOLVE("c:/upper/mIsSiNg", "C:/UPPER/mIsSiNg");
    EXPECT_RESOLVE("c:/upper/link-upper", "C:/UPPER/LINK-UPPER");
    EXPECT_RESOLVE("c:/upper/link-c-upper", "C:/UPPER/LINK-C-UPPER");
  }

  {
    Resolver<Policies::RealPath> const r(os);
    EXPECT_ENOENT("c:/mIxEd/MiSsInG", "C:/MiXeD/MiSsInG");
    EXPECT_RESOLVE("c:/mIxEd/link-MiXeD", "C:/MiXeD/MiXeD");
    EXPECT_RESOLVE("c:/mIxEd/link-c-MiXeD", "C:/MiXeD");
    EXPECT_ENOENT("c:/upper/mIsSiNg", "C:/UPPER/mIsSiNg");
    EXPECT_RESOLVE("c:/upper/link-upper", "C:/UPPER/UPPER");
    EXPECT_RESOLVE("c:/upper/link-c-upper", "C:/UPPER");
  }

  return true;
}

bool windowsWorkingDirectory()
{
  std::cout << "windowsWorkingDirectory()\n";
  MockSystem os;
  os.SetPaths({
    { "c:/", { {}, {} } },
    { "c:/a", { {}, {} } },
    { "c:/cwd", { {}, {} } },
    { "c:/cwd/a", { {}, {} } },
  });
  {
    Resolver<Policies::LogicalPath> const r(os);
    EXPECT_RESOLVE("", "/");
    EXPECT_RESOLVE(".", "/");
    EXPECT_RESOLVE("..", "/");
    EXPECT_RESOLVE("a", "/a");
  }
  {
    Resolver<Policies::RealPath> const r(os);
    os.SetWorkDir("c:/cwd");
    EXPECT_RESOLVE("", "C:/cwd");
    EXPECT_RESOLVE(".", "C:/cwd");
    EXPECT_RESOLVE("..", "C:/");
    EXPECT_RESOLVE("a", "C:/cwd/a");
    EXPECT_ENOENT("missing", "C:/cwd/missing");
  }
  return true;
}

bool windowsWorkingDirectoryOnDrive()
{
  std::cout << "windowsWorkingDirectoryOnDrive()\n";
  MockSystem os;
  os.SetWorkDir("c:/cwd");
  os.SetWorkDirOnDrive({
    { 'd', "d:/cwd-d" },
  });
  {
    Resolver<Policies::NaivePath> const r(os);
    EXPECT_RESOLVE("c:", "c:/cwd");
    EXPECT_RESOLVE("c:.", "c:/cwd");
    EXPECT_RESOLVE("c:..", "c:/");
    EXPECT_RESOLVE("C:", "C:/cwd");
    EXPECT_RESOLVE("C:.", "C:/cwd");
    EXPECT_RESOLVE("C:..", "C:/");
    EXPECT_RESOLVE("d:", "d:/cwd-d");
    EXPECT_RESOLVE("d:.", "d:/cwd-d");
    EXPECT_RESOLVE("d:..", "d:/");
    EXPECT_RESOLVE("D:", "D:/cwd-d");
    EXPECT_RESOLVE("D:.", "D:/cwd-d");
    EXPECT_RESOLVE("D:..", "D:/");
    EXPECT_RESOLVE("e:", "e:/");
    EXPECT_RESOLVE("e:.", "e:/");
    EXPECT_RESOLVE("e:..", "e:/");
    EXPECT_RESOLVE("E:", "E:/");
    EXPECT_RESOLVE("E:.", "E:/");
    EXPECT_RESOLVE("E:..", "E:/");
  }
  {
    Resolver<Policies::CasePath> const r(os);
    EXPECT_RESOLVE("c:", "C:/cwd");
    EXPECT_RESOLVE("c:.", "C:/cwd");
    EXPECT_RESOLVE("c:..", "C:/");
    EXPECT_RESOLVE("C:", "C:/cwd");
    EXPECT_RESOLVE("C:.", "C:/cwd");
    EXPECT_RESOLVE("C:..", "C:/");
    EXPECT_RESOLVE("d:", "D:/cwd-d");
    EXPECT_RESOLVE("d:.", "D:/cwd-d");
    EXPECT_RESOLVE("d:..", "D:/");
    EXPECT_RESOLVE("D:", "D:/cwd-d");
    EXPECT_RESOLVE("D:.", "D:/cwd-d");
    EXPECT_RESOLVE("D:..", "D:/");
    EXPECT_RESOLVE("e:", "E:/");
    EXPECT_RESOLVE("e:.", "E:/");
    EXPECT_RESOLVE("e:..", "E:/");
    EXPECT_RESOLVE("E:", "E:/");
    EXPECT_RESOLVE("E:.", "E:/");
    EXPECT_RESOLVE("E:..", "E:/");
  }
  os.SetPaths({
    { "c:/", { {}, {} } },
    { "c:/cwd", { {}, {} } },
    { "c:/cwd/existing", { {}, {} } },
    { "d:/", { {}, {} } },
    { "d:/cwd-d", { {}, {} } },
    { "d:/cwd-d/existing", { {}, {} } },
    { "e:/", { {}, {} } },
  });
  {
    Resolver<Policies::RealPath> const r(os);
    EXPECT_RESOLVE("c:existing", "C:/cwd/existing");
    EXPECT_ENOENT("c:missing", "C:/cwd/missing");
    EXPECT_RESOLVE("C:existing", "C:/cwd/existing");
    EXPECT_ENOENT("C:missing", "C:/cwd/missing");
    EXPECT_RESOLVE("d:existing", "D:/cwd-d/existing");
    EXPECT_ENOENT("d:missing", "D:/cwd-d/missing");
    EXPECT_ENOENT("e:missing", "E:/missing");
    EXPECT_ENOENT("f:", "F:/");
  }
  return true;
}

bool windowsNetworkShare()
{
  std::cout << "windowsNetworkShare()\n";
  MockSystem os;
  os.SetPaths({
    { "c:/", { {}, {} } },
    { "c:/cwd", { {}, {} } },
    { "c:/cwd/link-to-host-share", { {}, "//host/share" } },
    { "//host/", { {}, {} } },
    { "//host/share", { {}, {} } },
  });
  os.SetWorkDir("c:/cwd");
  {
    Resolver<Policies::RealPath> const r(os);
    EXPECT_RESOLVE("//host/share", "//host/share");
    EXPECT_RESOLVE("//host/share/", "//host/share");
    EXPECT_RESOLVE("//host/share/.", "//host/share");
    EXPECT_RESOLVE("//host/share/./", "//host/share");
    EXPECT_RESOLVE("//host/share/..", "//host/");
    EXPECT_RESOLVE("//host/share/../", "//host/");
    EXPECT_RESOLVE("//host/share/../..", "//host/");
    EXPECT_RESOLVE("link-to-host-share", "//host/share");
    EXPECT_RESOLVE("link-to-host-share/..", "//host/");
    EXPECT_ENOENT("link-to-host-share/../missing", "//host/missing");
  }

  {
    Resolver<Policies::LogicalPath> const r(os);
    EXPECT_RESOLVE("link-to-host-share", "C:/cwd/link-to-host-share");
    EXPECT_RESOLVE("link-to-host-share/..", "//host/");
    EXPECT_RESOLVE("link-to-host-share/../missing", "//host/missing");
  }

  {
    Resolver<Policies::CasePath> const r(os);
    EXPECT_RESOLVE("link-to-host-share", "C:/cwd/link-to-host-share");
    EXPECT_RESOLVE("link-to-host-share/..", "C:/cwd");
    EXPECT_RESOLVE("link-to-host-share/../missing", "C:/cwd/missing");
  }
  return true;
}
#endif

}

int testPathResolver(int /*unused*/, char* /*unused*/[])
{
  return runTests({
    posixRoot,
    posixAbsolutePath,
    posixWorkingDirectory,
    posixSymlink,
#ifdef __APPLE__
    macosActualCase,
#endif
#ifdef _WIN32
    windowsRoot,
    windowsAbsolutePath,
    windowsActualCase,
    windowsWorkingDirectory,
    windowsWorkingDirectoryOnDrive,
    windowsNetworkShare,
#endif
  });
}
