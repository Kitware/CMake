/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmServerProtocol.h"

#include "cmExternalMakefileProjectGenerator.h"
#include "cmGlobalGenerator.h"
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
  if (this->m_Payload == PAYLOAD_ERROR) {
    return this->m_ErrorMessage;
  }
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
  if (!result) {
    this->m_CMakeInstance = CM_NULLPTR;
  }
  return result;
}

void cmServerProtocol::SendSignal(const std::string& name,
                                  const Json::Value& data) const
{
  if (this->m_Server) {
    this->m_Server->WriteSignal(name, data);
  }
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
    if (errorMessage) {
      *errorMessage =
        std::string("\"") + kBUILD_DIRECTORY_KEY + "\" is missing.";
    }
    return false;
  }
  cmake* cm = CMakeInstance();
  if (cmSystemTools::PathExists(buildDirectory)) {
    if (!cmSystemTools::FileIsDirectory(buildDirectory)) {
      if (errorMessage) {
        *errorMessage = std::string("\"") + kBUILD_DIRECTORY_KEY +
          "\" exists but is not a directory.";
      }
      return false;
    }

    const std::string cachePath = cm->FindCacheFile(buildDirectory);
    if (cm->LoadCache(cachePath)) {
      cmState* state = cm->GetState();

      // Check generator:
      const std::string cachedGenerator =
        std::string(state->GetCacheEntryValue("CMAKE_GENERATOR"));
      if (cachedGenerator.empty() && generator.empty()) {
        if (errorMessage) {
          *errorMessage =
            std::string("\"") + kGENERATOR_KEY + "\" is required but unset.";
        }
        return false;
      }
      if (generator.empty()) {
        generator = cachedGenerator;
      }
      if (generator != cachedGenerator) {
        if (errorMessage) {
          *errorMessage = std::string("\"") + kGENERATOR_KEY +
            "\" set but incompatible with configured generator.";
        }
        return false;
      }

      // check extra generator:
      const std::string cachedExtraGenerator =
        std::string(state->GetCacheEntryValue("CMAKE_EXTRA_GENERATOR"));
      if (!cachedExtraGenerator.empty() && !extraGenerator.empty() &&
          cachedExtraGenerator != extraGenerator) {
        if (errorMessage) {
          *errorMessage = std::string("\"") + kEXTRA_GENERATOR_KEY +
            "\" is set but incompatible with configured extra generator.";
        }
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
        if (errorMessage) {
          *errorMessage = std::string("\"") + kSOURCE_DIRECTORY_KEY +
            "\" is set but incompatible with configured source directory.";
        }
        return false;
      }
      if (sourceDirectory.empty()) {
        sourceDirectory = cachedSourceDirectory;
      }
    }
  }

  if (sourceDirectory.empty()) {
    if (errorMessage) {
      *errorMessage = std::string("\"") + kSOURCE_DIRECTORY_KEY +
        "\" is unset but required.";
    }
    return false;
  }
  if (!cmSystemTools::FileIsDirectory(sourceDirectory)) {
    if (errorMessage) {
      *errorMessage =
        std::string("\"") + kSOURCE_DIRECTORY_KEY + "\" is not a directory.";
    }
    return false;
  }
  if (generator.empty()) {
    if (errorMessage) {
      *errorMessage =
        std::string("\"") + kGENERATOR_KEY + "\" is unset but required.";
    }
    return false;
  }

  const std::string fullGeneratorName =
    cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
      generator, extraGenerator);

  cmGlobalGenerator* gg = cm->CreateGlobalGenerator(fullGeneratorName);
  if (!gg) {
    if (errorMessage) {
      *errorMessage =
        std::string("Could not set up the requested combination of \"") +
        kGENERATOR_KEY + "\" and \"" + kEXTRA_GENERATOR_KEY + "\"";
    }
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

  if (request.Type == kCOMPUTE_TYPE) {
    return this->ProcessCompute(request);
  }
  if (request.Type == kCONFIGURE_TYPE) {
    return this->ProcessConfigure(request);
  }
  if (request.Type == kGLOBAL_SETTINGS_TYPE) {
    return this->ProcessGlobalSettings(request);
  }
  if (request.Type == kSET_GLOBAL_SETTINGS_TYPE) {
    return this->ProcessSetGlobalSettings(request);
  }

  return request.ReportError("Unknown command!");
}

bool cmServerProtocol1_0::IsExperimental() const
{
  return true;
}

cmServerResponse cmServerProtocol1_0::ProcessCompute(
  const cmServerRequest& request)
{
  if (this->m_State > STATE_CONFIGURED) {
    return request.ReportError("This build system was already generated.");
  }
  if (this->m_State < STATE_CONFIGURED) {
    return request.ReportError("This project was not configured yet.");
  }

  cmake* cm = this->CMakeInstance();
  int ret = cm->Generate();

  if (ret < 0) {
    return request.ReportError("Failed to compute build system.");
  }
  m_State = STATE_COMPUTED;
  return request.Reply(Json::Value());
}

