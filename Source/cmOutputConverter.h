/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include <cm/string_view>

#include "cmStateSnapshot.h"

class cmState;

class cmOutputConverter
{
public:
  cmOutputConverter(cmStateSnapshot const& snapshot);

  /**
   * Convert the given remote path to a relative path with respect to
   * one of our common work directories.  The path must use forward
   * slashes and not already be escaped or quoted.
   * The conversion is skipped if the paths are not both in the source
   * or both in the binary tree.
   */
  std::string MaybeRelativeToTopBinDir(std::string const& path) const;
  std::string MaybeRelativeToCurBinDir(std::string const& path) const;

  std::string const& GetRelativePathTopSource() const;
  std::string const& GetRelativePathTopBinary() const;
  void SetRelativePathTop(std::string const& topSource,
                          std::string const& topBinary);

  enum OutputFormat
  {
    SHELL,
    WATCOMQUOTE,
    NINJAMULTI,
    RESPONSE
  };
  std::string ConvertToOutputFormat(cm::string_view source,
                                    OutputFormat output) const;
  std::string ConvertDirectorySeparatorsForShell(cm::string_view source) const;

  //! for existing files convert to output path and short path if spaces
  std::string ConvertToOutputForExisting(const std::string& remote,
                                         OutputFormat format = SHELL) const;

  void SetLinkScriptShell(bool linkScriptShell);

  /**
   * Flags to pass to Shell_GetArgument.  These modify the generated
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
    Shell_Flag_WatcomQuote = (1 << 7),

    Shell_Flag_IsUnix = (1 << 8),

    Shell_Flag_UnescapeNinjaConfiguration = (1 << 9),

    Shell_Flag_IsResponse = (1 << 10)
  };

  std::string EscapeForShell(cm::string_view str, bool makeVars = false,
                             bool forEcho = false, bool useWatcomQuote = false,
                             bool unescapeNinjaConfiguration = false,
                             bool forResponse = false) const;

  enum class WrapQuotes
  {
    Wrap,
    NoWrap,
  };
  static std::string EscapeForCMake(cm::string_view str,
                                    WrapQuotes wrapQuotes = WrapQuotes::Wrap);

  /** Compute an escaped version of the given argument for use in a
      windows shell.  */
  static std::string EscapeWindowsShellArgument(cm::string_view arg,
                                                int shell_flags);

  enum FortranFormat
  {
    FortranFormatNone,
    FortranFormatFixed,
    FortranFormatFree
  };
  static FortranFormat GetFortranFormat(cm::string_view value);

  enum class FortranPreprocess
  {
    Unset,
    NotNeeded,
    Needed
  };
  static FortranPreprocess GetFortranPreprocess(cm::string_view value);

protected:
  cmStateSnapshot StateSnapshot;

private:
  cmState* GetState() const;

  static bool Shell_CharNeedsQuotes(char c, int flags);
  static cm::string_view::iterator Shell_SkipMakeVariables(
    cm::string_view::iterator begin, cm::string_view::iterator end);
  static bool Shell_ArgumentNeedsQuotes(cm::string_view in, int flags);
  static std::string Shell_GetArgument(cm::string_view in, int flags);

  bool LinkScriptShell = false;

  // The top-most directories for relative path conversion.  Both the
  // source and destination location of a relative path conversion
  // must be underneath one of these directories (both under source or
  // both under binary) in order for the relative path to be evaluated
  // safely by the build tools.
  std::string RelativePathTopSource;
  std::string RelativePathTopBinary;
  enum class TopRelation
  {
    Separate,
    BinInSrc,
    SrcInBin,
    InSource,
  };
  TopRelation RelativePathTopRelation = TopRelation::Separate;
  void ComputeRelativePathTopSource();
  void ComputeRelativePathTopBinary();
  void ComputeRelativePathTopRelation();
  std::string MaybeRelativeTo(std::string const& local_path,
                              std::string const& remote_path) const;
};
