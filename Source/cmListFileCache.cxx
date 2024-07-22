/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#define cmListFileCache_cxx
#include "cmListFileCache.h"

#include <memory>
#include <ostream>
#include <utility>

#ifdef _WIN32
#  include <cmsys/Encoding.hxx>
#endif

#include "cmList.h"
#include "cmListFileLexer.h"
#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {

enum class NestingStateEnum
{
  If,
  Else,
  While,
  Foreach,
  Function,
  Macro,
  Block
};

struct NestingState
{
  NestingStateEnum State;
  cmListFileContext Context;
};

bool TopIs(std::vector<NestingState>& stack, NestingStateEnum state)
{
  return !stack.empty() && stack.back().State == state;
}

class cmListFileParser
{
public:
  cmListFileParser(cmListFile* lf, cmListFileBacktrace lfbt,
                   cmMessenger* messenger);
  cmListFileParser(const cmListFileParser&) = delete;
  cmListFileParser& operator=(const cmListFileParser&) = delete;

  bool ParseFile(const char* filename);
  bool ParseString(const char* str, const char* virtual_filename);

private:
  bool Parse();
  bool ParseFunction(const char* name, long line);
  bool AddArgument(cmListFileLexer_Token* token,
                   cmListFileArgument::Delimiter delim);
  void IssueFileOpenError(std::string const& text) const;
  void IssueError(std::string const& text) const;

  cm::optional<cmListFileContext> CheckNesting() const;

  enum
  {
    SeparationOkay,
    SeparationWarning,
    SeparationError
  } Separation;

  cmListFile* ListFile;
  cmListFileBacktrace Backtrace;
  cmMessenger* Messenger;
  const char* FileName = nullptr;
  std::unique_ptr<cmListFileLexer, void (*)(cmListFileLexer*)> Lexer;
  std::string FunctionName;
  long FunctionLine;
  long FunctionLineEnd;
  std::vector<cmListFileArgument> FunctionArguments;
};

cmListFileParser::cmListFileParser(cmListFile* lf, cmListFileBacktrace lfbt,
                                   cmMessenger* messenger)
  : ListFile(lf)
  , Backtrace(std::move(lfbt))
  , Messenger(messenger)
  , Lexer(cmListFileLexer_New(), cmListFileLexer_Delete)
{
}

void cmListFileParser::IssueFileOpenError(const std::string& text) const
{
  this->Messenger->IssueMessage(MessageType::FATAL_ERROR, text,
                                this->Backtrace);
}

void cmListFileParser::IssueError(const std::string& text) const
{
  cmListFileContext lfc;
  lfc.FilePath = this->FileName;
  lfc.Line = cmListFileLexer_GetCurrentLine(this->Lexer.get());
  cmListFileBacktrace lfbt = this->Backtrace;
  lfbt = lfbt.Push(lfc);
  this->Messenger->IssueMessage(MessageType::FATAL_ERROR, text, lfbt);
  cmSystemTools::SetFatalErrorOccurred();
}

bool cmListFileParser::ParseFile(const char* filename)
{
  this->FileName = filename;

#ifdef _WIN32
  std::string expandedFileName = cmsys::Encoding::ToNarrow(
    cmSystemTools::ConvertToWindowsExtendedPath(filename));
  filename = expandedFileName.c_str();
#endif

  // Open the file.
  cmListFileLexer_BOM bom;
  if (!cmListFileLexer_SetFileName(this->Lexer.get(), filename, &bom)) {
    this->IssueFileOpenError("cmListFileCache: error can not open file.");
    return false;
  }

  if (bom == cmListFileLexer_BOM_Broken) {
    cmListFileLexer_SetFileName(this->Lexer.get(), nullptr, nullptr);
    this->IssueFileOpenError("Error while reading Byte-Order-Mark. "
                             "File not seekable?");
    return false;
  }

  // Verify the Byte-Order-Mark, if any.
  if (bom != cmListFileLexer_BOM_None && bom != cmListFileLexer_BOM_UTF8) {
    cmListFileLexer_SetFileName(this->Lexer.get(), nullptr, nullptr);
    this->IssueFileOpenError(
      "File starts with a Byte-Order-Mark that is not UTF-8.");
    return false;
  }

  return this->Parse();
}

