/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmOutputConverter.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <set>
#include <vector>

#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmOutputConverter::cmOutputConverter(cmStateSnapshot const& snapshot)
  : StateSnapshot(snapshot)
  , LinkScriptShell(false)
{
  assert(this->StateSnapshot.IsValid());
}

std::string cmOutputConverter::ConvertToOutputForExisting(
  const std::string& remote, OutputFormat format) const
{
  // If this is a windows shell, the result has a space, and the path
  // already exists, we can use a short-path to reference it without a
  // space.
  if (this->GetState()->UseWindowsShell() &&
      remote.find(' ') != std::string::npos &&
      cmSystemTools::FileExists(remote)) {
    std::string tmp;
    if (cmSystemTools::GetShortPath(remote, tmp)) {
      return this->ConvertToOutputFormat(tmp, format);
    }
  }

  // Otherwise, perform standard conversion.
  return this->ConvertToOutputFormat(remote, format);
}

std::string cmOutputConverter::ConvertToOutputFormat(cm::string_view source,
                                                     OutputFormat output) const
{
  std::string result(source);
  // Convert it to an output path.
  if (output == SHELL || output == WATCOMQUOTE || output == NINJAMULTI) {
    result = this->ConvertDirectorySeparatorsForShell(source);
    result = this->EscapeForShell(result, true, false, output == WATCOMQUOTE,
                                  output == NINJAMULTI);
  } else if (output == RESPONSE) {
    result = this->EscapeForShell(result, false, false, false);
  }
  return result;
}

std::string cmOutputConverter::ConvertDirectorySeparatorsForShell(
  cm::string_view source) const
{
  std::string result(source);
  // For the MSYS shell convert drive letters to posix paths, so
  // that c:/some/path becomes /c/some/path.  This is needed to
  // avoid problems with the shell path translation.
  if (this->GetState()->UseMSYSShell() && !this->LinkScriptShell) {
    if (result.size() > 2 && result[1] == ':') {
      result[1] = result[0];
      result[0] = '/';
    }
  }
  if (this->GetState()->UseWindowsShell()) {
    std::replace(result.begin(), result.end(), '/', '\\');
  }
  return result;
}

static bool cmOutputConverterIsShellOperator(cm::string_view str)
{
  static std::set<cm::string_view> const shellOperators{
    "<", ">", "<<", ">>", "|", "||", "&&", "&>", "1>", "2>", "2>&1", "1>&2"
  };
  return (shellOperators.count(str) != 0);
}

std::string cmOutputConverter::EscapeForShell(
  cm::string_view str, bool makeVars, bool forEcho, bool useWatcomQuote,
  bool unescapeNinjaConfiguration) const
{
  // Do not escape shell operators.
  if (cmOutputConverterIsShellOperator(str)) {
    return std::string(str);
  }

  // Compute the flags for the target shell environment.
  int flags = 0;
  if (this->GetState()->UseWindowsVSIDE()) {
    flags |= Shell_Flag_VSIDE;
  } else if (!this->LinkScriptShell) {
    flags |= Shell_Flag_Make;
  }
  if (unescapeNinjaConfiguration) {
    flags |= Shell_Flag_UnescapeNinjaConfiguration;
  }
  if (makeVars) {
    flags |= Shell_Flag_AllowMakeVariables;
  }
  if (forEcho) {
    flags |= Shell_Flag_EchoWindows;
  }
  if (useWatcomQuote) {
    flags |= Shell_Flag_WatcomQuote;
  }
  if (this->GetState()->UseWatcomWMake()) {
    flags |= Shell_Flag_WatcomWMake;
  }
  if (this->GetState()->UseMinGWMake()) {
    flags |= Shell_Flag_MinGWMake;
  }
  if (this->GetState()->UseNMake()) {
    flags |= Shell_Flag_NMake;
  }
  if (!this->GetState()->UseWindowsShell()) {
    flags |= Shell_Flag_IsUnix;
  }

  return Shell__GetArgument(str, flags);
}

