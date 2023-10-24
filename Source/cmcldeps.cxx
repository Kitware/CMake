// Copyright 2011 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Wrapper around cl that adds /showIncludes to command line, and uses that to
// generate .d files that match the style from gcc -MD.
//
// /showIncludes is equivalent to -MD, not -MMD, that is, system headers are
// included.

#include <algorithm>
#include <sstream>

#include <windows.h>

#include "cmsys/Encoding.hxx"

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// We don't want any wildcard expansion.
// See http://msdn.microsoft.com/en-us/library/zay8tzh6(v=vs.85).aspx
void _setargv()
{
}

static void Fatal(const char* msg, ...)
{
  va_list ap;
  fprintf(stderr, "ninja: FATAL: ");
  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  // On Windows, some tools may inject extra threads.
  // exit() may block on locks held by those threads, so forcibly exit.
  fflush(stderr);
  fflush(stdout);
  ExitProcess(1);
}

static void usage(const char* msg)
{
  Fatal("%s\n\nusage:\n    "
        "cmcldeps "
        "<language C, CXX or RC>  "
        "<source file path>  "
        "<output path for *.d file>  "
        "<output path for *.obj file>  "
        "<prefix of /showIncludes>  "
        "<path to cl.exe>  "
        "<path to tool (cl or rc)>  "
        "<rest of command ...>\n",
        msg);
}

static cm::string_view trimLeadingSpace(cm::string_view cmdline)
{
  int i = 0;
  for (; cmdline[i] == ' '; ++i)
    ;
  return cmdline.substr(i);
}

static void replaceAll(std::string& str, const std::string& search,
                       const std::string& repl)
{
  std::string::size_type pos = 0;
  while ((pos = str.find(search, pos)) != std::string::npos) {
    str.replace(pos, search.size(), repl);
    pos += repl.size();
  }
}

// Strips one argument from the cmdline and returns it. "surrounding quotes"
// are removed from the argument if there were any.
static std::string getArg(std::string& cmdline)
{
  bool in_quoted = false;
  unsigned int i = 0;

  cm::string_view cmdview = trimLeadingSpace(cmdline);
  size_t spaceCnt = cmdline.size() - cmdview.size();

  for (;; ++i) {
    if (i >= cmdview.size())
      usage("Couldn't parse arguments.");
    if (!in_quoted && cmdview[i] == ' ')
      break; // "a b" "x y"
    if (cmdview[i] == '"')
      in_quoted = !in_quoted;
  }

  cmdview = cmdview.substr(0, i);
  if (cmdview[0] == '"' && cmdview[i - 1] == '"')
    cmdview = cmdview.substr(1, i - 2);
  std::string ret(cmdview);
  cmdline.erase(0, spaceCnt + i);
  return ret;
}

static void parseCommandLine(LPWSTR wincmdline, std::string& lang,
                             std::string& srcfile, std::string& dfile,
                             std::string& objfile, std::string& prefix,
                             std::string& clpath, std::string& binpath,
                             std::string& rest)
{
  std::string cmdline = cmsys::Encoding::ToNarrow(wincmdline);
  /* self */ getArg(cmdline);
  lang = getArg(cmdline);
  srcfile = getArg(cmdline);
  dfile = getArg(cmdline);
  objfile = getArg(cmdline);
  prefix = getArg(cmdline);
  clpath = getArg(cmdline);
  binpath = getArg(cmdline);
  rest = std::string(trimLeadingSpace(cmdline));
}

// Not all backslashes need to be escaped in a depfile, but it's easier that
// way.  See the re2c grammar in ninja's source code for more info.
static void escapePath(std::string& path)
{
  replaceAll(path, "\\", "\\\\");
  replaceAll(path, " ", "\\ ");
}

static void outputDepFile(const std::string& dfile, const std::string& objfile,
                          std::vector<std::string>& incs)
{

  if (dfile.empty())
    return;

  // strip duplicates
  std::sort(incs.begin(), incs.end());
  incs.erase(std::unique(incs.begin(), incs.end()), incs.end());

  FILE* out = cmsys::SystemTools::Fopen(dfile.c_str(), "wb");

  // FIXME should this be fatal or not? delete obj? delete d?
  if (!out)
    return;
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  replaceAll(cwd, "/", "\\");
  cwd += "\\";

  std::string tmp = objfile;
  escapePath(tmp);
  fprintf(out, "%s: \\\n", tmp.c_str());

  std::vector<std::string>::iterator it = incs.begin();
  for (; it != incs.end(); ++it) {
    tmp = *it;
    // The paths need to match the ones used to identify build artifacts in the
    // build.ninja file.  Therefore we need to canonicalize the path to use
    // backward slashes and relativize the path to the build directory.
    replaceAll(tmp, "/", "\\");
    if (cmHasPrefix(tmp, cwd))
      tmp.erase(0, cwd.size());
    escapePath(tmp);
    fprintf(out, "%s \\\n", tmp.c_str());
  }

  fprintf(out, "\n");
  fclose(out);
}

