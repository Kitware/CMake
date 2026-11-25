/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmListFileCoverage.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <memory>
#include <utility>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/uv.h>

#include "cmsys/FStream.hxx"

#include "cmGeneratedFileStream.h"
#include "cmListFileCache.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {

std::unique_ptr<cmListFileCoverage> Instance;

bool IsExecutable(cmListFileFunction const& lff)
{
  if (lff.Line() <= 0) {
    return false;
  }
  static constexpr std::array<cm::string_view, 7> notExecutable{
    { "else"_s, "endif"_s, "endforeach"_s, "endwhile"_s, "endfunction"_s,
      "endmacro"_s, "endblock"_s }
  };
  return !cm::contains(notExecutable, lff.LowerCaseName());
}
}

cmListFileCoverage::cmListFileCoverage(std::string outputPath)
  : OutputPath(std::move(outputPath))
{
}

bool cmListFileCoverage::Start(std::string& error)
{
  if (Instance) {
    return true;
  }
  cm::optional<std::string> const value =
    cmSystemTools::GetEnvVar("CMAKE_LISTFILE_COVERAGE_DIR");
  if (!value || value->empty()) {
    return true;
  }
  std::string const& dir = *value;
  if (!cmSystemTools::FileIsFullPath(dir)) {
    error = cmStrCat(
      "CMAKE_LISTFILE_COVERAGE_DIR is not an absolute path: \""_s, dir, '"');
    return false;
  }
  cmsys::Status const status = cmSystemTools::MakeDirectory(dir);
  if (!status) {
    error = cmStrCat("Cannot write CMake listfile coverage to directory \""_s,
                     dir, "\": "_s, status.GetString());
    return false;
  }

  std::array<char, 9> token{};
  std::snprintf(token.data(), token.size(), "%08x",
                static_cast<unsigned int>(cmSystemTools::RandomNumber()));
  std::string base = dir;
  cmSystemTools::ConvertToUnixSlashes(base);
  if (base.back() != '/') {
    base += '/';
  }
  std::string outputPath =
    cmStrCat(base, static_cast<long long>(uv_os_getpid()), '-', token.data(),
             ".info"_s);

  std::string const probePath = cmStrCat(outputPath, ".probe"_s);
  {
    cmsys::ofstream probe(probePath.c_str());
    if (!probe) {
      error =
        cmStrCat("Cannot write CMake listfile coverage to directory \""_s, dir,
                 "\": "_s, cmSystemTools::GetLastSystemError());
      return false;
    }
  }
  cmSystemTools::RemoveFile(probePath);

  Instance.reset(new cmListFileCoverage(std::move(outputPath)));
  return true;
}

cmListFileCoverage* cmListFileCoverage::Get()
{
  return Instance.get();
}

void cmListFileCoverage::Finish()
{
  if (!Instance) {
    return;
  }
  // A tracefile with no SF record is not valid, so write nothing then.
  cmLcovTracefile const tracefile = Instance->ToTracefile();
  if (tracefile.HasData()) {
    cmGeneratedFileStream fout(Instance->OutputPath, true);
    tracefile.Write(fout);
  }
  Instance.reset();
}

cmListFileCoverage::SyntheticCommandScope::SyntheticCommandScope(
  cmListFileFunction const& lff)
{
  cmListFileCoverage* cov = cmListFileCoverage::Get();
  if (cov && cov->SyntheticCommands.insert(&lff).second) {
    this->Command = &lff;
  }
}

cmListFileCoverage::SyntheticCommandScope::~SyntheticCommandScope()
{
  cmListFileCoverage* cov = cmListFileCoverage::Get();
  if (cov && this->Command) {
    cov->SyntheticCommands.erase(this->Command);
  }
}

void cmListFileCoverage::FileParsed(
  std::string const& path, std::vector<cmListFileFunction> const& functions)
{
  cmLcovSourceFile& file =
    this->Files[cmSystemTools::FileIsFullPath(path)
                  ? path
                  : cmSystemTools::CollapseFullPath(path)];
  for (long line : ExecutableLines(functions)) {
    file.Lines.emplace(line, 0);
  }
  for (long line : DecisionLines(functions)) {
    file.Branches.emplace(cmLcovBranchId{ line, 0, 0 }, cm::nullopt);
    file.Branches.emplace(cmLcovBranchId{ line, 0, 1 }, cm::nullopt);
  }
  for (cmListFileFunction const& lff : functions) {
    std::string const& name = lff.LowerCaseName();
    if (lff.Line() > 0 && (name == "function"_s || name == "macro"_s) &&
        !lff.Arguments().empty()) {
      file.Functions.emplace(lff.Line(),
                             cmLcovFunction(lff.Arguments().front().Value));
    }
  }
}

void cmListFileCoverage::CommandRan(cmListFileFunction const& lff,
                                    std::string const& path, long line)
{
  if (this->SyntheticCommands.count(&lff)) {
    return;
  }
  auto file = this->FindFile(path);
  if (file == this->Files.end()) {
    return;
  }
  auto hits = file->second.Lines.find(line);
  if (hits != file->second.Lines.end()) {
    ++hits->second;
  }
}

void cmListFileCoverage::FunctionCalled(std::string const& path,
                                        long definitionLine)
{
  auto file = this->FindFile(path);
  if (file == this->Files.end()) {
    return;
  }
  auto fn = file->second.Functions.find(definitionLine);
  if (fn != file->second.Functions.end()) {
    ++fn->second.Hits;
  }
}

void cmListFileCoverage::BranchTaken(std::string const& path, long line,
                                     unsigned int branch)
{
  if (cmSystemTools::GetFatalErrorOccurred()) {
    return;
  }
  auto file = this->FindFile(path);
  if (file == this->Files.end()) {
    return;
  }
  auto taken = file->second.Branches.find({ line, 0, branch });
  auto other = file->second.Branches.find({ line, 0, branch ^ 1u });
  if (taken == file->second.Branches.end() ||
      other == file->second.Branches.end()) {
    return;
  }
  taken->second = taken->second.value_or(0) + 1;
  if (!other->second) {
    other->second = 0;
  }
}

std::map<std::string, cmLcovSourceFile>::iterator cmListFileCoverage::FindFile(
  std::string const& path)
{
  if (cmSystemTools::FileIsFullPath(path)) {
    return this->Files.find(path);
  }
  return this->Files.find(cmSystemTools::CollapseFullPath(path));
}

cmLcovTracefile cmListFileCoverage::ToTracefile() const
{
  cmLcovTracefile tracefile;
  tracefile.Files = this->Files;
  return tracefile;
}

std::vector<long> cmListFileCoverage::ExecutableLines(
  std::vector<cmListFileFunction> const& functions)
{
  std::vector<long> lines;
  for (cmListFileFunction const& lff : functions) {
    if (IsExecutable(lff)) {
      lines.push_back(lff.Line());
    }
  }
  std::sort(lines.begin(), lines.end());
  lines.erase(std::unique(lines.begin(), lines.end()), lines.end());
  return lines;
}

std::vector<long> cmListFileCoverage::DecisionLines(
  std::vector<cmListFileFunction> const& functions)
{
  std::vector<long> lines;
  for (cmListFileFunction const& lff : functions) {
    std::string const& name = lff.LowerCaseName();
    if (lff.Line() > 0 &&
        (name == "if"_s || name == "elseif"_s || name == "while"_s ||
         name == "foreach"_s)) {
      lines.push_back(lff.Line());
    }
  }
  std::sort(lines.begin(), lines.end());
  lines.erase(std::unique(lines.begin(), lines.end()), lines.end());
  return lines;
}