std::string cmOutputConverter::EscapeForCMake(cm::string_view str)
{
  // Always double-quote the argument to take care of most escapes.
  std::string result = "\"";
  for (const char c : str) {
    if (c == '"') {
      // Escape the double quote to avoid ending the argument.
      result += "\\\"";
    } else if (c == '$') {
      // Escape the dollar to avoid expanding variables.
      result += "\\$";
    } else if (c == '\\') {
      // Escape the backslash to avoid other escapes.
      result += "\\\\";
    } else {
      // Other characters will be parsed correctly.
      result += c;
    }
  }
  result += "\"";
  return result;
}

std::string cmOutputConverter::EscapeWindowsShellArgument(cm::string_view arg,
                                                          int shell_flags)
{
  return Shell__GetArgument(arg, shell_flags);
}

cmOutputConverter::FortranFormat cmOutputConverter::GetFortranFormat(
  cm::string_view value)
{
  FortranFormat format = FortranFormatNone;
  if (!value.empty()) {
    for (std::string const& fi : cmExpandedList(value)) {
      if (fi == "FIXED") {
        format = FortranFormatFixed;
      }
      if (fi == "FREE") {
        format = FortranFormatFree;
      }
    }
  }
  return format;
}

cmOutputConverter::FortranFormat cmOutputConverter::GetFortranFormat(
  const char* value)
{
  if (!value) {
    return FortranFormatNone;
  }
  return GetFortranFormat(cm::string_view(value));
}

void cmOutputConverter::SetLinkScriptShell(bool linkScriptShell)
{
  this->LinkScriptShell = linkScriptShell;
}

cmState* cmOutputConverter::GetState() const
{
  return this->StateSnapshot.GetState();
}

/*

Notes:

Make variable replacements open a can of worms.  Sometimes they should
be quoted and sometimes not.  Sometimes their replacement values are
already quoted.

VS variables cause problems.  In order to pass the referenced value
with spaces the reference must be quoted.  If the variable value ends
in a backslash then it will escape the ending quote!  In order to make
the ending backslash appear we need this:

  "$(InputDir)\"

However if there is not a trailing backslash then this will put a
quote in the value so we need:

  "$(InputDir)"

Make variable references are platform specific so we should probably
just NOT quote them and let the listfile author deal with it.

*/

/*
TODO: For windows echo:

To display a pipe (|) or redirection character (< or >) when using the
echo command, use a caret character immediately before the pipe or
redirection character (for example, ^>, ^<, or ^| ). If you need to
use the caret character itself (^), use two in a row (^^).
*/

/* Some helpers to identify character classes */
static bool Shell__CharIsWhitespace(char c)
{
  return ((c == ' ') || (c == '\t'));
}

static bool Shell__CharNeedsQuotesOnUnix(char c)
{
  return ((c == '\'') || (c == '`') || (c == ';') || (c == '#') ||
          (c == '&') || (c == '$') || (c == '(') || (c == ')') || (c == '~') ||
          (c == '<') || (c == '>') || (c == '|') || (c == '*') || (c == '^') ||
          (c == '\\'));
}

static bool Shell__CharNeedsQuotesOnWindows(char c)
{
  return ((c == '\'') || (c == '#') || (c == '&') || (c == '<') ||
          (c == '>') || (c == '|') || (c == '^'));
}

static bool Shell__CharIsMakeVariableName(char c)
{
  return c && (c == '_' || isalpha((static_cast<int>(c))));
}

bool cmOutputConverter::Shell__CharNeedsQuotes(char c, int flags)
{
  /* On Windows the built-in command shell echo never needs quotes.  */
  if (!(flags & Shell_Flag_IsUnix) && (flags & Shell_Flag_EchoWindows)) {
    return false;
  }

  /* On all platforms quotes are needed to preserve whitespace.  */
  if (Shell__CharIsWhitespace(c)) {
    return true;
  }

  if (flags & Shell_Flag_IsUnix) {
    /* On UNIX several special characters need quotes to preserve them.  */
    if (Shell__CharNeedsQuotesOnUnix(c)) {
      return true;
    }
  } else {
    /* On Windows several special characters need quotes to preserve them.  */
    if (Shell__CharNeedsQuotesOnWindows(c)) {
      return true;
    }
  }
  return false;
}

