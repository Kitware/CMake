/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmVisualStudio10ToolsetOptions_h
#define cmVisualStudio10ToolsetOptions_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** \class cmVisualStudio10ToolsetOptions
 * \brief Retrieves toolset options for MSBuild.
 *
 * cmVisualStudio10ToolsetOptions manages toolsets within MSBuild
 */
class cmVisualStudio10ToolsetOptions
{
public:
  std::string GetClFlagTableName(std::string const& name,
                                 std::string const& toolset) const;
  std::string GetCSharpFlagTableName(std::string const& name,
                                     std::string const& toolset) const;
  std::string GetRcFlagTableName(std::string const& name,
                                 std::string const& toolset) const;
  std::string GetLibFlagTableName(std::string const& name,
                                  std::string const& toolset) const;
  std::string GetLinkFlagTableName(std::string const& name,
                                   std::string const& toolset) const;
  std::string GetMasmFlagTableName(std::string const& name,
                                   std::string const& toolset) const;
  std::string GetToolsetName(std::string const& name,
                             std::string const& toolset) const;
};
#endif
