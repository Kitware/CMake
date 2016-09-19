/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmOutputConverter_h
#define cmOutputConverter_h

#include <cmConfigure.h> // IWYU pragma: keep

#include "cmState.h"

#include <string>
#include <vector>

class cmOutputConverter
{
public:
  cmOutputConverter(cmState::Snapshot snapshot);

  /**
   * Convert something to something else. This is a centralized conversion
   * routine used by the generators to handle relative paths and the like.
   * The flags determine what is actually done.
   *
   * relative: treat the argument as a directory and convert it to make it
   * relative or full or unchanged. If relative (HOME, START etc) then that
   * specifies what it should be relative to.
   *
   * output: make the result suitable for output to a...
   *
   * optional: should any relative path operation be controlled by the rel
   * path setting
   */
  enum RelativeRoot
  {
    HOME,
    START,
    HOME_OUTPUT,
    START_OUTPUT
  };
  enum OutputFormat
  {
    MAKERULE,
    SHELL,
    WATCOMQUOTE,
    RESPONSE
  };
  std::string ConvertToOutputFormat(const std::string& source,
                                    OutputFormat output) const;
  std::string Convert(const std::string& remote, RelativeRoot local,
                      OutputFormat output) const;
  std::string ConvertToRelativePath(const std::string& remote,
                                    RelativeRoot local) const;
  std::string ConvertDirectorySeparatorsForShell(
    const std::string& source) const;

  ///! for existing files convert to output path and short path if spaces
  std::string ConvertToOutputForExisting(const std::string& remote,
                                         OutputFormat format = SHELL) const;

  void SetLinkScriptShell(bool linkScriptShell);

  /**
   * Flags to pass to Shell_GetArgumentForWindows or
   * Shell_GetArgumentForUnix.  These modify the generated
   * quoting and escape sequences to work under alternative
   * environments.
   */
  enum Shell_Flag_e
  {
    /** The target shell is in a makefile.  */
    Shell_Flag_Make = (1 << 0),

    /** The target shell is in a VS project file.  Do not use with
        Shell_Flag_Make.  */
    Shell_Flag_VSIDE = (1 << 1),

    /** In a windows shell the argument is being passed to "echo".  */
    Shell_Flag_EchoWindows = (1 << 2),

    /** The target shell is in a Watcom WMake makefile.  */
    Shell_Flag_WatcomWMake = (1 << 3),

    /** The target shell is in a MinGW Make makefile.  */
    Shell_Flag_MinGWMake = (1 << 4),

    /** The target shell is in a NMake makefile.  */
    Shell_Flag_NMake = (1 << 5),

    /** Make variable reference syntax $(MAKEVAR) should not be escaped
        to allow a build tool to replace it.  Replacement values
        containing spaces, quotes, backslashes, or other
        non-alphanumeric characters that have significance to some makes
        or shells produce undefined behavior.  */
    Shell_Flag_AllowMakeVariables = (1 << 6),

    /** The target shell quoting uses extra single Quotes for Watcom tools.  */
    Shell_Flag_WatcomQuote = (1 << 7)
  };

  /**
   * Transform the given command line argument for use in a Windows or
   * Unix shell.  Returns a pointer to the end of the command line
   * argument in the provided output buffer.  Flags may be passed to
   * modify the generated quoting and escape sequences to work under
   * alternative environments.
   */
  static std::string Shell_GetArgumentForWindows(const char* in, int flags);
  static std::string Shell_GetArgumentForUnix(const char* in, int flags);

  std::string EscapeForShell(const std::string& str, bool makeVars = false,
                             bool forEcho = false,
                             bool useWatcomQuote = false) const;

  static std::string EscapeForCMake(const std::string& str);

  /** Compute an escaped version of the given argument for use in a
      windows shell.  */
  static std::string EscapeWindowsShellArgument(const char* arg,
                                                int shell_flags);

  enum FortranFormat
  {
    FortranFormatNone,
    FortranFormatFixed,
    FortranFormatFree
  };
  static FortranFormat GetFortranFormat(const char* value);

  /**
   * Convert the given remote path to a relative path with respect to
   * the given local path.  The local path must be given in component
   * form (see SystemTools::SplitPath) without a trailing slash.  The
   * remote path must use forward slashes and not already be escaped
   * or quoted.
   */
  std::string ConvertToRelativePath(const std::vector<std::string>& local,
                                    const std::string& in_remote,
                                    bool force = false) const;

  /**
   * Convert the given remote path to a relative path with respect to
   * the given local path.  Both paths must use forward slashes and not
   * already be escaped or quoted.
   * The conversion is skipped if the paths are not both in the source
   * or both in the binary tree.
   */
  std::string ConvertToRelativePath(std::string const& local_path,
                                    std::string const& remote_path) const;

  /**
   * Convert the given remote path to a relative path with respect to
   * the given local path.  Both paths must use forward slashes and not
   * already be escaped or quoted.
   */
  static std::string ForceToRelativePath(std::string const& local_path,
                                         std::string const& remote_path);

private:
  cmState* GetState() const;

  static int Shell__CharIsWhitespace(char c);
  static int Shell__CharNeedsQuotesOnUnix(char c);
  static int Shell__CharNeedsQuotesOnWindows(char c);
  static int Shell__CharNeedsQuotes(char c, int isUnix, int flags);
  static int Shell__CharIsMakeVariableName(char c);
  static const char* Shell__SkipMakeVariables(const char* c);
  static int Shell__ArgumentNeedsQuotes(const char* in, int isUnix, int flags);
  static std::string Shell__GetArgument(const char* in, int isUnix, int flags);

private:
  cmState::Snapshot StateSnapshot;

  bool LinkScriptShell;
};

#endif