cm::string_view::iterator cmOutputConverter::Shell__SkipMakeVariables(
  cm::string_view::iterator c, cm::string_view::iterator end)
{
  while ((c != end && (c + 1) != end) && (*c == '$' && *(c + 1) == '(')) {
    cm::string_view::iterator skip = c + 2;
    while ((skip != end) && Shell__CharIsMakeVariableName(*skip)) {
      ++skip;
    }
    if ((skip != end) && *skip == ')') {
      c = skip + 1;
    } else {
      break;
    }
  }
  return c;
}

/*
Allowing make variable replacements opens a can of worms.  Sometimes
they should be quoted and sometimes not.  Sometimes their replacement
values are already quoted or contain escapes.

Some Visual Studio variables cause problems.  In order to pass the
referenced value with spaces the reference must be quoted.  If the
variable value ends in a backslash then it will escape the ending
quote!  In order to make the ending backslash appear we need this:

  "$(InputDir)\"

However if there is not a trailing backslash then this will put a
quote in the value so we need:

  "$(InputDir)"

This macro decides whether we quote an argument just because it
contains a make variable reference.  This should be replaced with a
flag later when we understand applications of this better.
*/
#define KWSYS_SYSTEM_SHELL_QUOTE_MAKE_VARIABLES 0

bool cmOutputConverter::Shell__ArgumentNeedsQuotes(cm::string_view in,
                                                   int flags)
{
  /* The empty string needs quotes.  */
  if (in.empty()) {
    return true;
  }

  /* Scan the string for characters that require quoting.  */
  for (cm::string_view::iterator cit = in.begin(), cend = in.end();
       cit != cend; ++cit) {
    /* Look for $(MAKEVAR) syntax if requested.  */
    if (flags & Shell_Flag_AllowMakeVariables) {
#if KWSYS_SYSTEM_SHELL_QUOTE_MAKE_VARIABLES
      cm::string_view::iterator skip = Shell__SkipMakeVariables(cit, cend);
      if (skip != cit) {
        /* We need to quote make variable references to preserve the
           string with contents substituted in its place.  */
        return true;
      }
#else
      /* Skip over the make variable references if any are present.  */
      cit = Shell__SkipMakeVariables(cit, cend);

      /* Stop if we have reached the end of the string.  */
      if (cit == cend) {
        break;
      }
#endif
    }

    /* Check whether this character needs quotes.  */
    if (Shell__CharNeedsQuotes(*cit, flags)) {
      return true;
    }
  }

  /* On Windows some single character arguments need quotes.  */
  if (flags & Shell_Flag_IsUnix && in.size() == 1) {
    char c = in[0];
    if ((c == '?') || (c == '&') || (c == '^') || (c == '|') || (c == '#')) {
      return true;
    }
  }

  return false;
}