static int process(cm::string_view srcfilename, const std::string& dfile,
                   const std::string& objfile, const std::string& prefix,
                   const std::string& cmd, const std::string& dir = "",
                   bool quiet = false)
{
  std::string output;
  // break up command line into a vector
  std::vector<std::string> args;
  cmSystemTools::ParseWindowsCommandLine(cmd.c_str(), args);
  // convert to correct vector type for RunSingleCommand
  std::vector<std::string> command;
  for (std::vector<std::string>::iterator i = args.begin(); i != args.end();
       ++i) {
    command.push_back(*i);
  }
  // run the command
  int exit_code = 0;
  bool run =
    cmSystemTools::RunSingleCommand(command, &output, &output, &exit_code,
                                    dir.c_str(), cmSystemTools::OUTPUT_NONE);

  // process the include directives and output everything else
  std::istringstream ss(output);
  std::string line;
  std::vector<std::string> includes;
  bool isFirstLine = true; // cl prints always first the source filename
  while (std::getline(ss, line)) {
    cm::string_view inc(line);
    if (cmHasPrefix(inc, prefix)) {
      inc = trimLeadingSpace(inc.substr(prefix.size()));
      if (inc.back() == '\r') // blech, stupid \r\n
        inc = inc.substr(0, inc.size() - 1);
      includes.emplace_back(std::string(inc));
    } else {
      if (!isFirstLine || !cmHasPrefix(inc, srcfilename)) {
        if (!quiet || exit_code != 0) {
          fprintf(stdout, "%s\n", line.c_str());
        }
      } else {
        isFirstLine = false;
      }
    }
  }

  // don't update .d until/unless we succeed compilation
  if (run && exit_code == 0)
    outputDepFile(dfile, objfile, includes);

  return exit_code;
}

int main()
{

  // Use the Win32 API instead of argc/argv so we can avoid interpreting the
  // rest of command line after the .d and .obj. Custom parsing seemed
  // preferable to the ugliness you get into in trying to re-escape quotes for
  // subprocesses, so by avoiding argc/argv, the subprocess is called with
  // the same command line verbatim.

  std::string lang, srcfile, dfile, objfile, prefix, cl, binpath, rest;
  parseCommandLine(GetCommandLineW(), lang, srcfile, dfile, objfile, prefix,
                   cl, binpath, rest);

  // needed to suppress filename output of msvc tools
  cm::string_view srcfilename(srcfile);
  std::string::size_type pos = srcfile.rfind('\\');
  if (pos != std::string::npos) {
    srcfilename = srcfilename.substr(pos + 1);
  }

  if (lang == "RC") {
    // "misuse" cl.exe to get headers from .rc files

    // Make sure there is at most one /nologo option.
    bool const haveNologo = (rest.find("/nologo ") != std::string::npos ||
                             rest.find("-nologo ") != std::string::npos);
    cmSystemTools::ReplaceString(rest, "-nologo ", " ");
    cmSystemTools::ReplaceString(rest, "/nologo ", " ");
    std::string clrest = rest;
    if (haveNologo) {
      rest = "/nologo " + rest;
    }

    // rc /fo X.dir\x.rc.res  =>  cl -FoX.dir\x.rc.res.obj
    // The object will not actually be written.
    cmSystemTools::ReplaceString(clrest, "/fo ", " ");
    cmSystemTools::ReplaceString(clrest, "-fo ", " ");
    cmSystemTools::ReplaceString(clrest, objfile, "-Fo" + objfile + ".obj");

    cl = "\"" + cl + "\" /P /DRC_INVOKED /nologo /showIncludes /TC ";

    // call cl in object dir so the .i is generated there
    std::string objdir;
    {
      pos = objfile.rfind("\\");
      if (pos != std::string::npos) {
        objdir = objfile.substr(0, pos);
      }
    }

    // extract dependencies with cl.exe
    int exit_code =
      process(srcfilename, dfile, objfile, prefix, cl + clrest, objdir, true);

    if (exit_code != 0)
      return exit_code;

    // compile rc file with rc.exe
    std::string rc = cmStrCat('"', binpath, '"');
    return process(srcfilename, "", objfile, prefix, cmStrCat(rc, ' ', rest),
                   std::string(), true);
  }

  usage("Invalid language specified.");
  return 1;
}
