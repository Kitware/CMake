
#include "cmServerParser.h"

#include "cmCommand.h"
#include "cmListFileLexer.h"
#include "cmServerDiff.h"
#include "cmSystemTools.h"

cmServerParser::cmServerParser(cmState* state, const std::string& fileName,
                               const std::string& rootDir)
  : Lexer(cmListFileLexer_New())
  , FileName(fileName)
  , mRootDir(rootDir)
  , State(state)
{
  this->IsString = false;
}

cmServerParser::~cmServerParser()
{
  cmListFileLexer_Delete(this->Lexer);
}

int cmServerParser::TransformLine(long line, DifferentialFileContent diff)
{
  auto const& chunks = diff.Chunks;

  auto chunkIt = std::lower_bound(
    chunks.begin(), chunks.end(), line,
    [](const Chunk& lhs, long rhs) { return lhs.OrigStart < rhs; });
  if (chunkIt == chunks.end() || chunkIt->OrigStart != line) {
    --chunkIt;
  }

  if (!this->IsString) {
    while (chunkIt->NumAdded != 0 || chunkIt->NumRemoved != 0) {
      ++chunkIt;
    }
    if (chunkIt->OrigStart > line) {
      return -1;
    }
  }

  long offset = chunkIt->NewStart - chunkIt->OrigStart;

  return line + offset;
}

void cmServerParser::Add(cmListFileLexer_Token* token,
                         std::vector<Token>& value,
                         DifferentialFileContent diff, const char* name,
                         cmCommand* cmd, std::vector<std::string> const& args)
{
  auto ln = this->TransformLine(token->line, diff);
  if (ln == -1) {
    return;
  }
  Token t;
  if (!cmd || args.empty()) {
    t.Type =
      name ? name : cmListFileLexer_GetTypeAsString(this->Lexer, token->type);
  } else {
    auto ctx = cmd->GetContextForParameter(args, args.size() - 1);
    switch (ctx) {
      case cmCommand::KeywordParameter:
        t.Type = "keyword";
        break;
      case cmCommand::SingleBinaryTargetParameter:
        t.Type = "target_name";
        break;
      default:
        t.Type = name ? name : cmListFileLexer_GetTypeAsString(this->Lexer,
                                                               token->type);
        break;
    }
  }
  t.Line = ln;
  t.Column = token->column;
  t.Length = token->length;
  value.push_back(t);
}

Json::Value cmServerParser::Parse(DifferentialFileContent diff)
{
  cmListFileLexer_BOM bom;
  if (!cmListFileLexer_SetFileName(this->Lexer, this->FileName.c_str(),
                                   &bom)) {
    cmSystemTools::Error("cmListFileCache: error can not open file ",
                         this->FileName.c_str());
    return false;
  }

  // Verify the Byte-Order-Mark, if any.
  if (bom != cmListFileLexer_BOM_None && bom != cmListFileLexer_BOM_UTF8) {
    return false;
  }

  std::vector<Token> tokens;
  Scan(diff, tokens);

  for (auto const& ch : diff.Chunks) {
    if (ch.NumAdded != 0) {
      long startLine = 1;
      long endLine = ch.NewStart + ch.NumAdded;
      auto insertionPoint = tokens.begin();
      if (ch.NewStart > tokens.front().Line) {
        auto lb = std::lower_bound(
          tokens.begin(), tokens.end(), ch.NewStart,
          [](Token const& lhs, long rhs) { return lhs.Line < rhs; });
        if (lb == tokens.end() || lb->Line != ch.NewStart) {
          --lb;
        }
        while (lb != tokens.begin()) {
          if (lb->Type == "command")
            break;
          --lb;
        }
        auto fileStartLine = lb->Line;

        auto itToExecFrom = std::lower_bound(
          diff.Chunks.begin(), diff.Chunks.end(), fileStartLine,
          [](Chunk const& lhs, long rhs) { return lhs.OrigStart < rhs; });
        if (itToExecFrom == diff.Chunks.end() ||
            itToExecFrom->OrigStart != fileStartLine) {
          --itToExecFrom;
        }

        auto offset = fileStartLine - itToExecFrom->OrigStart;
        startLine = itToExecFrom->NewStart + offset;

        auto lend = lb;
        while (lend != tokens.end() && lend->Line == lb->Line) {
          ++lend;
        }
        insertionPoint = tokens.erase(lb, lend);
      }

      auto prParseStart = diff.EditorLines.begin() + startLine - 1;
      auto prParseEnd = diff.EditorLines.begin() + endLine - 1;

      auto newString = cmJoin(cmMakeRange(prParseStart, prParseEnd), "\n");

      cmListFileLexer_Delete(this->Lexer);
      this->Lexer = cmListFileLexer_New();
      if (!cmListFileLexer_SetString(this->Lexer, newString.c_str())) {
        return false;
      }
      this->IsString = true;
      std::vector<Token> chunkTokens;
      this->Scan(diff, chunkTokens);
      assert(startLine > 0);
      for (auto& t : chunkTokens) {
        t.Line += startLine - 1;
      }

      tokens.insert(insertionPoint, chunkTokens.begin(), chunkTokens.end());
    }
  }

  Json::Value ret = Json::arrayValue;

  for (auto const& t : tokens) {
    Json::Value obj = Json::objectValue;
    obj["type"] = t.Type;
    obj["line"] = t.Line;
    obj["column"] = t.Column;
    obj["length"] = t.Length;
    ret.append(obj);
  }

  return ret;
}

