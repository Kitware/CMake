/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmIncludeDirectoryCommand.h"

#include <algorithm>
#include <set>
#include <utility>

#include <cmext/algorithm>

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

static void GetIncludes(cmMakefile& mf, const std::string& arg,
                        std::vector<std::string>& incs);
static void NormalizeInclude(cmMakefile& mf, std::string& inc);

bool cmIncludeDirectoryCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
  if (args.empty()) {
    return true;
  }

  cmMakefile& mf = status.GetMakefile();

  auto i = args.begin();

  bool before = mf.IsOn("CMAKE_INCLUDE_DIRECTORIES_BEFORE");
  bool system = false;

  if ((*i) == "BEFORE") {
    before = true;
    ++i;
  } else if ((*i) == "AFTER") {
    before = false;
    ++i;
  }

  std::vector<std::string> beforeIncludes;
  std::vector<std::string> afterIncludes;
  std::set<std::string> systemIncludes;

  for (; i != args.end(); ++i) {
    if (*i == "SYSTEM") {
      system = true;
      continue;
    }
    if (i->empty()) {
      status.SetError("given empty-string as include directory.");
      return false;
    }

    std::vector<std::string> includes;

    GetIncludes(mf, *i, includes);

    if (before) {
      cm::append(beforeIncludes, includes);
    } else {
      cm::append(afterIncludes, includes);
    }
    if (system) {
      systemIncludes.insert(includes.begin(), includes.end());
    }
  }
  std::reverse(beforeIncludes.begin(), beforeIncludes.end());

  mf.AddIncludeDirectories(afterIncludes);
  mf.AddIncludeDirectories(beforeIncludes, before);
  mf.AddSystemIncludeDirectories(systemIncludes);

  return true;
}

// do a lot of cleanup on the arguments because this is one place where folks
// sometimes take the output of a program and pass it directly into this
// command not thinking that a single argument could be filled with spaces
// and newlines etc like below:
//
// "   /foo/bar
//    /boo/hoo /dingle/berry "
//
// ideally that should be three separate arguments but when sucking the
// output from a program and passing it into a command the cleanup doesn't
// always happen
//
static void GetIncludes(cmMakefile& mf, const std::string& arg,
                        std::vector<std::string>& incs)
{
  // break apart any line feed arguments
  std::string::size_type pos = 0;
  std::string::size_type lastPos = 0;
  while ((pos = arg.find('\n', lastPos)) != std::string::npos) {
    if (pos) {
      std::string inc = arg.substr(lastPos, pos);
      NormalizeInclude(mf, inc);
      if (!inc.empty()) {
        incs.push_back(std::move(inc));
      }
    }
    lastPos = pos + 1;
  }
  std::string inc = arg.substr(lastPos);
  NormalizeInclude(mf, inc);
  if (!inc.empty()) {
    incs.push_back(std::move(inc));
  }
}

static void NormalizeInclude(cmMakefile& mf, std::string& inc)
{
  std::string::size_type b = inc.find_first_not_of(" \r");
  std::string::size_type e = inc.find_last_not_of(" \r");
  if ((b != std::string::npos) && (e != std::string::npos)) {
    inc.assign(inc, b, 1 + e - b); // copy the remaining substring
  } else {
    inc.clear();
    return;
  }

  if (!cmIsOff(inc)) {
    cmSystemTools::ConvertToUnixSlashes(inc);
    if (!cmSystemTools::FileIsFullPath(inc) &&
        !cmGeneratorExpression::StartsWithGeneratorExpression(inc)) {
      inc = cmStrCat(mf.GetCurrentSourceDirectory(), '/', inc);
    }
  }
}
