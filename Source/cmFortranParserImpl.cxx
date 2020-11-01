/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <cassert>
#include <cstdio>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "cmFortranParser.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

bool cmFortranParser_s::FindIncludeFile(const char* dir,
                                        const char* includeName,
                                        std::string& fileName)
{
  // If the file is a full path, include it directly.
  if (cmSystemTools::FileIsFullPath(includeName)) {
    fileName = includeName;
    return cmSystemTools::FileExists(fileName, true);
  }
  // Check for the file in the directory containing the including
  // file.
  std::string fullName = cmStrCat(dir, '/', includeName);
  if (cmSystemTools::FileExists(fullName, true)) {
    fileName = fullName;
    return true;
  }

  // Search the include path for the file.
  for (std::string const& i : this->IncludePath) {
    fullName = cmStrCat(i, '/', includeName);
    if (cmSystemTools::FileExists(fullName, true)) {
      fileName = fullName;
      return true;
    }
  }
  return false;
}

cmFortranParser_s::cmFortranParser_s(cmFortranCompiler fc,
                                     std::vector<std::string> includes,
                                     std::set<std::string> defines,
                                     cmFortranSourceInfo& info)
  : Compiler(std::move(fc))
  , IncludePath(std::move(includes))
  , PPDefinitions(std::move(defines))
  , Info(info)
{
  this->InInterface = false;
  this->InPPFalseBranch = 0;

  // Initialize the lexical scanner.
  cmFortran_yylex_init(&this->Scanner);
  cmFortran_yyset_extra(this, this->Scanner);

  // Create a dummy buffer that is never read but is the fallback
  // buffer when the last file is popped off the stack.
  YY_BUFFER_STATE buffer =
    cmFortran_yy_create_buffer(nullptr, 4, this->Scanner);
  cmFortran_yy_switch_to_buffer(buffer, this->Scanner);
}

cmFortranParser_s::~cmFortranParser_s()
{
  cmFortran_yylex_destroy(this->Scanner);
}

std::string cmFortranParser_s::ModName(std::string const& mod_name) const
{
  return mod_name + ".mod";
}

std::string cmFortranParser_s::SModName(std::string const& mod_name,
                                        std::string const& sub_name) const
{
  std::string const& SModExt =
    this->Compiler.SModExt.empty() ? ".mod" : this->Compiler.SModExt;
  // An empty separator means that the compiler does not use a prefix.
  if (this->Compiler.SModSep.empty()) {
    return sub_name + SModExt;
  }
  return mod_name + this->Compiler.SModSep + sub_name + SModExt;
}

bool cmFortranParser_FilePush(cmFortranParser* parser, const char* fname)
{
  // Open the new file and push it onto the stack.  Save the old
  // buffer with it on the stack.
  if (FILE* file = cmsys::SystemTools::Fopen(fname, "rb")) {
    YY_BUFFER_STATE current = cmFortranLexer_GetCurrentBuffer(parser->Scanner);
    std::string dir = cmSystemTools::GetParentDirectory(fname);
    cmFortranFile f(file, current, dir);
    YY_BUFFER_STATE buffer =
      cmFortran_yy_create_buffer(nullptr, 16384, parser->Scanner);
    cmFortran_yy_switch_to_buffer(buffer, parser->Scanner);
    parser->FileStack.push(f);
    return true;
  }
  return false;
}

bool cmFortranParser_FilePop(cmFortranParser* parser)
{
  // Pop one file off the stack and close it.  Switch the lexer back
  // to the next one on the stack.
  if (parser->FileStack.empty()) {
    return false;
  }
  cmFortranFile f = parser->FileStack.top();
  parser->FileStack.pop();
  fclose(f.File);
  YY_BUFFER_STATE current = cmFortranLexer_GetCurrentBuffer(parser->Scanner);
  cmFortran_yy_delete_buffer(current, parser->Scanner);
  cmFortran_yy_switch_to_buffer(f.Buffer, parser->Scanner);
  return true;
}

int cmFortranParser_Input(cmFortranParser* parser, char* buffer,
                          size_t bufferSize)
{
  // Read from the file on top of the stack.  If the stack is empty,
  // the end of the translation unit has been reached.
  if (!parser->FileStack.empty()) {
    cmFortranFile& ff = parser->FileStack.top();
    FILE* file = ff.File;
    size_t n = fread(buffer, 1, bufferSize, file);
    if (n > 0) {
      ff.LastCharWasNewline = buffer[n - 1] == '\n';
    } else if (!ff.LastCharWasNewline) {
      // The file ended without a newline.  Inject one so
      // that the file always ends in an end-of-statement.
      buffer[0] = '\n';
      n = 1;
      ff.LastCharWasNewline = true;
    }
    return static_cast<int>(n);
  }
  return 0;
}