cmListFileLexer_Token* cmServerParser::Scan()
{
  if (this->IsString) {
    return cmListFileLexer_ScanString(this->Lexer);
  }
  return cmListFileLexer_Scan(this->Lexer);
}

void cmServerParser::Scan(DifferentialFileContent diff,
                          std::vector<Token>& ret)
{
  bool haveNewline = true;
  while (cmListFileLexer_Token* token = this->Scan()) {
    if (token->type == cmListFileLexer_Token_Space) {
    } else if (token->type == cmListFileLexer_Token_Newline) {
      haveNewline = true;
    } else if (token->type == cmListFileLexer_Token_CommentBracket) {
      haveNewline = false;
    } else if (token->type == cmListFileLexer_Token_Identifier) {
      if (haveNewline) {
        haveNewline = false;
        std::string helpFile = mRootDir + "/Help/command/";
        helpFile += token->text;
        helpFile += ".rst";
        if (cmSystemTools::FileExists(helpFile.c_str(), true)) {
          this->Add(token, ret, diff, "command");
        } else {
          this->Add(token, ret, diff, "user_command");
        }
        if (!this->ParseFunction(token, ret, diff)) {
          return;
        }
      }
    }
  }
}

bool cmServerParser::ParseFunction(cmListFileLexer_Token* incomingToken,
                                   std::vector<Token>& ret,
                                   DifferentialFileContent diff)
{
  std::string name = incomingToken->text;

  cmListFileLexer_Token* token;
  while ((token = this->Scan()) &&
         token->type == cmListFileLexer_Token_Space) {
  }

  if (!token) {
    return false;
  }

  this->Add(token, ret, diff);

  unsigned long parenDepth = 0;
  this->Separation = SeparationOkay;

  cmCommand* cmd = this->State->GetCommand(name);

  std::vector<std::string> args;

  while ((token = this->Scan())) {
    if (token->type == cmListFileLexer_Token_Space ||
        token->type == cmListFileLexer_Token_Newline) {
      this->Separation = SeparationOkay;
      continue;
    }
    if (token->type == cmListFileLexer_Token_ParenLeft) {
      parenDepth++;
      this->Separation = SeparationOkay;
      this->Add(token, ret, diff);
      if (!this->AddArgument()) {
        return false;
      }
    } else if (token->type == cmListFileLexer_Token_ParenRight) {
      this->Add(token, ret, diff);
      if (parenDepth == 0) {
        return true;
      }
      parenDepth--;
      this->Separation = SeparationOkay;
      if (!this->AddArgument()) {
        return false;
      }
      this->Separation = SeparationWarning;
    } else if (token->type == cmListFileLexer_Token_Identifier ||
               token->type == cmListFileLexer_Token_ArgumentUnquoted) {
      args.push_back(token->text);
      this->Add(token, ret, diff, 0, cmd, args);
      if (!this->AddArgument()) {
        return false;
      }
      this->Separation = SeparationWarning;
    } else if (token->type == cmListFileLexer_Token_ArgumentQuoted) {
      args.push_back(token->text);
      this->Add(token, ret, diff, 0, cmd, args);
      if (!this->AddArgument()) {
        return false;
      }
      this->Separation = SeparationWarning;
    } else if (token->type == cmListFileLexer_Token_ArgumentBracket) {
      args.push_back(token->text);
      this->Add(token, ret, diff, 0, cmd, args);
      if (!this->AddArgument()) {
        return false;
      }
      this->Separation = SeparationError;
    } else if (token->type == cmListFileLexer_Token_CommentBracket) {
      this->Separation = SeparationError;
    }
  }
  return true;
}

bool cmServerParser::AddArgument()
{
  if (this->Separation == SeparationOkay) {
    return true;
  }
  return false;
}