cmServerResponse cmServerProtocol1_0::ProcessConfigure(
  const cmServerRequest& request)
{
  if (this->m_State == STATE_INACTIVE) {
    return request.ReportError("This instance is inactive.");
  }

  // Make sure the types of cacheArguments matches (if given):
  std::vector<std::string> cacheArgs;
  bool cacheArgumentsError = false;
  const Json::Value passedArgs = request.Data[kCACHE_ARGUMENTS_KEY];
  if (!passedArgs.isNull()) {
    if (passedArgs.isString()) {
      cacheArgs.push_back(passedArgs.asString());
    } else if (passedArgs.isArray()) {
      for (auto i = passedArgs.begin(); i != passedArgs.end(); ++i) {
        if (!i->isString()) {
          cacheArgumentsError = true;
          break;
        }
        cacheArgs.push_back(i->asString());
      }
    } else {
      cacheArgumentsError = true;
    }
  }
  if (cacheArgumentsError) {
    request.ReportError(
      "cacheArguments must be unset, a string or an array of strings.");
  }

  cmake* cm = this->CMakeInstance();
  std::string sourceDir = cm->GetHomeDirectory();
  const std::string buildDir = cm->GetHomeOutputDirectory();

  if (buildDir.empty()) {
    return request.ReportError(
      "No build directory set via setGlobalSettings.");
  }

  if (cm->LoadCache(buildDir)) {
    // build directory has been set up before
    const char* cachedSourceDir =
      cm->GetState()->GetInitializedCacheValue("CMAKE_HOME_DIRECTORY");
    if (!cachedSourceDir) {
      return request.ReportError("No CMAKE_HOME_DIRECTORY found in cache.");
    }
    if (sourceDir.empty()) {
      sourceDir = std::string(cachedSourceDir);
      cm->SetHomeDirectory(sourceDir);
    }

    const char* cachedGenerator =
      cm->GetState()->GetInitializedCacheValue("CMAKE_GENERATOR");
    if (cachedGenerator) {
      cmGlobalGenerator* gen = cm->GetGlobalGenerator();
      if (gen && gen->GetName() != cachedGenerator) {
        return request.ReportError("Configured generator does not match with "
                                   "CMAKE_GENERATOR found in cache.");
      }
    }
  } else {
    // build directory has not been set up before
    if (sourceDir.empty()) {
      return request.ReportError("No sourceDirectory set via "
                                 "setGlobalSettings and no cache found in "
                                 "buildDirectory.");
    }
  }

  if (cm->AddCMakePaths() != 1) {
    return request.ReportError("Failed to set CMake paths.");
  }

  if (!cm->SetCacheArgs(cacheArgs)) {
    return request.ReportError("cacheArguments could not be set.");
  }

  int ret = cm->Configure();
  if (ret < 0) {
    return request.ReportError("Configuration failed.");
  }
  m_State = STATE_CONFIGURED;
  return request.Reply(Json::Value());
}

cmServerResponse cmServerProtocol1_0::ProcessGlobalSettings(
  const cmServerRequest& request)
{
  cmake* cm = this->CMakeInstance();
  Json::Value obj = Json::objectValue;

  // Capabilities information:
  obj[kCAPABILITIES_KEY] = cm->ReportCapabilitiesJson(true);

  obj[kDEBUG_OUTPUT_KEY] = cm->GetDebugOutput();
  obj[kTRACE_KEY] = cm->GetTrace();
  obj[kTRACE_EXPAND_KEY] = cm->GetTraceExpand();
  obj[kWARN_UNINITIALIZED_KEY] = cm->GetWarnUninitialized();
  obj[kWARN_UNUSED_KEY] = cm->GetWarnUnused();
  obj[kWARN_UNUSED_CLI_KEY] = cm->GetWarnUnusedCli();
  obj[kCHECK_SYSTEM_VARS_KEY] = cm->GetCheckSystemVars();

  obj[kSOURCE_DIRECTORY_KEY] = cm->GetHomeDirectory();
  obj[kBUILD_DIRECTORY_KEY] = cm->GetHomeOutputDirectory();

  // Currently used generator:
  cmGlobalGenerator* gen = cm->GetGlobalGenerator();
  obj[kGENERATOR_KEY] = gen ? gen->GetName() : std::string();
  obj[kEXTRA_GENERATOR_KEY] =
    gen ? gen->GetExtraGeneratorName() : std::string();

  return request.Reply(obj);
}

static void setBool(const cmServerRequest& request, const std::string& key,
                    std::function<void(bool)> setter)
{
  if (request.Data[key].isNull()) {
    return;
  }
  setter(request.Data[key].asBool());
}

cmServerResponse cmServerProtocol1_0::ProcessSetGlobalSettings(
  const cmServerRequest& request)
{
  const std::vector<std::string> boolValues = {
    kDEBUG_OUTPUT_KEY,       kTRACE_KEY,       kTRACE_EXPAND_KEY,
    kWARN_UNINITIALIZED_KEY, kWARN_UNUSED_KEY, kWARN_UNUSED_CLI_KEY,
    kCHECK_SYSTEM_VARS_KEY
  };
  for (auto i : boolValues) {
    if (!request.Data[i].isNull() && !request.Data[i].isBool()) {
      return request.ReportError("\"" + i +
                                 "\" must be unset or a bool value.");
    }
  }

  cmake* cm = this->CMakeInstance();

  setBool(request, kDEBUG_OUTPUT_KEY,
          [cm](bool e) { cm->SetDebugOutputOn(e); });
  setBool(request, kTRACE_KEY, [cm](bool e) { cm->SetTrace(e); });
  setBool(request, kTRACE_EXPAND_KEY, [cm](bool e) { cm->SetTraceExpand(e); });
  setBool(request, kWARN_UNINITIALIZED_KEY,
          [cm](bool e) { cm->SetWarnUninitialized(e); });
  setBool(request, kWARN_UNUSED_KEY, [cm](bool e) { cm->SetWarnUnused(e); });
  setBool(request, kWARN_UNUSED_CLI_KEY,
          [cm](bool e) { cm->SetWarnUnusedCli(e); });
  setBool(request, kCHECK_SYSTEM_VARS_KEY,
          [cm](bool e) { cm->SetCheckSystemVars(e); });

  return request.Reply(Json::Value());
}
