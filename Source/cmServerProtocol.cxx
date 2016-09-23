/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2016 Tobias Hunger <tobias.hunger@qt.io>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmServerProtocol.h"

#include "cmExternalMakefileProjectGenerator.h"
#include "cmServer.h"
#include "cmServerDictionary.h"
#include "cmSystemTools.h"
#include "cmake.h"

#include "cmServerDictionary.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_reader.h"
#include "cm_jsoncpp_value.h"
#endif

cmServerRequest::cmServerRequest(cmServer* server, const std::string& t,
                                 const std::string& c, const Json::Value& d)
  : Type(t)
  , Cookie(c)
  , Data(d)
  , m_Server(server)
{
}

void cmServerRequest::ReportProgress(int min, int current, int max,
                                     const std::string& message) const
{
  this->m_Server->WriteProgress(*this, min, current, max, message);
}

void cmServerRequest::ReportMessage(const std::string& message,
                                    const std::string& title) const
{
  m_Server->WriteMessage(*this, message, title);
}

cmServerResponse cmServerRequest::Reply(const Json::Value& data) const
{
  cmServerResponse response(*this);
  response.SetData(data);
  return response;
}

cmServerResponse cmServerRequest::ReportError(const std::string& message) const
{
  cmServerResponse response(*this);
  response.SetError(message);
  return response;
}

cmServerResponse::cmServerResponse(const cmServerRequest& request)
  : Type(request.Type)
  , Cookie(request.Cookie)
{
}

void cmServerResponse::SetData(const Json::Value& data)
{
  assert(this->m_Payload == PAYLOAD_UNKNOWN);
  if (!data[kCOOKIE_KEY].isNull() || !data[kTYPE_KEY].isNull()) {
    this->SetError("Response contains cookie or type field.");
    return;
  }
  this->m_Payload = PAYLOAD_DATA;
  this->m_Data = data;
}

void cmServerResponse::SetError(const std::string& message)
{
  assert(this->m_Payload == PAYLOAD_UNKNOWN);
  this->m_Payload = PAYLOAD_ERROR;
  this->m_ErrorMessage = message;
}

bool cmServerResponse::IsComplete() const
{
  return this->m_Payload != PAYLOAD_UNKNOWN;
}

bool cmServerResponse::IsError() const
{
  assert(this->m_Payload != PAYLOAD_UNKNOWN);
  return this->m_Payload == PAYLOAD_ERROR;
}

std::string cmServerResponse::ErrorMessage() const
{
  if (this->m_Payload == PAYLOAD_ERROR)
    return this->m_ErrorMessage;
  else
    return std::string();
}

Json::Value cmServerResponse::Data() const
{
  assert(this->m_Payload != PAYLOAD_UNKNOWN);
  return this->m_Data;
}

bool cmServerProtocol::Activate(cmServer* server,
                                const cmServerRequest& request,
                                std::string* errorMessage)
{
  assert(server);
  this->m_Server = server;
  this->m_CMakeInstance = std::make_unique<cmake>();
  const bool result = this->DoActivate(request, errorMessage);
  if (!result)
    this->m_CMakeInstance = CM_NULLPTR;
  return result;
}

void cmServerProtocol::SendSignal(const std::string& name,
                                  const Json::Value& data) const
{
  if (this->m_Server)
    this->m_Server->WriteSignal(name, data);
}

cmake* cmServerProtocol::CMakeInstance() const
{
  return this->m_CMakeInstance.get();
}

bool cmServerProtocol::DoActivate(const cmServerRequest& /*request*/,
                                  std::string* /*errorMessage*/)
{
  return true;
}

std::pair<int, int> cmServerProtocol1_0::ProtocolVersion() const
{
  return std::make_pair(1, 0);
}

