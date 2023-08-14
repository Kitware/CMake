/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

/* This code was originally taken from part of the Clang-Tidy LLVM project and
 * modified for use with CMake under the following original license: */

//===--- HeaderGuard.cpp - clang-tidy
//-------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions. See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UsePragmaOnceCheck.h"

#include <algorithm>
#include <cassert>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/Path.h>

namespace clang {
namespace tidy {
namespace cmake {

/// canonicalize a path by removing ./ and ../ components.
static std::string cleanPath(StringRef Path)
{
  SmallString<256> Result = Path;
  llvm::sys::path::remove_dots(Result, true);
  return std::string(Result.str());
}

namespace {
// This class is a workaround for the fact that PPCallbacks doesn't give us the
// location of the hash for an #ifndef, #define, or #endif, so we have to find
// it ourselves. We can't lex backwards, and attempting to turn on the
// preprocessor's backtracking functionality wreaks havoc, so we have to
// instantiate a second lexer and lex all the way from the beginning of the
// file. Cache the results of this lexing so that we don't have to do it more
// times than needed.
//
// TODO: Upstream a change to LLVM to give us the location of the hash in
// PPCallbacks so we don't have to do this workaround.
class DirectiveCache
{
public:
  DirectiveCache(Preprocessor* PP, FileID FID)
    : PP(PP)
    , FID(FID)
  {
    SourceManager& SM = this->PP->getSourceManager();
    const FileEntry* Entry = SM.getFileEntryForID(FID);
    assert(Entry && "Invalid FileID given");

    Lexer MyLexer(FID, SM.getMemoryBufferForFileOrFake(Entry), SM,
                  this->PP->getLangOpts());
    Token Tok;

    while (!MyLexer.LexFromRawLexer(Tok)) {
      if (Tok.getKind() == tok::hash) {
        assert(SM.getFileID(Tok.getLocation()) == this->FID &&
               "Token FileID does not match passed FileID");
        if (!this->HashLocs.empty()) {
          assert(SM.getFileOffset(this->HashLocs.back()) <
                   SM.getFileOffset(Tok.getLocation()) &&
                 "Tokens in file are not in order");
        }

        this->HashLocs.push_back(Tok.getLocation());
      }
    }
  }

  SourceRange createRangeForIfndef(SourceLocation IfndefMacroTokLoc)
  {
    // The #ifndef of an include guard is likely near the beginning of the
    // file, so search from the front.
    return SourceRange(this->findPreviousHashFromFront(IfndefMacroTokLoc),
                       IfndefMacroTokLoc);
  }

  SourceRange createRangeForDefine(SourceLocation DefineMacroTokLoc)
  {
    // The #define of an include guard is likely near the beginning of the
    // file, so search from the front.
    return SourceRange(this->findPreviousHashFromFront(DefineMacroTokLoc),
                       DefineMacroTokLoc);
  }

  SourceRange createRangeForEndif(SourceLocation EndifLoc)
  {
    // The #endif of an include guard is likely near the end of the file, so
    // search from the back.
    return SourceRange(this->findPreviousHashFromBack(EndifLoc), EndifLoc);
  }

private:
  Preprocessor* PP;
  FileID FID;
  SmallVector<SourceLocation> HashLocs;

  SourceLocation findPreviousHashFromFront(SourceLocation Loc)
  {
    SourceManager& SM = this->PP->getSourceManager();
    Loc = SM.getExpansionLoc(Loc);
    assert(SM.getFileID(Loc) == this->FID &&
           "Loc FileID does not match our FileID");

    auto It = std::find_if(
      this->HashLocs.begin(), this->HashLocs.end(),
      [&SM, &Loc](const SourceLocation& OtherLoc) -> bool {
        return SM.getFileOffset(OtherLoc) >= SM.getFileOffset(Loc);
      });
    assert(It != this->HashLocs.begin() &&
           "No hash associated with passed Loc");
    return *--It;
  }

  SourceLocation findPreviousHashFromBack(SourceLocation Loc)
  {
    SourceManager& SM = this->PP->getSourceManager();
    Loc = SM.getExpansionLoc(Loc);
    assert(SM.getFileID(Loc) == this->FID &&
           "Loc FileID does not match our FileID");

    auto It =
      std::find_if(this->HashLocs.rbegin(), this->HashLocs.rend(),
                   [&SM, &Loc](const SourceLocation& OtherLoc) -> bool {
                     return SM.getFileOffset(OtherLoc) < SM.getFileOffset(Loc);
                   });
    assert(It != this->HashLocs.rend() &&
           "No hash associated with passed Loc");
    return *It;
  }
};

class UsePragmaOncePPCallbacks : public PPCallbacks
{
public:
  UsePragmaOncePPCallbacks(Preprocessor* PP, UsePragmaOnceCheck* Check)
    : PP(PP)
    , Check(Check)
  {
  }

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override
  {
    // Record all files we enter. We'll need them to diagnose headers without
    // guards.
    SourceManager& SM = this->PP->getSourceManager();
    if (Reason == EnterFile && FileType == SrcMgr::C_User) {
      if (const FileEntry* FE = SM.getFileEntryForID(SM.getFileID(Loc))) {
        std::string FileName = cleanPath(FE->getName());
        this->Files[FileName] = FE;
      }
    }
  }

