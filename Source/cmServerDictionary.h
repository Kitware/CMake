/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

// Vocabulary:

static const std::string kCOMPUTE_TYPE = "compute";
static const std::string kCONFIGURE_TYPE = "configure";
static const std::string kERROR_TYPE = "error";
static const std::string kGLOBAL_SETTINGS_TYPE = "globalSettings";
static const std::string kHANDSHAKE_TYPE = "handshake";
static const std::string kMESSAGE_TYPE = "message";
static const std::string kPROGRESS_TYPE = "progress";
static const std::string kREPLY_TYPE = "reply";
static const std::string kSET_GLOBAL_SETTINGS_TYPE = "setGlobalSettings";
static const std::string kSIGNAL_TYPE = "signal";

static const std::string kBUILD_DIRECTORY_KEY = "buildDirectory";
static const std::string kCACHE_ARGUMENTS_KEY = "cacheArguments";
static const std::string kCAPABILITIES_KEY = "capabilities";
static const std::string kCHECK_SYSTEM_VARS_KEY = "checkSystemVars";
static const std::string kCOOKIE_KEY = "cookie";
static const std::string kDEBUG_OUTPUT_KEY = "debugOutput";
static const std::string kERROR_MESSAGE_KEY = "errorMessage";
static const std::string kEXTRA_GENERATOR_KEY = "extraGenerator";
static const std::string kGENERATOR_KEY = "generator";
static const std::string kIS_EXPERIMENTAL_KEY = "isExperimental";
static const std::string kMAJOR_KEY = "major";
static const std::string kMESSAGE_KEY = "message";
static const std::string kMINOR_KEY = "minor";
static const std::string kNAME_KEY = "name";
static const std::string kPROGRESS_CURRENT_KEY = "progressCurrent";
static const std::string kPROGRESS_MAXIMUM_KEY = "progressMaximum";
static const std::string kPROGRESS_MESSAGE_KEY = "progressMessage";
static const std::string kPROGRESS_MINIMUM_KEY = "progressMinimum";
static const std::string kPROTOCOL_VERSION_KEY = "protocolVersion";
static const std::string kREPLY_TO_KEY = "inReplyTo";
static const std::string kSOURCE_DIRECTORY_KEY = "sourceDirectory";
static const std::string kSUPPORTED_PROTOCOL_VERSIONS =
  "supportedProtocolVersions";
static const std::string kTITLE_KEY = "title";
static const std::string kTRACE_EXPAND_KEY = "traceExpand";
static const std::string kTRACE_KEY = "trace";
static const std::string kTYPE_KEY = "type";
static const std::string kWARN_UNINITIALIZED_KEY = "warnUninitialized";
static const std::string kWARN_UNUSED_CLI_KEY = "warnUnusedCli";
static const std::string kWARN_UNUSED_KEY = "warnUnused";

static const std::string kSTART_MAGIC = "[== CMake Server ==[";
static const std::string kEND_MAGIC = "]== CMake Server ==]";
