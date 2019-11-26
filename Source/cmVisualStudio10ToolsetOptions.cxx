/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmVisualStudio10ToolsetOptions.h"

#include "cmAlgorithms.h"
#include "cmIDEFlagTable.h"
#include "cmVisualStudioGeneratorOptions.h"

std::string cmVisualStudio10ToolsetOptions::GetClFlagTableName(
  std::string const& name, std::string const& toolset) const
{
  std::string const useToolset = this->GetToolsetName(name, toolset);

  if (toolset == "v142") {
    return "v142";
  } else if (toolset == "v141") {
    return "v141";
  } else if (useToolset == "v140") {
    return "v140";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmVisualStudio10ToolsetOptions::GetCSharpFlagTableName(
  std::string const& name, std::string const& toolset) const
{
  std::string const useToolset = this->GetToolsetName(name, toolset);

  if (useToolset == "v142") {
    return "v142";
  } else if (useToolset == "v141") {
    return "v141";
  } else if (useToolset == "v140") {
    return "v140";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmVisualStudio10ToolsetOptions::GetRcFlagTableName(
  std::string const& name, std::string const& toolset) const
{
  std::string const useToolset = this->GetToolsetName(name, toolset);

  if ((useToolset == "v140") || (useToolset == "v141") ||
      (useToolset == "v142")) {
    return "v14";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmVisualStudio10ToolsetOptions::GetLibFlagTableName(
  std::string const& name, std::string const& toolset) const
{
  std::string const useToolset = this->GetToolsetName(name, toolset);

  if ((useToolset == "v140") || (useToolset == "v141") ||
      (useToolset == "v142")) {
    return "v14";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmVisualStudio10ToolsetOptions::GetLinkFlagTableName(
  std::string const& name, std::string const& toolset) const
{
  std::string const useToolset = this->GetToolsetName(name, toolset);

  if (useToolset == "v142") {
    return "v142";
  } else if (useToolset == "v141") {
    return "v141";
  } else if (useToolset == "v140") {
    return "v140";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmVisualStudio10ToolsetOptions::GetMasmFlagTableName(
  std::string const& name, std::string const& toolset) const
{
  std::string const useToolset = this->GetToolsetName(name, toolset);

  if ((useToolset == "v140") || (useToolset == "v141") ||
      (useToolset == "v142")) {
    return "v14";
  } else if (useToolset == "v120") {
    return "v12";
  } else if (useToolset == "v110") {
    return "v11";
  } else if (useToolset == "v100") {
    return "v10";
  } else {
    return "";
  }
}

std::string cmVisualStudio10ToolsetOptions::GetToolsetName(
  std::string const& name, std::string const& toolset) const
{
  static_cast<void>(name);
  std::size_t length = toolset.length();

  if (cmHasLiteralSuffix(toolset, "_xp")) {
    length -= 3;
  }

  return toolset.substr(0, length);
}
