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
#include "cmake.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_reader.h"
#include "cm_jsoncpp_value.h"
#endif

namespace {
// Vocabulary:

const std::string kCOOKIE_KEY = "cookie";
const std::string kTYPE_KEY = "type";

} // namespace

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

bool cmServerProtocol::Activate(const cmServerRequest& request,
                                std::string* errorMessage)
{
  this->m_CMakeInstance = std::make_unique<cmake>();
  const bool result = this->DoActivate(request, errorMessage);
  if (!result)
    this->m_CMakeInstance = CM_NULLPTR;
  return result;
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