bool cmListFileParser::ParseString(const char* str,
                                   const char* virtual_filename)
{
  this->FileName = virtual_filename;

  if (!cmListFileLexer_SetString(this->Lexer.get(), str)) {
    this->IssueFileOpenError("cmListFileCache: cannot allocate buffer.");
    return false;
  }

  return this->Parse();
}

bool cmListFileParser::Parse()
{
  // Use a simple recursive-descent parser to process the token
  // stream.
  bool haveNewline = true;
  while (cmListFileLexer_Token* token =
           cmListFileLexer_Scan(this->Lexer.get())) {
    if (token->type == cmListFileLexer_Token_Space) {
    } else if (token->type == cmListFileLexer_Token_Newline) {
      haveNewline = true;
    } else if (token->type == cmListFileLexer_Token_CommentBracket) {
      haveNewline = false;
    } else if (token->type == cmListFileLexer_Token_Identifier) {
      if (haveNewline) {
        haveNewline = false;
        if (this->ParseFunction(token->text, token->line)) {
          this->ListFile->Functions.emplace_back(
            std::move(this->FunctionName), this->FunctionLine,
            this->FunctionLineEnd, std::move(this->FunctionArguments));
        } else {
          return false;
        }
      } else {
        auto error = cmStrCat(
          "Parse error.  Expected a newline, got ",
          cmListFileLexer_GetTypeAsString(this->Lexer.get(), token->type),
          " with text \"", token->text, "\".");
        this->IssueError(error);
        return false;
      }
    } else {
      auto error = cmStrCat(
        "Parse error.  Expected a command name, got ",
        cmListFileLexer_GetTypeAsString(this->Lexer.get(), token->type),
        " with text \"", token->text, "\".");
      this->IssueError(error);
      return false;
    }
  }

  // Check if all functions are nested properly.
  if (auto badNesting = this->CheckNesting()) {
    this->Messenger->IssueMessage(
      MessageType::FATAL_ERROR,
      "Flow control statements are not properly nested.",
      this->Backtrace.Push(*badNesting));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  return true;
}

bool cmListFileParser::ParseFunction(const char* name, long line)
{
  // Ininitialize a new function call.
  this->FunctionName = name;
  this->FunctionLine = line;

  // Command name has already been parsed.  Read the left paren.
  cmListFileLexer_Token* token;
  while ((token = cmListFileLexer_Scan(this->Lexer.get())) &&
         token->type == cmListFileLexer_Token_Space) {
  }
  if (!token) {
    this->IssueError("Unexpected end of file.\n"
                     "Parse error.  Function missing opening \"(\".");
    return false;
  }
  if (token->type != cmListFileLexer_Token_ParenLeft) {
    auto error =
      cmStrCat("Parse error.  Expected \"(\", got ",
               cmListFileLexer_GetTypeAsString(this->Lexer.get(), token->type),
               " with text \"", token->text, "\".");
    this->IssueError(error);
    return false;
  }

  // Arguments.
  unsigned long parenDepth = 0;
  this->Separation = SeparationOkay;
  while ((token = cmListFileLexer_Scan(this->Lexer.get()))) {
    if (token->type == cmListFileLexer_Token_Space ||
        token->type == cmListFileLexer_Token_Newline) {
      this->Separation = SeparationOkay;
      continue;
    }
    if (token->type == cmListFileLexer_Token_ParenLeft) {
      parenDepth++;
      this->Separation = SeparationOkay;
      if (!this->AddArgument(token, cmListFileArgument::Unquoted)) {
        return false;
      }
    } else if (token->type == cmListFileLexer_Token_ParenRight) {
      if (parenDepth == 0) {
        this->FunctionLineEnd = token->line;
        return true;
      }
      parenDepth--;
      this->Separation = SeparationOkay;
      if (!this->AddArgument(token, cmListFileArgument::Unquoted)) {
        return false;
      }
      this->Separation = SeparationWarning;
    } else if (token->type == cmListFileLexer_Token_Identifier ||
               token->type == cmListFileLexer_Token_ArgumentUnquoted) {
      if (!this->AddArgument(token, cmListFileArgument::Unquoted)) {
        return false;
      }
      this->Separation = SeparationWarning;
    } else if (token->type == cmListFileLexer_Token_ArgumentQuoted) {
      if (!this->AddArgument(token, cmListFileArgument::Quoted)) {
        return false;
      }
      this->Separation = SeparationWarning;
    } else if (token->type == cmListFileLexer_Token_ArgumentBracket) {
      if (!this->AddArgument(token, cmListFileArgument::Bracket)) {
        return false;
      }
      this->Separation = SeparationError;
    } else if (token->type == cmListFileLexer_Token_CommentBracket) {
      this->Separation = SeparationError;
    } else {
      // Error.
      auto error = cmStrCat(
        "Parse error.  Function missing ending \")\".  "
        "Instead found ",
        cmListFileLexer_GetTypeAsString(this->Lexer.get(), token->type),
        " with text \"", token->text, "\".");
      this->IssueError(error);
      return false;
    }
  }

  cmListFileContext lfc;
  lfc.FilePath = this->FileName;
  lfc.Line = line;
  cmListFileBacktrace lfbt = this->Backtrace;
  lfbt = lfbt.Push(lfc);
  this->Messenger->IssueMessage(
    MessageType::FATAL_ERROR,
    "Parse error.  Function missing ending \")\".  "
    "End of file reached.",
    lfbt);
  return false;
}

bool cmListFileParser::AddArgument(cmListFileLexer_Token* token,
                                   cmListFileArgument::Delimiter delim)
{
  this->FunctionArguments.emplace_back(token->text, delim, token->line);
  if (this->Separation == SeparationOkay) {
    return true;
  }
  bool isError = (this->Separation == SeparationError ||
                  delim == cmListFileArgument::Bracket);
  cmListFileContext lfc;
  lfc.FilePath = this->FileName;
  lfc.Line = token->line;
  cmListFileBacktrace lfbt = this->Backtrace;
  lfbt = lfbt.Push(lfc);
  auto msg =
    cmStrCat("Syntax ", (isError ? "Error" : "Warning"),
             " in cmake code at column ", token->column,
             "\n"
             "Argument not separated from preceding token by whitespace.");
  if (isError) {
    this->Messenger->IssueMessage(MessageType::FATAL_ERROR, msg, lfbt);
    return false;
  }
  this->Messenger->IssueMessage(MessageType::AUTHOR_WARNING, msg, lfbt);
  return true;
}

cm::optional<cmListFileContext> cmListFileParser::CheckNesting() const
{
  std::vector<NestingState> stack;

  for (auto const& func : this->ListFile->Functions) {
    auto const& name = func.LowerCaseName();
    if (name == "if") {
      stack.push_back({
        NestingStateEnum::If,
        cmListFileContext::FromListFileFunction(func, this->FileName),
      });
    } else if (name == "elseif") {
      if (!TopIs(stack, NestingStateEnum::If)) {
        return cmListFileContext::FromListFileFunction(func, this->FileName);
      }
      stack.back() = {
        NestingStateEnum::If,
        cmListFileContext::FromListFileFunction(func, this->FileName),
      };
    } else if (name == "else") {
      if (!TopIs(stack, NestingStateEnum::If)) {
        return cmListFileContext::FromListFileFunction(func, this->FileName);
      }
      stack.back() = {
        NestingStateEnum::Else,
        cmListFileContext::FromListFileFunction(func, this->FileName),
      };
    } else if (name == "endif") {
      if (!TopIs(stack, NestingStateEnum::If) &&
          !TopIs(stack, NestingStateEnum::Else)) {
        return cmListFileContext::FromListFileFunction(func, this->FileName);
      }
      stack.pop_back();
    } else if (name == "while") {
      stack.push_back({
        NestingStateEnum::While,
        cmListFileContext::FromListFileFunction(func, this->FileName),
      });
    } else if (name == "endwhile") {
      if (!TopIs(stack, NestingStateEnum::While)) {
        return cmListFileContext::FromListFileFunction(func, this->FileName);
      }
      stack.pop_back();
    } else if (name == "foreach") {
      stack.push_back({
        NestingStateEnum::Foreach,
        cmListFileContext::FromListFileFunction(func, this->FileName),
      });
    } else if (name == "endforeach") {
      if (!TopIs(stack, NestingStateEnum::Foreach)) {
        return cmListFileContext::FromListFileFunction(func, this->FileName);
      }
      stack.pop_back();
    } else if (name == "function") {
      stack.push_back({
        NestingStateEnum::Function,
        cmListFileContext::FromListFileFunction(func, this->FileName),
      });
    } else if (name == "endfunction") {
      if (!TopIs(stack, NestingStateEnum::Function)) {
        return cmListFileContext::FromListFileFunction(func, this->FileName);
      }
      stack.pop_back();
    } else if (name == "macro") {
      stack.push_back({
        NestingStateEnum::Macro,
        cmListFileContext::FromListFileFunction(func, this->FileName),
      });
    } else if (name == "endmacro") {
      if (!TopIs(stack, NestingStateEnum::Macro)) {
        return cmListFileContext::FromListFileFunction(func, this->FileName);
      }
      stack.pop_back();
    } else if (name == "block") {
      stack.push_back({
        NestingStateEnum::Block,
        cmListFileContext::FromListFileFunction(func, this->FileName),
      });
    } else if (name == "endblock") {
      if (!TopIs(stack, NestingStateEnum::Block)) {
        return cmListFileContext::FromListFileFunction(func, this->FileName);
      }
      stack.pop_back();
    }
  }

  if (!stack.empty()) {
    return stack.back().Context;
  }

  return cm::nullopt;
}

} // anonymous namespace

