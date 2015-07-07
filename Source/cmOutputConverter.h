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

#include "cmStandardIncludes.h"

#include "cmGlobalGenerator.h"
#include "cmState.h"

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
  enum RelativeRoot { NONE, FULL, HOME, START, HOME_OUTPUT, START_OUTPUT };
  enum OutputFormat { UNCHANGED, MAKERULE, SHELL, WATCOMQUOTE, RESPONSE };
  std::string ConvertToOutputFormat(const std::string& source,
                                    OutputFormat output) const;
  std::string Convert(const std::string& remote, RelativeRoot local,
                      OutputFormat output = UNCHANGED) const;
  std::string Convert(RelativeRoot remote, const std::string& local,
                      OutputFormat output = UNCHANGED,
                      bool optional = false) const;

  /**
    * Get path for the specified relative root.
    */
  const char* GetRelativeRootPath(RelativeRoot relroot) const;

  ///! for existing files convert to output path and short path if spaces
  std::string ConvertToOutputForExisting(const std::string& remote,
                                         RelativeRoot local = START_OUTPUT,
                                         OutputFormat format = SHELL) const;

  /** For existing path identified by RelativeRoot convert to output
      path and short path if spaces.  */
  std::string ConvertToOutputForExisting(RelativeRoot remote,
                                         const std::string& local = "",
                                         OutputFormat format = SHELL) const;

  void SetLinkScriptShell(bool linkScriptShell);

  std::string EscapeForShell(const std::string& str,
                                    bool makeVars = false,
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

private:
  cmState* GetState() const;

  std::string ConvertToOutputForExistingCommon(const std::string& remote,
                                               std::string const& result,
                                               OutputFormat format) const;

private:
  cmState::Snapshot StateSnapshot;

  bool LinkScriptShell;
};

#endif