void cmFortranParser_StringStart(cmFortranParser* parser)
{
  parser->TokenString.clear();
}

const char* cmFortranParser_StringEnd(cmFortranParser* parser)
{
  return parser->TokenString.c_str();
}

void cmFortranParser_StringAppend(cmFortranParser* parser, char c)
{
  parser->TokenString += c;
}

void cmFortranParser_SetInInterface(cmFortranParser* parser, bool in)
{
  if (parser->InPPFalseBranch) {
    return;
  }

  parser->InInterface = in;
}

bool cmFortranParser_GetInInterface(cmFortranParser* parser)
{
  return parser->InInterface;
}

void cmFortranParser_SetOldStartcond(cmFortranParser* parser, int arg)
{
  parser->OldStartcond = arg;
}

int cmFortranParser_GetOldStartcond(cmFortranParser* parser)
{
  return parser->OldStartcond;
}

void cmFortranParser_Error(cmFortranParser* parser, const char* msg)
{
  parser->Error = msg ? msg : "unknown error";
}

void cmFortranParser_RuleUse(cmFortranParser* parser, const char* module_name)
{
  if (parser->InPPFalseBranch) {
    return;
  }

  // syntax:   "use module_name"
  // requires: "module_name.mod"
  std::string const& mod_name = cmSystemTools::LowerCase(module_name);
  parser->Info.Requires.insert(parser->ModName(mod_name));
}

void cmFortranParser_RuleLineDirective(cmFortranParser* parser,
                                       const char* filename)
{
  // This is a #line directive naming a file encountered during preprocessing.
  std::string included = filename;

  // Skip #line directives referencing non-files like
  // "<built-in>" or "<command-line>".
  if (included.empty() || included[0] == '<') {
    return;
  }

  // Fix windows file path separators since our lexer does not
  // process escape sequences in string literals.
  cmSystemTools::ReplaceString(included, "\\\\", "\\");
  cmSystemTools::ConvertToUnixSlashes(included);

  // Save the named file as included in the source.
  if (cmSystemTools::FileExists(included, true)) {
    parser->Info.Includes.insert(included);
  }
}

void cmFortranParser_RuleInclude(cmFortranParser* parser, const char* name)
{
  if (parser->InPPFalseBranch) {
    return;
  }

  // If processing an include statement there must be an open file.
  assert(!parser->FileStack.empty());

  // Get the directory containing the source in which the include
  // statement appears.  This is always the first search location for
  // Fortran include files.
  std::string dir = parser->FileStack.top().Directory;

  // Find the included file.  If it cannot be found just ignore the
  // problem because either the source will not compile or the user
  // does not care about depending on this included source.
  std::string fullName;
  if (parser->FindIncludeFile(dir.c_str(), name, fullName)) {
    // Found the included file.  Save it in the set of included files.
    parser->Info.Includes.insert(fullName);

    // Parse it immediately to translate the source inline.
    cmFortranParser_FilePush(parser, fullName.c_str());
  }
}

void cmFortranParser_RuleModule(cmFortranParser* parser,
                                const char* module_name)
{
  if (parser->InPPFalseBranch) {
    return;
  }

  if (!parser->InInterface) {
    // syntax:   "module module_name"
    // provides: "module_name.mod"
    std::string const& mod_name = cmSystemTools::LowerCase(module_name);
    parser->Info.Provides.insert(parser->ModName(mod_name));
  }
}

void cmFortranParser_RuleSubmodule(cmFortranParser* parser,
                                   const char* module_name,
                                   const char* submodule_name)
{
  if (parser->InPPFalseBranch) {
    return;
  }

  // syntax:   "submodule (module_name) submodule_name"
  // requires: "module_name.mod"
  // provides: "module_name@submodule_name.smod"
  //
  // FIXME: Some compilers split the submodule part of a module into a
  // separate "module_name.smod" file.  Whether it is generated or
  // not depends on conditions more subtle than we currently detect.
  // For now we depend directly on "module_name.mod".

  std::string const& mod_name = cmSystemTools::LowerCase(module_name);
  std::string const& sub_name = cmSystemTools::LowerCase(submodule_name);
  parser->Info.Requires.insert(parser->ModName(mod_name));
  parser->Info.Provides.insert(parser->SModName(mod_name, sub_name));
}

