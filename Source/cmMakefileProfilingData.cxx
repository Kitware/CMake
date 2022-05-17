/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmMakefileProfilingData.h"

#include <chrono>
#include <stdexcept>
#include <vector>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmsys/FStream.hxx"
#include "cmsys/SystemInformation.hxx"

#include "cmListFileCache.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmMakefileProfilingData::cmMakefileProfilingData(
  const std::string& profileStream)
{
  std::ios::openmode omode = std::ios::out | std::ios::trunc;
  this->ProfileStream.open(profileStream.c_str(), omode);
  Json::StreamWriterBuilder wbuilder;
  this->JsonWriter =
    std::unique_ptr<Json::StreamWriter>(wbuilder.newStreamWriter());
  if (!this->ProfileStream.good()) {
    throw std::runtime_error(std::string("Unable to open: ") + profileStream);
  }

  this->ProfileStream << "[";
}

cmMakefileProfilingData::~cmMakefileProfilingData() noexcept
{
  if (this->ProfileStream.good()) {
    try {
      this->ProfileStream << "]";
      this->ProfileStream.close();
    } catch (...) {
      cmSystemTools::Error("Error writing profiling output!");
    }
  }
}

void cmMakefileProfilingData::StartEntry(const cmListFileFunction& lff,
                                         cmListFileContext const& lfc)
{
  /* Do not try again if we previously failed to write to output. */
  if (!this->ProfileStream.good()) {
    return;
  }

  try {
    if (this->ProfileStream.tellp() > 1) {
      this->ProfileStream << ",";
    }
    cmsys::SystemInformation info;
    Json::Value v;
    v["ph"] = "B";
    v["name"] = lff.LowerCaseName();
    v["cat"] = "cmake";
    v["ts"] = static_cast<Json::Value::UInt64>(
      std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
        .count());
    v["pid"] = static_cast<int>(info.GetProcessId());
    v["tid"] = 0;
    Json::Value argsValue;
    if (!lff.Arguments().empty()) {
      std::string args;
      for (auto const& a : lff.Arguments()) {
        args += (args.empty() ? "" : " ") + a.Value;
      }
      argsValue["functionArgs"] = args;
    }
    argsValue["location"] = lfc.FilePath + ":" + std::to_string(lfc.Line);
    v["args"] = argsValue;

    this->JsonWriter->write(v, &this->ProfileStream);
  } catch (std::ios_base::failure& fail) {
    cmSystemTools::Error(
      cmStrCat("Failed to write to profiling output: ", fail.what()));
  } catch (...) {
    cmSystemTools::Error("Error writing profiling output!");
  }
}

void cmMakefileProfilingData::StopEntry()
{
  /* Do not try again if we previously failed to write to output. */
  if (!this->ProfileStream.good()) {
    return;
  }

  try {
    this->ProfileStream << ",";
    cmsys::SystemInformation info;
    Json::Value v;
    v["ph"] = "E";
    v["ts"] = static_cast<Json::Value::UInt64>(
      std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
        .count());
    v["pid"] = static_cast<int>(info.GetProcessId());
    v["tid"] = 0;
    this->JsonWriter->write(v, &this->ProfileStream);
  } catch (std::ios_base::failure& fail) {
    cmSystemTools::Error(
      cmStrCat("Failed to write to profiling output:", fail.what()));
  } catch (...) {
    cmSystemTools::Error("Error writing profiling output!");
  }
}