bool cmServerProtocol1_0::DoActivate(const cmServerRequest& request,
                                     std::string* errorMessage)
{
  std::string sourceDirectory = request.Data[kSOURCE_DIRECTORY_KEY].asString();
  const std::string buildDirectory =
    request.Data[kBUILD_DIRECTORY_KEY].asString();
  std::string generator = request.Data[kGENERATOR_KEY].asString();
  std::string extraGenerator = request.Data[kEXTRA_GENERATOR_KEY].asString();

  if (buildDirectory.empty()) {
    if (errorMessage)
      *errorMessage =
        std::string("\"") + kBUILD_DIRECTORY_KEY + "\" is missing.";
    return false;
  }
  cmake* cm = CMakeInstance();
  if (cmSystemTools::PathExists(buildDirectory)) {
    if (!cmSystemTools::FileIsDirectory(buildDirectory)) {
      if (errorMessage)
        *errorMessage = std::string("\"") + kBUILD_DIRECTORY_KEY +
          "\" exists but is not a directory.";
      return false;
    }

    const std::string cachePath = cm->FindCacheFile(buildDirectory);
    if (cm->LoadCache(cachePath)) {
      cmState* state = cm->GetState();

      // Check generator:
      const std::string cachedGenerator =
        std::string(state->GetCacheEntryValue("CMAKE_GENERATOR"));
      if (cachedGenerator.empty() && generator.empty()) {
        if (errorMessage)
          *errorMessage =
            std::string("\"") + kGENERATOR_KEY + "\" is required but unset.";
        return false;
      }
      if (generator.empty()) {
        generator = cachedGenerator;
      }
      if (generator != cachedGenerator) {
        if (errorMessage)
          *errorMessage = std::string("\"") + kGENERATOR_KEY +
            "\" set but incompatible with configured generator.";
        return false;
      }

      // check extra generator:
      const std::string cachedExtraGenerator =
        std::string(state->GetCacheEntryValue("CMAKE_EXTRA_GENERATOR"));
      if (!cachedExtraGenerator.empty() && !extraGenerator.empty() &&
          cachedExtraGenerator != extraGenerator) {
        if (errorMessage)
          *errorMessage = std::string("\"") + kEXTRA_GENERATOR_KEY +
            "\" is set but incompatible with configured extra generator.";
        return false;
      }
      if (extraGenerator.empty()) {
        extraGenerator = cachedExtraGenerator;
      }

      // check sourcedir:
      const std::string cachedSourceDirectory =
        std::string(state->GetCacheEntryValue("CMAKE_HOME_DIRECTORY"));
      if (!cachedSourceDirectory.empty() && !sourceDirectory.empty() &&
          cachedSourceDirectory != sourceDirectory) {
        if (errorMessage)
          *errorMessage = std::string("\"") + kSOURCE_DIRECTORY_KEY +
            "\" is set but incompatible with configured source directory.";
        return false;
      }
      if (sourceDirectory.empty()) {
        sourceDirectory = cachedSourceDirectory;
      }
    }
  }

  if (sourceDirectory.empty()) {
    if (errorMessage)
      *errorMessage = std::string("\"") + kSOURCE_DIRECTORY_KEY +
        "\" is unset but required.";
    return false;
  }
  if (!cmSystemTools::FileIsDirectory(sourceDirectory)) {
    if (errorMessage)
      *errorMessage =
        std::string("\"") + kSOURCE_DIRECTORY_KEY + "\" is not a directory.";
    return false;
  }
  if (generator.empty()) {
    if (errorMessage)
      *errorMessage =
        std::string("\"") + kGENERATOR_KEY + "\" is unset but required.";
    return false;
  }

  const std::string fullGeneratorName =
    cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
      generator, extraGenerator);

  cmGlobalGenerator* gg = cm->CreateGlobalGenerator(fullGeneratorName);
  if (!gg) {
    if (errorMessage)
      *errorMessage =
        std::string("Could not set up the requested combination of \"") +
        kGENERATOR_KEY + "\" and \"" + kEXTRA_GENERATOR_KEY + "\"";
    return false;
  }

  cm->SetGlobalGenerator(gg);
  cm->SetHomeDirectory(sourceDirectory);
  cm->SetHomeOutputDirectory(buildDirectory);

  this->m_State = STATE_ACTIVE;
  return true;
}

const cmServerResponse cmServerProtocol1_0::Process(
  const cmServerRequest& request)
{
  assert(this->m_State >= STATE_ACTIVE);

  return request.ReportError("Unknown command!");
}

bool cmServerProtocol1_0::IsExperimental() const
{
  return true;
}
