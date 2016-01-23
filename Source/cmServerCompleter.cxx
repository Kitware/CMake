
#include "cmServerCompleter.h"

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmServer.h"

#include <cmsys/Glob.hxx>

cmServerCompleter::cmServerCompleter(cmake* cm, cmState::Snapshot snp,
                                     bool originMode)
  : CMakeInstance(cm)
  , Snapshot(snp)
  , mOriginMode(originMode)
{
}

Json::Value noCompletions()
{
  Json::Value root = Json::objectValue;

  Json::Value& obj = root["completion"] = Json::objectValue;
  obj["result"] = "no_completions";
  return root;
}

Json::Value cmServerCompleter::Complete(cmState::Snapshot snp,
                                        cmListFileFunction fn,
                                        std::string matcher, long fileLine,
                                        long fileColumn)
{
  if (!fn.Name.empty()) {
    if (fn.Line <= fileLine && fn.CloseParenLine <= fileLine) {
      if (fn.Line == fileLine) {
        if (fn.Column > fileColumn) {
          return noCompletions();
        }
        if (fileColumn <= long(fn.Column + fn.Name.size())) {
          return this->CodeCompleteCommand(snp, matcher);
        }
        if (fileLine < fn.Line) {
          return noCompletions();
        }
        assert(fileLine == fn.Line);
        if (fileColumn < fn.OpenParenColumn) {
          return noCompletions();
        }
      }
      return this->CodeCompleteParameter(snp, &fn, fileLine, fileColumn);
    }
    return this->CodeCompleteCommand(snp, matcher);
  }

  if (matcher.back() == ' ') {
    return noCompletions();
  }
  // Make sure we are not in a block- or long-bracket- comment?
  // Store that fact in the cmListFile?
  return this->CodeCompleteCommand(snp, matcher);
}

struct ParamMapEntry
{
  long StartLine;
  long StartColumn;
  long EndLine;
  long EndColumn;
  std::vector<std::string> Value;
};

Json::Value cmServerCompleter::CodeCompleteParameter(cmState::Snapshot snp,
                                                     cmListFileFunction* fn,
                                                     long fileLine,
                                                     long fileColumn)
{
  auto& args = fn->Arguments;

  cmCommand* cmd = this->CMakeInstance->GetState()->GetCommand(fn->Name);

  if (!cmd) {
    return noCompletions();
  }

  std::vector<std::string> params;

  if (args.empty()) {
    auto ctx = cmd->GetContextForParameter(params, 0);
    return doComplete(ctx, "", cmd, params, snp);
  }
  std::vector<ParamMapEntry> mapping;
  for (auto& arg : args) {
    std::vector<std::string> value;
    {
      std::stringstream ss(arg.Value);

      for (std::string line; std::getline(ss, line, '\n');) {
        value.push_back(line);
      }
    }

    auto endLine = arg.Line + value.size() - 1;
    auto endColumn = value.back().size();
    if (value.size() == 1) {
      endColumn += arg.Column;
    }

    mapping.push_back(
      { arg.Line, arg.Column, long(endLine), long(endColumn), value });
  }

  mapping.push_back({ fn->CloseParenLine,
                      fn->CloseParenColumn,
                      fn->CloseParenLine,
                      fn->CloseParenColumn,
                      { {} } });

  auto end = std::lower_bound(
    mapping.begin(), mapping.end(), fileLine,
    [](ParamMapEntry const& lhs, long rhs) { return lhs.StartLine < rhs; });
  while (end->EndLine <= fileLine && end->EndColumn < fileColumn) {
    ++end;
    //    assert(end != mapping.end());
  }
  if (end == mapping.end()) {
    --end;
  }

  //  assert(end != mapping.end());

  for (auto it = mapping.begin(); it != end; ++it) {
    params.push_back(cmJoin(it->Value, ";"));
  }

  auto ctx = cmd->GetContextForParameter(params, params.size());

  std::string matcher;

  bool withinCommand =
    (end->StartLine <= fileLine && end->EndLine >= fileLine &&
     end->StartColumn <= fileColumn && end->EndColumn >= fileColumn);

  if (withinCommand) {
    auto paramLine = fileLine - end->StartLine;
    assert(paramLine < long(end->Value.size()));
    auto line = end->Value[paramLine];
    if (end->StartLine == fileLine) {
      fileColumn -= end->StartColumn;
    }

    if (mOriginMode) {
      std::string context = line;
      auto openPos = context.find("${");
      if (openPos != std::string::npos) {
        context = context.substr(openPos + 2);
        auto closePos = context.find("}");
        if (closePos != std::string::npos) {
          context = context.substr(0, closePos);
          return this->VariableMatch(snp, context);
        }
      }
      return noCompletions();
    }

    matcher = line.substr(0, fileColumn);
    auto openPos = matcher.find("${");
    if (openPos != std::string::npos) {
      auto closePos = matcher.find("}");
      if (closePos == std::string::npos) {
        matcher = matcher.substr(openPos + 2);
        return this->CodeCompleteVariable(snp, matcher);
      }
    }
  }

  return doComplete(ctx, matcher, cmd, params, snp);
}