void cmFortranParser_RuleSubmoduleNested(cmFortranParser* parser,
                                         const char* module_name,
                                         const char* submodule_name,
                                         const char* nested_submodule_name)
{
  if (parser->InPPFalseBranch) {
    return;
  }

  // syntax:   "submodule (module_name:submodule_name) nested_submodule_name"
  // requires: "module_name@submodule_name.smod"
  // provides: "module_name@nested_submodule_name.smod"

  std::string const& mod_name = cmSystemTools::LowerCase(module_name);
  std::string const& sub_name = cmSystemTools::LowerCase(submodule_name);
  std::string const& nest_name =
    cmSystemTools::LowerCase(nested_submodule_name);
  parser->Info.Requires.insert(parser->SModName(mod_name, sub_name));
  parser->Info.Provides.insert(parser->SModName(mod_name, nest_name));
}

void cmFortranParser_RuleDefine(cmFortranParser* parser, const char* macro)
{
  if (!parser->InPPFalseBranch) {
    parser->PPDefinitions.insert(macro);
  }
}

void cmFortranParser_RuleUndef(cmFortranParser* parser, const char* macro)
{
  if (!parser->InPPFalseBranch) {
    std::set<std::string>::iterator match;
    match = parser->PPDefinitions.find(macro);
    if (match != parser->PPDefinitions.end()) {
      parser->PPDefinitions.erase(match);
    }
  }
}

void cmFortranParser_RuleIfdef(cmFortranParser* parser, const char* macro)
{
  // A new PP branch has been opened
  parser->SkipToEnd.push(false);

  if (parser->InPPFalseBranch) {
    parser->InPPFalseBranch++;
  } else if (parser->PPDefinitions.find(macro) ==
             parser->PPDefinitions.end()) {
    parser->InPPFalseBranch = 1;
  } else {
    parser->SkipToEnd.top() = true;
  }
}

void cmFortranParser_RuleIfndef(cmFortranParser* parser, const char* macro)
{
  // A new PP branch has been opened
  parser->SkipToEnd.push(false);

  if (parser->InPPFalseBranch) {
    parser->InPPFalseBranch++;
  } else if (parser->PPDefinitions.find(macro) !=
             parser->PPDefinitions.end()) {
    parser->InPPFalseBranch = 1;
  } else {
    // ignore other branches
    parser->SkipToEnd.top() = true;
  }
}

void cmFortranParser_RuleIf(cmFortranParser* parser)
{
  /* Note: The current parser is _not_ able to get statements like
   *   #if 0
   *   #if 1
   *   #if MYSMBOL
   *   #if defined(MYSYMBOL)
   *   #if defined(MYSYMBOL) && ...
   * right.  The same for #elif.  Thus in
   *   #if SYMBOL_1
   *     ..
   *   #elif SYMBOL_2
   *     ...
   *     ...
   *   #elif SYMBOL_N
   *     ..
   *   #else
   *     ..
   *   #endif
   * _all_ N+1 branches are considered.  If you got something like this
   *   #if defined(MYSYMBOL)
   *   #if !defined(MYSYMBOL)
   * use
   *   #ifdef MYSYMBOL
   *   #ifndef MYSYMBOL
   * instead.
   */

  // A new PP branch has been opened
  // Never skip!  See note above.
  parser->SkipToEnd.push(false);
}

void cmFortranParser_RuleElif(cmFortranParser* parser)
{
  /* Note: There are parser limitations.  See the note at
   * cmFortranParser_RuleIf(..)
   */

  // Always taken unless an #ifdef or #ifndef-branch has been taken
  // already.  If the second condition isn't meet already
  // (parser->InPPFalseBranch == 0) correct it.
  if (!parser->SkipToEnd.empty() && parser->SkipToEnd.top() &&
      !parser->InPPFalseBranch) {
    parser->InPPFalseBranch = 1;
  }
}

void cmFortranParser_RuleElse(cmFortranParser* parser)
{
  // if the parent branch is false do nothing!
  if (parser->InPPFalseBranch > 1) {
    return;
  }

  // parser->InPPFalseBranch is either 0 or 1.  We change it depending on
  // parser->SkipToEnd.top()
  if (!parser->SkipToEnd.empty() && parser->SkipToEnd.top()) {
    parser->InPPFalseBranch = 1;
  } else {
    parser->InPPFalseBranch = 0;
  }
}

void cmFortranParser_RuleEndif(cmFortranParser* parser)
{
  if (!parser->SkipToEnd.empty()) {
    parser->SkipToEnd.pop();
  }

  // #endif doesn't know if there was a "#else" in before, so it
  // always decreases InPPFalseBranch
  if (parser->InPPFalseBranch) {
    parser->InPPFalseBranch--;
  }
}
