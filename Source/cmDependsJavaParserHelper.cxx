/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDependsJavaParserHelper.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <utility>

#include <cm/memory>
#include <cm/string_view>

#include "cmsys/FStream.hxx"

#include "cmDependsJavaLexer.h"
#include "cmSystemTools.h"

int cmDependsJava_yyparse(yyscan_t yyscanner);

cmDependsJavaParserHelper::cmDependsJavaParserHelper()
{
  this->CurrentDepth = 0;

  this->UnionsAvailable = 0;
  this->LastClassId = 0;

  CurrentClass tl;
  tl.Name = "*";
  this->ClassStack.push_back(std::move(tl));
}

cmDependsJavaParserHelper::~cmDependsJavaParserHelper()
{
  this->CleanupParser();
}

void cmDependsJavaParserHelper::CurrentClass::AddFileNamesForPrinting(
  std::vector<std::string>* files, const char* prefix, const char* sep) const
{
  std::string rname;
  if (prefix) {
    rname += prefix;
    rname += sep;
  }
  rname += this->Name;
  files->push_back(rname);
  for (CurrentClass const& nc : this->NestedClasses) {
    nc.AddFileNamesForPrinting(files, rname.c_str(), sep);
  }
}

void cmDependsJavaParserHelper::DeallocateParserType(char** pt)
{
  if (!pt) {
    return;
  }
  if (!*pt) {
    return;
  }
  *pt = nullptr;
  this->UnionsAvailable--;
}

void cmDependsJavaParserHelper::AddClassFound(const char* sclass)
{
  if (!sclass) {
    return;
  }
  for (std::string const& cf : this->ClassesFound) {
    if (cf == sclass) {
      return;
    }
  }
  this->ClassesFound.emplace_back(sclass);
}

void cmDependsJavaParserHelper::AddPackagesImport(const char* sclass)
{
  for (std::string const& pi : this->PackagesImport) {
    if (pi == sclass) {
      return;
    }
  }
  this->PackagesImport.emplace_back(sclass);
}

void cmDependsJavaParserHelper::SafePrintMissing(const char* str, int line,
                                                 int cnt)
{
  if (str) {
    std::cout << line << " String " << cnt << " exists: ";
    unsigned int cc;
    for (cc = 0; cc < strlen(str); cc++) {
      unsigned char ch = str[cc];
      if (ch >= 32 && ch <= 126) {
        std::cout << static_cast<char>(ch);
      } else {
        std::cout << "<" << static_cast<int>(ch) << ">";
        break;
      }
    }
    std::cout << "- " << strlen(str) << std::endl;
  }
}
void cmDependsJavaParserHelper::Print(const char* place, const char* str)
{
  if (this->Verbose) {
    std::cout << "[" << place << "=" << str << "]" << std::endl;
  }
}

void cmDependsJavaParserHelper::CombineUnions(char** out, const char* in1,
                                              char** in2, const char* sep)
{
  size_t len = 1;
  if (in1) {
    len += strlen(in1);
  }
  if (*in2) {
    len += strlen(*in2);
  }
  if (sep) {
    len += strlen(sep);
  }
  *out = new char[len];
  *out[0] = 0;
  if (in1) {
    strcat(*out, in1);
  }
  if (sep) {
    strcat(*out, sep);
  }
  if (*in2) {
    strcat(*out, *in2);
  }
  if (*in2) {
    this->DeallocateParserType(in2);
  }
  this->UnionsAvailable++;
}

void cmDependsJavaParserHelper::CheckEmpty(
  int line, int cnt, cmDependsJavaParserHelper::ParserType* pt)
{
  int cc;
  int kk = -cnt + 1;
  for (cc = 1; cc <= cnt; cc++) {
    cmDependsJavaParserHelper::ParserType* cpt = pt + kk;
    this->SafePrintMissing(cpt->str, line, cc);
    kk++;
  }
}

void cmDependsJavaParserHelper::PrepareElement(
  cmDependsJavaParserHelper::ParserType* me)
{
  // Inititalize self
  me->str = nullptr;
}

void cmDependsJavaParserHelper::AllocateParserType(
  cmDependsJavaParserHelper::ParserType* pt, const char* str, int len)
{
  pt->str = nullptr;
  if (len == 0) {
    len = static_cast<int>(strlen(str));
  }
  if (len == 0) {
    return;
  }
  this->UnionsAvailable++;
  auto up = cm::make_unique<char[]>(len + 1);
  pt->str = up.get();
  strncpy(pt->str, str, len);
  pt->str[len] = 0;
  this->Allocates.push_back(std::move(up));
}

void cmDependsJavaParserHelper::StartClass(const char* cls)
{
  CurrentClass cl;
  cl.Name = cls;
  this->ClassStack.push_back(std::move(cl));

  this->CurrentDepth++;
}