std::string cmOutputConverter::Shell__GetArgument(cm::string_view in,
                                                  int flags)
{
  /* Output will be at least as long as input string.  */
  std::string out;
  out.reserve(in.size());

  /* Keep track of how many backslashes have been encountered in a row.  */
  int windows_backslashes = 0;

  /* Whether the argument must be quoted.  */
  int needQuotes = Shell__ArgumentNeedsQuotes(in, flags);
  if (needQuotes) {
    /* Add the opening quote for this argument.  */
    if (flags & Shell_Flag_WatcomQuote) {
      if (flags & Shell_Flag_IsUnix) {
        out += '"';
      }
      out += '\'';
    } else {
      out += '"';
    }
  }

  /* Scan the string for characters that require escaping or quoting.  */
  for (cm::string_view::iterator cit = in.begin(), cend = in.end();
       cit != cend; ++cit) {
    /* Look for $(MAKEVAR) syntax if requested.  */
    if (flags & Shell_Flag_AllowMakeVariables) {
      cm::string_view::iterator skip = Shell__SkipMakeVariables(cit, cend);
      if (skip != cit) {
        /* Copy to the end of the make variable references.  */
        while (cit != skip) {
          out += *cit++;
        }

        /* The make variable reference eliminates any escaping needed
           for preceding backslashes.  */
        windows_backslashes = 0;

        /* Stop if we have reached the end of the string.  */
        if (cit == cend) {
          break;
        }
      }
    }

    /* Check whether this character needs escaping for the shell.  */
    if (flags & Shell_Flag_IsUnix) {
      /* On Unix a few special characters need escaping even inside a
         quoted argument.  */
      if (*cit == '\\' || *cit == '"' || *cit == '`' || *cit == '$') {
        /* This character needs a backslash to escape it.  */
        out += '\\';
      }
    } else if (flags & Shell_Flag_EchoWindows) {
      /* On Windows the built-in command shell echo never needs escaping.  */
    } else {
      /* On Windows only backslashes and double-quotes need escaping.  */
      if (*cit == '\\') {
        /* Found a backslash.  It may need to be escaped later.  */
        ++windows_backslashes;
      } else if (*cit == '"') {
        /* Found a double-quote.  Escape all immediately preceding
           backslashes.  */
        while (windows_backslashes > 0) {
          --windows_backslashes;
          out += '\\';
        }

        /* Add the backslash to escape the double-quote.  */
        out += '\\';
      } else {
        /* We encountered a normal character.  This eliminates any
           escaping needed for preceding backslashes.  */
        windows_backslashes = 0;
      }
    }

    /* Check whether this character needs escaping for a make tool.  */
    if (*cit == '$') {
      if (flags & Shell_Flag_Make) {
        /* In Makefiles a dollar is written $$.  The make tool will
           replace it with just $ before passing it to the shell.  */
        out += "$$";
      } else if (flags & Shell_Flag_VSIDE) {
        /* In a VS IDE a dollar is written "$".  If this is written in
           an un-quoted argument it starts a quoted segment, inserts
           the $ and ends the segment.  If it is written in a quoted
           argument it ends quoting, inserts the $ and restarts
           quoting.  Either way the $ is isolated from surrounding
           text to avoid looking like a variable reference.  */
        out += "\"$\"";
      } else {
        /* Otherwise a dollar is written just $. */
        out += '$';
      }
    } else if (*cit == '#') {
      if ((flags & Shell_Flag_Make) && (flags & Shell_Flag_WatcomWMake)) {
        /* In Watcom WMake makefiles a pound is written $#.  The make
           tool will replace it with just # before passing it to the
           shell.  */
        out += "$#";
      } else {
        /* Otherwise a pound is written just #. */
        out += '#';
      }
    } else if (*cit == '%') {
      if ((flags & Shell_Flag_VSIDE) ||
          ((flags & Shell_Flag_Make) &&
           ((flags & Shell_Flag_MinGWMake) || (flags & Shell_Flag_NMake)))) {
        /* In the VS IDE, NMake, or MinGW make a percent is written %%.  */
        out += "%%";
      } else {
        /* Otherwise a percent is written just %. */
        out += '%';
      }
    } else if (*cit == ';') {
      if (flags & Shell_Flag_VSIDE) {
        /* In a VS IDE a semicolon is written ";".  If this is written
           in an un-quoted argument it starts a quoted segment,
           inserts the ; and ends the segment.  If it is written in a
           quoted argument it ends quoting, inserts the ; and restarts
           quoting.  Either way the ; is isolated.  */
        out += "\";\"";
      } else {
        /* Otherwise a semicolon is written just ;. */
        out += ';';
      }
    } else {
      /* Store this character.  */
      out += *cit;
    }
  }

  if (needQuotes) {
    /* Add enough backslashes to escape any trailing ones.  */
    while (windows_backslashes > 0) {
      --windows_backslashes;
      out += '\\';
    }

    /* Add the closing quote for this argument.  */
    if (flags & Shell_Flag_WatcomQuote) {
      out += '\'';
      if (flags & Shell_Flag_IsUnix) {
        out += '"';
      }
    } else {
      out += '"';
    }
  }

  if (flags & Shell_Flag_UnescapeNinjaConfiguration) {
    std::string ninjaConfigReplace;
    if (flags & Shell_Flag_IsUnix) {
      ninjaConfigReplace += '\\';
    }
    ninjaConfigReplace += "$${CONFIGURATION}";
    cmSystemTools::ReplaceString(out, ninjaConfigReplace, "${CONFIGURATION}");
  }

  return out;
}