bool cmListFile::ParseFile(const char* filename, cmMessenger* messenger,
                           cmListFileBacktrace const& lfbt)
{
  if (!cmSystemTools::FileExists(filename) ||
      cmSystemTools::FileIsDirectory(filename)) {
    return false;
  }

  bool parseError = false;

  {
    cmListFileParser parser(this, lfbt, messenger);
    parseError = !parser.ParseFile(filename);
  }

  return !parseError;
}

bool cmListFile::ParseString(const char* str, const char* virtual_filename,
                             cmMessenger* messenger,
                             const cmListFileBacktrace& lfbt)
{
  bool parseError = false;

  {
    cmListFileParser parser(this, lfbt, messenger);
    parseError = !parser.ParseString(str, virtual_filename);
  }

  return !parseError;
}

#include "cmConstStack.tcc"
template class cmConstStack<cmListFileContext, cmListFileBacktrace>;

std::ostream& operator<<(std::ostream& os, cmListFileContext const& lfc)
{
  os << lfc.FilePath;
  if (lfc.Line > 0) {
    os << ':' << lfc.Line;
    if (!lfc.Name.empty()) {
      os << " (" << lfc.Name << ')';
    }
  } else if (lfc.Line == cmListFileContext::DeferPlaceholderLine) {
    os << ":DEFERRED";
  }
  return os;
}

bool operator<(const cmListFileContext& lhs, const cmListFileContext& rhs)
{
  if (lhs.Line != rhs.Line) {
    return lhs.Line < rhs.Line;
  }
  return lhs.FilePath < rhs.FilePath;
}

bool operator==(const cmListFileContext& lhs, const cmListFileContext& rhs)
{
  return lhs.Line == rhs.Line && lhs.FilePath == rhs.FilePath;
}

bool operator!=(const cmListFileContext& lhs, const cmListFileContext& rhs)
{
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, BT<std::string> const& s)
{
  return os << s.Value;
}

std::vector<BT<std::string>> cmExpandListWithBacktrace(
  std::string const& list, cmListFileBacktrace const& bt,
  cmList::EmptyElements emptyArgs)
{
  std::vector<BT<std::string>> result;
  cmList tmp{ list, emptyArgs };
  result.reserve(tmp.size());
  for (std::string& i : tmp) {
    result.emplace_back(std::move(i), bt);
  }
  return result;
}
