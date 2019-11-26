/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmVariableWatchCommand.h"

#include <memory>
#include <utility>

#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVariableWatch.h"
#include "cmake.h"

struct cmVariableWatchCallbackData
{
  bool InCallback;
  std::string Command;
};

static void cmVariableWatchCommandVariableAccessed(const std::string& variable,
                                                   int access_type,
                                                   void* client_data,
                                                   const char* newValue,
                                                   const cmMakefile* mf)
{
  cmVariableWatchCallbackData* data =
    static_cast<cmVariableWatchCallbackData*>(client_data);

  if (data->InCallback) {
    return;
  }
  data->InCallback = true;

  cmListFileFunction newLFF;
  cmListFileArgument arg;
  bool processed = false;
  const char* accessString = cmVariableWatch::GetAccessAsString(access_type);
  const char* currentListFile = mf->GetDefinition("CMAKE_CURRENT_LIST_FILE");

  /// Ultra bad!!
  cmMakefile* makefile = const_cast<cmMakefile*>(mf);

  std::string stack = makefile->GetProperty("LISTFILE_STACK");
  if (!data->Command.empty()) {
    newLFF.Arguments.clear();
    newLFF.Arguments.emplace_back(variable, cmListFileArgument::Quoted, 9999);
    newLFF.Arguments.emplace_back(accessString, cmListFileArgument::Quoted,
                                  9999);
    newLFF.Arguments.emplace_back(newValue ? newValue : "",
                                  cmListFileArgument::Quoted, 9999);
    newLFF.Arguments.emplace_back(currentListFile, cmListFileArgument::Quoted,
                                  9999);
    newLFF.Arguments.emplace_back(stack, cmListFileArgument::Quoted, 9999);
    newLFF.Name = data->Command;
    newLFF.Line = 9999;
    cmExecutionStatus status(*makefile);
    if (!makefile->ExecuteCommand(newLFF, status)) {
      cmSystemTools::Error(
        cmStrCat("Error in cmake code at\nUnknown:0:\nA command failed "
                 "during the invocation of callback \"",
                 data->Command, "\"."));
      data->InCallback = false;
      return;
    }
    processed = true;
  }
  if (!processed) {
    makefile->IssueMessage(
      MessageType::LOG,
      cmStrCat("Variable \"", variable, "\" was accessed using ", accessString,
               " with value \"", (newValue ? newValue : ""), "\"."));
  }

  data->InCallback = false;
}

static void deleteVariableWatchCallbackData(void* client_data)
{
  cmVariableWatchCallbackData* data =
    static_cast<cmVariableWatchCallbackData*>(client_data);
  delete data;
}

/** This command does not really have a final pass but it needs to
    stay alive since it owns variable watch callback information. */
class FinalAction
{
public:
  /* NOLINTNEXTLINE(performance-unnecessary-value-param) */
  FinalAction(cmMakefile* makefile, std::string variable)
    : Action(std::make_shared<Impl>(makefile, std::move(variable)))
  {
  }

  void operator()(cmMakefile&) const {}

private:
  struct Impl
  {
    Impl(cmMakefile* makefile, std::string variable)
      : Makefile(makefile)
      , Variable(std::move(variable))
    {
    }

    ~Impl()
    {
      this->Makefile->GetCMakeInstance()->GetVariableWatch()->RemoveWatch(
        this->Variable, cmVariableWatchCommandVariableAccessed);
    }

    cmMakefile* Makefile;
    std::string Variable;
  };

  std::shared_ptr<Impl const> Action;
};

bool cmVariableWatchCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("must be called with at least one argument.");
    return false;
  }
  std::string const& variable = args[0];
  std::string command;
  if (args.size() > 1) {
    command = args[1];
  }
  if (variable == "CMAKE_CURRENT_LIST_FILE") {
    status.SetError(cmStrCat("cannot be set on the variable: ", variable));
    return false;
  }

  cmVariableWatchCallbackData* data = new cmVariableWatchCallbackData;

  data->InCallback = false;
  data->Command = command;

  if (!status.GetMakefile().GetCMakeInstance()->GetVariableWatch()->AddWatch(
        variable, cmVariableWatchCommandVariableAccessed, data,
        deleteVariableWatchCallbackData)) {
    deleteVariableWatchCallbackData(data);
    return false;
  }

  status.GetMakefile().AddFinalAction(
    FinalAction(&status.GetMakefile(), variable));
  return true;
}
