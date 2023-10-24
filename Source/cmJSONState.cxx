/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmJSONState.h"

#include <iterator>
#include <sstream>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmsys/FStream.hxx"

#include "cmStringAlgorithms.h"

cmJSONState::cmJSONState(const std::string& filename, Json::Value* root)
{
  cmsys::ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
  if (!fin) {
    this->AddError(cmStrCat("File not found: ", filename));
    return;
  }
  // If there's a BOM, toss it.
  cmsys::FStream::ReadBOM(fin);

  // Save the entire document.
  std::streampos finBegin = fin.tellg();
  this->doc = std::string(std::istreambuf_iterator<char>(fin),
                          std::istreambuf_iterator<char>());
  if (this->doc.empty()) {
    this->AddError("A JSON document cannot be empty");
    return;
  }
  fin.seekg(finBegin);

  // Parse the document.
  Json::CharReaderBuilder builder;
  Json::CharReaderBuilder::strictMode(&builder.settings_);
  std::string errMsg;
  if (!Json::parseFromStream(builder, fin, root, &errMsg)) {
    errMsg = cmStrCat("JSON Parse Error: ", filename, ":\n", errMsg);
    this->AddError(errMsg);
  }
}

void cmJSONState::AddError(std::string const& errMsg)
{
  this->errors.emplace_back(errMsg);
}

void cmJSONState::AddErrorAtValue(std::string const& errMsg,
                                  const Json::Value* value)
{
  if (value && !value->isNull()) {
    this->AddErrorAtOffset(errMsg, value->getOffsetStart());
  } else {
    this->AddError(errMsg);
  }
}

void cmJSONState::AddErrorAtOffset(std::string const& errMsg,
                                   std::ptrdiff_t offset)
{
  if (doc.empty()) {
    this->AddError(errMsg);
  } else {
    Location loc = LocateInDocument(offset);
    this->errors.emplace_back(loc, errMsg);
  }
}

std::string cmJSONState::GetErrorMessage(bool showContext)
{
  std::string message;
  for (auto const& error : this->errors) {
    message = cmStrCat(message, error.GetErrorMessage(), "\n");
    if (showContext) {
      Location loc = error.GetLocation();
      if (loc.column > 0) {
        message = cmStrCat(message, GetJsonContext(loc), "\n");
      }
    }
  }
  message = cmStrCat("\n", message);
  message.pop_back();
  return message;
}

std::string cmJSONState::key()
{
  if (!this->parseStack.empty()) {
    return this->parseStack.back().first;
  }
  return "";
}

std::string cmJSONState::key_after(std::string const& k)
{
  for (auto it = this->parseStack.begin(); it != this->parseStack.end();
       ++it) {
    if (it->first == k && (++it) != this->parseStack.end()) {
      return it->first;
    }
  }
  return "";
}

const Json::Value* cmJSONState::value_after(std::string const& k)
{
  for (auto it = this->parseStack.begin(); it != this->parseStack.end();
       ++it) {
    if (it->first == k && (++it) != this->parseStack.end()) {
      return it->second;
    }
  }
  return nullptr;
}

void cmJSONState::push_stack(std::string const& k, const Json::Value* value)
{
  this->parseStack.emplace_back(k, value);
}

void cmJSONState::pop_stack()
{
  this->parseStack.pop_back();
}

std::string cmJSONState::GetJsonContext(Location loc)
{
  std::string line;
  std::stringstream sstream(doc);
  for (int i = 0; i < loc.line; ++i) {
    std::getline(sstream, line, '\n');
  }
  return cmStrCat(line, '\n', std::string(loc.column - 1, ' '), '^');
}

cmJSONState::Location cmJSONState::LocateInDocument(ptrdiff_t offset)
{
  int line = 1;
  int col = 1;
  const char* beginDoc = doc.data();
  const char* last = beginDoc + offset;
  for (; beginDoc != last; ++beginDoc) {
    switch (*beginDoc) {
      case '\r':
        if (beginDoc + 1 != last && beginDoc[1] == '\n') {
          continue; // consume CRLF as a single token.
        }
        CM_FALLTHROUGH; // CR without a following LF is same as LF
      case '\n':
        col = 1;
        ++line;
        break;
      default:
        ++col;
        break;
    }
  }
  return { line, col };
}