void cmDependsJavaParserHelper::EndClass()
{
  if (this->ClassStack.empty()) {
    std::cerr << "Error when parsing. Current class is null" << std::endl;
    abort();
  }
  if (this->ClassStack.size() <= 1) {
    std::cerr << "Error when parsing. Parent class is null" << std::endl;
    abort();
  }
  CurrentClass& current = this->ClassStack.back();
  CurrentClass& parent = this->ClassStack[this->ClassStack.size() - 2];
  this->CurrentDepth--;
  parent.NestedClasses.push_back(current);
  this->ClassStack.pop_back();
}

void cmDependsJavaParserHelper::PrintClasses()
{
  if (this->ClassStack.empty()) {
    std::cerr << "Error when parsing. No classes on class stack" << std::endl;
    abort();
  }
  for (std::string const& f : this->GetFilesProduced()) {
    std::cout << "  " << f << ".class" << std::endl;
  }
}

std::vector<std::string> cmDependsJavaParserHelper::GetFilesProduced()
{
  std::vector<std::string> files;
  CurrentClass const& toplevel = this->ClassStack.front();
  for (CurrentClass const& nc : toplevel.NestedClasses) {
    nc.AddFileNamesForPrinting(&files, nullptr, "$");
  }
  return files;
}

int cmDependsJavaParserHelper::ParseString(const char* str, int verb)
{
  if (!str) {
    return 0;
  }
  this->Verbose = verb;
  this->InputBuffer = str;
  this->InputBufferPos = 0;
  this->CurrentLine = 0;

  yyscan_t yyscanner;
  cmDependsJava_yylex_init(&yyscanner);
  cmDependsJava_yyset_extra(this, yyscanner);
  int res = cmDependsJava_yyparse(yyscanner);
  cmDependsJava_yylex_destroy(yyscanner);
  if (res != 0) {
    std::cout << "JP_Parse returned: " << res << std::endl;
    return 0;
  }

  if (verb) {
    if (!this->CurrentPackage.empty()) {
      std::cout << "Current package is: " << this->CurrentPackage << std::endl;
    }
    std::cout << "Imports packages:";
    if (!this->PackagesImport.empty()) {
      std::vector<std::string>::iterator it;
      for (it = this->PackagesImport.begin(); it != this->PackagesImport.end();
           ++it) {
        std::cout << " " << *it;
      }
    }
    std::cout << std::endl;
    std::cout << "Depends on:";
    if (!this->ClassesFound.empty()) {
      for (std::string const& cf : this->ClassesFound) {
        std::cout << " " << cf;
      }
    }
    std::cout << std::endl;
    std::cout << "Generated files:" << std::endl;
    this->PrintClasses();
    if (this->UnionsAvailable != 0) {
      std::cout << "There are still " << this->UnionsAvailable
                << " unions available" << std::endl;
    }
  }
  this->CleanupParser();
  return 1;
}

void cmDependsJavaParserHelper::CleanupParser()
{
  this->Allocates.clear();
}

int cmDependsJavaParserHelper::LexInput(char* buf, int maxlen)
{
  if (maxlen < 1) {
    return 0;
  }
  if (this->InputBufferPos < this->InputBuffer.size()) {
    buf[0] = this->InputBuffer[this->InputBufferPos++];
    if (buf[0] == '\n') {
      this->CurrentLine++;
    }
    return 1;
  }
  buf[0] = '\n';
  return 0;
}
void cmDependsJavaParserHelper::Error(const char* str)
{
  unsigned long pos = static_cast<unsigned long>(this->InputBufferPos);
  fprintf(stderr, "JPError: %s (%lu / Line: %d)\n", str, pos,
          this->CurrentLine);
  std::cerr << "String: ["
            << cm::string_view{ this->InputBuffer }.substr(
                 this->InputBufferPos, 30)
            << "]" << std::endl;
}

void cmDependsJavaParserHelper::UpdateCombine(const char* str1,
                                              const char* str2)
{
  if (this->CurrentCombine.empty() && str1 != nullptr) {
    this->CurrentCombine = str1;
  }
  this->CurrentCombine += ".";
  this->CurrentCombine += str2;
}

int cmDependsJavaParserHelper::ParseFile(const char* file)
{
  if (!cmSystemTools::FileExists(file)) {
    return 0;
  }
  cmsys::ifstream ifs(file);
  if (!ifs) {
    return 0;
  }

  std::string fullfile;
  std::string line;
  while (cmSystemTools::GetLineFromStream(ifs, line)) {
    fullfile += line + "\n";
  }
  return this->ParseString(fullfile.c_str(), 0);
}
