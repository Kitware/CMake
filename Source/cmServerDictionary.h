/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

// Vocabulary:

static const std::string kDIRTY_SIGNAL = "dirty";
static const std::string kFILE_CHANGE_SIGNAL = "fileChange";

static const std::string kCACHE_TYPE = "cache";
static const std::string kCMAKE_INPUTS_TYPE = "cmakeInputs";
static const std::string kCODE_MODEL_TYPE = "codemodel";
static const std::string kCOMPUTE_TYPE = "compute";
static const std::string kCONFIGURE_TYPE = "configure";
static const std::string kERROR_TYPE = "error";
static const std::string kFILESYSTEM_WATCHERS_TYPE = "fileSystemWatchers";
static const std::string kGLOBAL_SETTINGS_TYPE = "globalSettings";
static const std::string kHANDSHAKE_TYPE = "handshake";
static const std::string kMESSAGE_TYPE = "message";
static const std::string kPROGRESS_TYPE = "progress";
static const std::string kREPLY_TYPE = "reply";
static const std::string kSET_GLOBAL_SETTINGS_TYPE = "setGlobalSettings";
static const std::string kSIGNAL_TYPE = "signal";
static const std::string kCTEST_INFO_TYPE = "ctestInfo";

static const std::string kBUILD_FILES_KEY = "buildFiles";
static const std::string kCACHE_ARGUMENTS_KEY = "cacheArguments";
static const std::string kCACHE_KEY = "cache";
static const std::string kCAPABILITIES_KEY = "capabilities";
static const std::string kCHECK_SYSTEM_VARS_KEY = "checkSystemVars";
static const std::string kCMAKE_ROOT_DIRECTORY_KEY = "cmakeRootDirectory";
static const std::string kCOOKIE_KEY = "cookie";
static const std::string kDEBUG_OUTPUT_KEY = "debugOutput";
static const std::string kERROR_MESSAGE_KEY = "errorMessage";
static const std::string kEXTRA_GENERATOR_KEY = "extraGenerator";
static const std::string kGENERATOR_KEY = "generator";
static const std::string kIS_EXPERIMENTAL_KEY = "isExperimental";
static const std::string kKEYS_KEY = "keys";
static const std::string kMAJOR_KEY = "major";
static const std::string kMESSAGE_KEY = "message";
static const std::string kMINOR_KEY = "minor";
static const std::string kPLATFORM_KEY = "platform";
static const std::string kPROGRESS_CURRENT_KEY = "progressCurrent";
static const std::string kPROGRESS_MAXIMUM_KEY = "progressMaximum";
static const std::string kPROGRESS_MESSAGE_KEY = "progressMessage";
static const std::string kPROGRESS_MINIMUM_KEY = "progressMinimum";
static const std::string kPROTOCOL_VERSION_KEY = "protocolVersion";
static const std::string kREPLY_TO_KEY = "inReplyTo";
static const std::string kSUPPORTED_PROTOCOL_VERSIONS =
  "supportedProtocolVersions";
static const std::string kTITLE_KEY = "title";
static const std::string kTOOLSET_KEY = "toolset";
static const std::string kTRACE_EXPAND_KEY = "traceExpand";
static const std::string kTRACE_KEY = "trace";
static const std::string kWARN_UNINITIALIZED_KEY = "warnUninitialized";
static const std::string kWARN_UNUSED_CLI_KEY = "warnUnusedCli";
static const std::string kWARN_UNUSED_KEY = "warnUnused";
static const std::string kWATCHED_DIRECTORIES_KEY = "watchedDirectories";
static const std::string kWATCHED_FILES_KEY = "watchedFiles";

static const std::string kSTART_MAGIC = "[== \"CMake Server\" ==[";
static const std::string kEND_MAGIC = "]== \"CMake Server\" ==]";

static const std::string kRENAME_PROPERTY_VALUE = "rename";
static const std::string kCHANGE_PROPERTY_VALUE = "change";