Json::Value cmServerCompleter::doComplete(cmCommand::ParameterContext ctx,
                                          std::string matcher, cmCommand* cmd,
                                          std::vector<std::string> params,
                                          cmState::Snapshot snp)
{
  switch (ctx) {
    case cmCommand::VariableIdentifierParameter: {
      return CodeCompleteVariable(this->Snapshot, matcher);
      break;
    }
    case cmCommand::SingleTargetParameter:
    case cmCommand::SingleBinaryTargetParameter: {
      auto mfs = this->CMakeInstance->GetGlobalGenerator()->GetMakefiles();

      Json::Value root = Json::objectValue;

      Json::Value& obj = root["completion"] = Json::objectValue;
      obj["matcher"] = matcher;
      auto& targets = obj["targets"] = Json::arrayValue;

      for (auto& mf : mfs) {
        cmTargets& tgts = mf->GetTargets();
        for (cmTargets::iterator it = tgts.begin(); it != tgts.end(); ++it) {
          if (ctx == cmCommand::SingleBinaryTargetParameter) {
            if (it->second.GetType() >= cmState::UTILITY) {
              continue;
            }
          }
          if (it->second.GetName().find(matcher) == 0) {
            targets.append(it->second.GetName());
          }
        }
      }
      return root;
    }
    case cmCommand::KeywordParameter: {
      auto options = cmd->GetKeywords(params, params.size());
      Json::Value root = Json::objectValue;

      Json::Value& obj = root["completion"] = Json::objectValue;
      obj["matcher"] = matcher;
      auto& keywords = obj["keywords"] = Json::arrayValue;
      for (auto& opt : options) {
        if (cmSystemTools::StringStartsWith(opt.c_str(), matcher.c_str())) {
          keywords.append(opt);
        }
      }
      return root;
    }
    case cmCommand::PackageNameParameter: {
      Json::Value root = Json::objectValue;

      auto options = this->GetPackageNames(snp);

      Json::Value& obj = root["completion"] = Json::objectValue;
      obj["matcher"] = matcher;
      auto& packages = obj["packages"] = Json::arrayValue;
      for (auto& opt : options) {
        if (cmSystemTools::StringStartsWith(opt.c_str(), matcher.c_str())) {
          packages.append(opt);
        }
      }
      return root;
    }
    case cmCommand::ModuleNameParameter: {
      Json::Value root = Json::objectValue;

      auto options = this->GetModuleNames(snp);

      Json::Value& obj = root["completion"] = Json::objectValue;
      obj["matcher"] = matcher;
      auto& modules = obj["modules"] = Json::arrayValue;
      for (auto& opt : options) {
        if (cmSystemTools::StringStartsWith(opt.c_str(), matcher.c_str())) {
          modules.append(opt);
        }
      }
      return root;
    }
    case cmCommand::PolicyParameter: {
      Json::Value root = Json::objectValue;

      Json::Value& obj = root["completion"] = Json::objectValue;
      obj["matcher"] = matcher;
      auto& policies = obj["policies"] = Json::arrayValue;
      auto& descriptions = obj["descriptions"] = Json::arrayValue;

#define CM_SELECT_ID_DOC(F, A1, A2, A3, A4, A5, A6) F(A1, A2)
#define CM_FOR_EACH_POLICY_ID_DOC(POLICY)                                     \
  CM_FOR_EACH_POLICY_TABLE(POLICY, CM_SELECT_ID_DOC)

#define POLICY_CASE(ID, DOC)                                                  \
  if (cmSystemTools::StringStartsWith(#ID, matcher.c_str())) {                \
    policies.append(#ID);                                                     \
    descriptions.append(#DOC);                                                \
  }
      CM_FOR_EACH_POLICY_ID_DOC(POLICY_CASE)
#undef POLICY_CASE

      return root;
    }
    default:
      break;
  }
  return noCompletions();
}

std::vector<std::string> cmServerCompleter::GetPackageNames(
  cmState::Snapshot snp) const
{
  std::vector<std::string> names;

  const char* cmakeModulePath = snp.GetDefinition("CMAKE_MODULE_PATH");
  if (cmakeModulePath) {
    std::vector<std::string> modulePath;
    cmSystemTools::ExpandListArgument(cmakeModulePath, modulePath);

    // Look through the possible module directories.
    for (std::vector<std::string>::iterator i = modulePath.begin();
         i != modulePath.end(); ++i) {
      std::string prefix = *i;
      cmSystemTools::ConvertToUnixSlashes(prefix);
      prefix += "/Find";

      std::string glob = prefix + "*.cmake";
      cmsys::Glob globIt;
      globIt.FindFiles(glob);
      auto files = globIt.GetFiles();

      for (auto& file : files) {
        names.push_back(
          file.substr(prefix.size(), file.size() - prefix.size() - 6));
      }
    }
  }

  std::string prefix = cmSystemTools::GetCMakeRoot();
  prefix += "/Modules/Find";

  std::string glob = prefix + "*.cmake";
  cmsys::Glob globIt;
  globIt.FindFiles(glob);
  auto files = globIt.GetFiles();

  for (auto& file : files) {
    names.push_back(
      file.substr(prefix.size(), file.size() - prefix.size() - 6));
  }

  return names;
}

std::vector<std::string> cmServerCompleter::GetModuleNames(
  cmState::Snapshot snp) const
{
  std::vector<std::string> names;

  const char* cmakeModulePath = snp.GetDefinition("CMAKE_MODULE_PATH");
  if (cmakeModulePath) {
    std::vector<std::string> modulePath;
    cmSystemTools::ExpandListArgument(cmakeModulePath, modulePath);

    // Look through the possible module directories.
    for (std::vector<std::string>::iterator i = modulePath.begin();
         i != modulePath.end(); ++i) {
      std::string prefix = *i;
      cmSystemTools::ConvertToUnixSlashes(prefix);
      prefix += "/";

      std::string glob = prefix + "*.cmake";
      cmsys::Glob globIt;
      globIt.FindFiles(glob);
      auto files = globIt.GetFiles();

      for (auto& file : files) {
        auto name =
          file.substr(prefix.size(), file.size() - prefix.size() - 6);
        if (name.find("Find") == 0) {
          continue;
        }
        names.push_back(name);
      }
    }
  }

  std::string prefix = cmSystemTools::GetCMakeRoot();
  prefix += "/Modules/";

  std::string glob = prefix + "*.cmake";
  cmsys::Glob globIt;
  globIt.FindFiles(glob);
  auto files = globIt.GetFiles();

  for (auto& file : files) {
    auto name = file.substr(prefix.size(), file.size() - prefix.size() - 6);
    if (name.find("Find") == 0) {
      continue;
    }
    names.push_back(name);
  }

  names.erase(cmRemoveDuplicates(names), names.end());

  return names;
}

Json::Value cmServerCompleter::VariableMatch(cmState::Snapshot snp,
                                             std::string matcher)
{
  (void)snp;
  Json::Value root = Json::objectValue;

  Json::Value& obj = root["context_origin"] = Json::objectValue;
  obj["matcher"] = matcher;
  return root;
}

Json::Value cmServerCompleter::CodeCompleteVariable(cmState::Snapshot snp,
                                                    std::string matcher)
{
  auto defs = snp.ClosureKeys();
  Json::Value root = Json::objectValue;

  Json::Value& obj = root["completion"] = Json::objectValue;
  obj["matcher"] = matcher;
  auto& variables = obj["variables"] = Json::arrayValue;
  for (auto& def : defs) {
    if (def.find(matcher) == 0) {
      variables.append(def);
    }
  }
  return root;
}

Json::Value cmServerCompleter::CodeCompleteCommand(cmState::Snapshot snp,
                                                   std::string matcher)
{
  auto cmds = snp.GetState()->GetCommandNames();

  Json::Value root = Json::objectValue;

  Json::Value& obj = root["completion"] = Json::objectValue;
  obj["matcher"] = matcher;
  auto& commands = obj["commands"] = Json::arrayValue;
  for (auto& cmd : cmds) {
    if (cmd.find(matcher) == 0) {
      commands.append(cmd);
    }
  }
  return root;
}
