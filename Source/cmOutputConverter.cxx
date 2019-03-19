/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmOutputConverter.h"

#include <algorithm>
#include <assert.h>
#include <ctype.h>
#include <set>
#include <string.h>
#include <vector>

#include "cmState.h"
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

std::string cmOutputConverter::ConvertToOutputFormat(const std::string& source,
                                                     OutputFormat output) const
{
  std::string result = source;
  // Convert it to an output path.
  if (output == SHELL || output == WATCOMQUOTE) {
    result = this->ConvertDirectorySeparatorsForShell(source);
    result = this->EscapeForShell(result, true, false, output == WATCOMQUOTE);
  } else if (output == RESPONSE) {
    result = this->EscapeForShell(result, false, false, false);
  }
  return result;
}

std::string cmOutputConverter::ConvertDirectorySeparatorsForShell(
  const std::string& source) const
{
  std::string result = source;
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

static bool cmOutputConverterIsShellOperator(const std::string& str)
{
  static std::set<std::string> shellOperators;
  if (shellOperators.empty()) {
    shellOperators.insert("<");
    shellOperators.insert(">");
    shellOperators.insert("<<");
    shellOperators.insert(">>");
    shellOperators.insert("|");
    shellOperators.insert("||");
    shellOperators.insert("&&");
    shellOperators.insert("&>");
    shellOperators.insert("1>");
    shellOperators.insert("2>");
    shellOperators.insert("2>&1");
    shellOperators.insert("1>&2");
  }
  return shellOperators.count(str) > 0;
}

std::string cmOutputConverter::EscapeForShell(const std::string& str,
                                              bool makeVars, bool forEcho,
                                              bool useWatcomQuote) const
{
  // Do not escape shell operators.
  if (cmOutputConverterIsShellOperator(str)) {
    return str;
  }

  // Compute the flags for the target shell environment.
  int flags = 0;
  if (this->GetState()->UseWindowsVSIDE()) {
    flags |= Shell_Flag_VSIDE;
  } else if (!this->LinkScriptShell) {
    flags |= Shell_Flag_Make;
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

  return Shell__GetArgument(str.c_str(), flags);
}

std::string cmOutputConverter::EscapeForCMake(const std::string& str)
{
  // Always double-quote the argument to take care of most escapes.
  std::string result = "\"";
  for (const char* c = str.c_str(); *c; ++c) {
    if (*c == '"') {
      // Escape the double quote to avoid ending the argument.
      result += "\\\"";
    } else if (*c == '$') {
      // Escape the dollar to avoid expanding variables.
      result += "\\$";
    } else if (*c == '\\') {
      // Escape the backslash to avoid other escapes.
      result += "\\\\";
    } else {
      // Other characters will be parsed correctly.
      result += *c;
    }
  }
  result += "\"";
  return result;
}

std::string cmOutputConverter::EscapeWindowsShellArgument(const char* arg,
                                                          int shell_flags)
{
  return Shell__GetArgument(arg, shell_flags);
}

cmOutputConverter::FortranFormat cmOutputConverter::GetFortranFormat(
  const char* value)
{
  FortranFormat format = FortranFormatNone;
  if (value && *value) {
    std::vector<std::string> fmt;
    cmSystemTools::ExpandListArgument(value, fmt);
    for (std::string const& fi : fmt) {
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
static int Shell__CharIsWhitespace(char c)
{
  return ((c == ' ') || (c == '\t'));
}

static int Shell__CharNeedsQuotesOnUnix(char c)
{
  return ((c == '\'') || (c == '`') || (c == ';') || (c == '#') ||
          (c == '&') || (c == '$') || (c == '(') || (c == ')') || (c == '~') ||
          (c == '<') || (c == '>') || (c == '|') || (c == '*') || (c == '^') ||
          (c == '\\'));
}

static int Shell__CharNeedsQuotesOnWindows(char c)
{
  return ((c == '\'') || (c == '#') || (c == '&') || (c == '<') ||
          (c == '>') || (c == '|') || (c == '^'));
}

static int Shell__CharIsMakeVariableName(char c)
{
  return c && (c == '_' || isalpha((static_cast<int>(c))));
}

int cmOutputConverter::Shell__CharNeedsQuotes(char c, int flags)
{
  /* On Windows the built-in command shell echo never needs quotes.  */
  if (!(flags & Shell_Flag_IsUnix) && (flags & Shell_Flag_EchoWindows)) {
    return 0;
  }

  /* On all platforms quotes are needed to preserve whitespace.  */
  if (Shell__CharIsWhitespace(c)) {
    return 1;
  }

  if (flags & Shell_Flag_IsUnix) {
    /* On UNIX several special characters need quotes to preserve them.  */
    if (Shell__CharNeedsQuotesOnUnix(c)) {
      return 1;
    }
  } else {
    /* On Windows several special characters need quotes to preserve them.  */
    if (Shell__CharNeedsQuotesOnWindows(c)) {
      return 1;
    }
  }
  return 0;
}

const char* cmOutputConverter::Shell__SkipMakeVariables(const char* c)
{
  while (*c == '$' && *(c + 1) == '(') {
    const char* skip = c + 2;
    while (Shell__CharIsMakeVariableName(*skip)) {
      ++skip;
    }
    if (*skip == ')') {
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

int cmOutputConverter::Shell__ArgumentNeedsQuotes(const char* in, int flags)
{
  /* The empty string needs quotes.  */
  if (!*in) {
    return 1;
  }

  /* Scan the string for characters that require quoting.  */
  {
    const char* c;
    for (c = in; *c; ++c) {
      /* Look for $(MAKEVAR) syntax if requested.  */
      if (flags & Shell_Flag_AllowMakeVariables) {
#if KWSYS_SYSTEM_SHELL_QUOTE_MAKE_VARIABLES
        const char* skip = Shell__SkipMakeVariables(c);
        if (skip != c) {
          /* We need to quote make variable references to preserve the
             string with contents substituted in its place.  */
          return 1;
        }
#else
        /* Skip over the make variable references if any are present.  */
        c = Shell__SkipMakeVariables(c);

        /* Stop if we have reached the end of the string.  */
        if (!*c) {
          break;
        }
#endif
      }

      /* Check whether this character needs quotes.  */
      if (Shell__CharNeedsQuotes(*c, flags)) {
        return 1;
      }
    }
  }

  /* On Windows some single character arguments need quotes.  */
  if (flags & Shell_Flag_IsUnix && *in && !*(in + 1)) {
    char c = *in;
    if ((c == '?') || (c == '&') || (c == '^') || (c == '|') || (c == '#')) {
      return 1;
    }
  }

  return 0;
}

std::string cmOutputConverter::Shell__GetArgument(const char* in, int flags)
{
  /* Output will be at least as long as input string.  */
  std::string out;
  out.reserve(strlen(in));

  /* String iterator.  */
  const char* c;

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
  for (c = in; *c; ++c) {
    /* Look for $(MAKEVAR) syntax if requested.  */
    if (flags & Shell_Flag_AllowMakeVariables) {
      const char* skip = Shell__SkipMakeVariables(c);
      if (skip != c) {
        /* Copy to the end of the make variable references.  */
        while (c != skip) {
          out += *c++;
        }

        /* The make variable reference eliminates any escaping needed
           for preceding backslashes.  */
        windows_backslashes = 0;

        /* Stop if we have reached the end of the string.  */
        if (!*c) {
          break;
        }
      }
    }

    /* Check whether this character needs escaping for the shell.  */
    if (flags & Shell_Flag_IsUnix) {
      /* On Unix a few special characters need escaping even inside a
         quoted argument.  */
      if (*c == '\\' || *c == '"' || *c == '`' || *c == '$') {
        /* This character needs a backslash to escape it.  */
        out += '\\';
      }
    } else if (flags & Shell_Flag_EchoWindows) {
      /* On Windows the built-in command shell echo never needs escaping.  */
    } else {
      /* On Windows only backslashes and double-quotes need escaping.  */
      if (*c == '\\') {
        /* Found a backslash.  It may need to be escaped later.  */
        ++windows_backslashes;
      } else if (*c == '"') {
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
    if (*c == '$') {
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
    } else if (*c == '#') {
      if ((flags & Shell_Flag_Make) && (flags & Shell_Flag_WatcomWMake)) {
        /* In Watcom WMake makefiles a pound is written $#.  The make
           tool will replace it with just # before passing it to the
           shell.  */
        out += "$#";
      } else {
        /* Otherwise a pound is written just #. */
        out += '#';
      }
    } else if (*c == '%') {
      if ((flags & Shell_Flag_VSIDE) ||
          ((flags & Shell_Flag_Make) &&
           ((flags & Shell_Flag_MinGWMake) || (flags & Shell_Flag_NMake)))) {
        /* In the VS IDE, NMake, or MinGW make a percent is written %%.  */
        out += "%%";
      } else {
        /* Otherwise a percent is written just %. */
        out += '%';
      }
    } else if (*c == ';') {
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
      out += *c;
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

  return out;
}