  void Ifndef(SourceLocation Loc, const Token& MacroNameTok,
              const MacroDefinition& MD) override
  {
    if (MD) {
      return;
    }

    // Record #ifndefs that succeeded. We also need the Location of the Name.
    this->Ifndefs[MacroNameTok.getIdentifierInfo()] =
      std::make_pair(Loc, MacroNameTok.getLocation());
  }

  void MacroDefined(const Token& MacroNameTok,
                    const MacroDirective* MD) override
  {
    // Record all defined macros. We store the whole token to get info on the
    // name later.
    this->Macros.emplace_back(MacroNameTok, MD->getMacroInfo());
  }

  void Endif(SourceLocation Loc, SourceLocation IfLoc) override
  {
    // Record all #endif and the corresponding #ifs (including #ifndefs).
    this->EndIfs[IfLoc] = Loc;
  }

  void EndOfMainFile() override
  {
    // Now that we have all this information from the preprocessor, use it!
    SourceManager& SM = this->PP->getSourceManager();

    for (const auto& MacroEntry : this->Macros) {
      const MacroInfo* MI = MacroEntry.second;

      // We use clang's header guard detection. This has the advantage of also
      // emitting a warning for cases where a pseudo header guard is found but
      // preceded by something blocking the header guard optimization.
      if (!MI->isUsedForHeaderGuard()) {
        continue;
      }

      const FileEntry* FE =
        SM.getFileEntryForID(SM.getFileID(MI->getDefinitionLoc()));
      std::string FileName = cleanPath(FE->getName());
      this->Files.erase(FileName);

      // Look up Locations for this guard.
      SourceLocation Ifndef =
        this->Ifndefs[MacroEntry.first.getIdentifierInfo()].second;
      SourceLocation Define = MacroEntry.first.getLocation();
      SourceLocation EndIf =
        this
          ->EndIfs[this->Ifndefs[MacroEntry.first.getIdentifierInfo()].first];

      std::vector<FixItHint> FixIts;

      HeaderSearch& HeaderInfo = this->PP->getHeaderSearchInfo();

      HeaderFileInfo& Info = HeaderInfo.getFileInfo(FE);

      DirectiveCache Cache(this->PP, SM.getFileID(MI->getDefinitionLoc()));
      SourceRange IfndefSrcRange = Cache.createRangeForIfndef(Ifndef);
      SourceRange DefineSrcRange = Cache.createRangeForDefine(Define);
      SourceRange EndifSrcRange = Cache.createRangeForEndif(EndIf);

      if (Info.isPragmaOnce) {
        FixIts.push_back(FixItHint::CreateRemoval(IfndefSrcRange));
      } else {
        FixIts.push_back(
          FixItHint::CreateReplacement(IfndefSrcRange, "#pragma once"));
      }

      FixIts.push_back(FixItHint::CreateRemoval(DefineSrcRange));
      FixIts.push_back(FixItHint::CreateRemoval(EndifSrcRange));

      this->Check->diag(IfndefSrcRange.getBegin(), "use #pragma once")
        << FixIts;
    }

    // Emit warnings for headers that are missing guards.
    checkGuardlessHeaders();
    clearAllState();
  }

  /// Looks for files that were visited but didn't have a header guard.
  /// Emits a warning with fixits suggesting adding one.
  void checkGuardlessHeaders()
  {
    // Look for header files that didn't have a header guard. Emit a warning
    // and fix-its to add the guard.
    // TODO: Insert the guard after top comments.
    for (const auto& FE : this->Files) {
      StringRef FileName = FE.getKey();
      if (!Check->shouldSuggestToAddPragmaOnce(FileName)) {
        continue;
      }

      SourceManager& SM = this->PP->getSourceManager();
      FileID FID = SM.translateFile(FE.getValue());
      SourceLocation StartLoc = SM.getLocForStartOfFile(FID);
      if (StartLoc.isInvalid()) {
        continue;
      }

      HeaderSearch& HeaderInfo = this->PP->getHeaderSearchInfo();

      HeaderFileInfo& Info = HeaderInfo.getFileInfo(FE.second);
      if (Info.isPragmaOnce) {
        continue;
      }

      this->Check->diag(StartLoc, "use #pragma once")
        << FixItHint::CreateInsertion(StartLoc, "#pragma once\n");
    }
  }

private:
  void clearAllState()
  {
    this->Macros.clear();
    this->Files.clear();
    this->Ifndefs.clear();
    this->EndIfs.clear();
  }

  std::vector<std::pair<Token, const MacroInfo*>> Macros;
  llvm::StringMap<const FileEntry*> Files;
  std::map<const IdentifierInfo*, std::pair<SourceLocation, SourceLocation>>
    Ifndefs;
  std::map<SourceLocation, SourceLocation> EndIfs;

  Preprocessor* PP;
  UsePragmaOnceCheck* Check;
};
} // namespace

void UsePragmaOnceCheck::storeOptions(ClangTidyOptions::OptionMap& Opts)
{
  this->Options.store(Opts, "HeaderFileExtensions",
                      RawStringHeaderFileExtensions);
}

void UsePragmaOnceCheck::registerPPCallbacks(const SourceManager& SM,
                                             Preprocessor* PP,
                                             Preprocessor* ModuleExpanderPP)
{
  PP->addPPCallbacks(std::make_unique<UsePragmaOncePPCallbacks>(PP, this));
}

bool UsePragmaOnceCheck::shouldSuggestToAddPragmaOnce(StringRef FileName)
{
  return utils::isFileExtension(FileName, this->HeaderFileExtensions);
}

} // namespace cmake
} // namespace tidy
} // namespace clang
