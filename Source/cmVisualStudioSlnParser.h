/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <bitset>
#include <cstddef>
#include <iosfwd>
#include <string>

#include <cm/string_view>

class cmSlnData;

class cmVisualStudioSlnParser
{
public:
  enum ParseResult
  {
    ResultOK = 0,

    ResultInternalError = -1,
    ResultExternalError = 1,

    ResultErrorOpeningInput = ResultExternalError,
    ResultErrorReadingInput,
    ResultErrorInputStructure,
    ResultErrorInputData,

    ResultErrorBadInternalState = ResultInternalError,
    ResultErrorUnsupportedDataGroup = ResultInternalError - 1
  };

  enum DataGroup
  {
    DataGroupProjectsBit,
    DataGroupProjectDependenciesBit,
    DataGroupSolutionConfigurationsBit,
    DataGroupProjectConfigurationsBit,
    DataGroupSolutionFiltersBit,
    DataGroupGenericGlobalSectionsBit,
    DataGroupCount
  };

  using DataGroupSet = std::bitset<DataGroupCount>;

  static DataGroupSet const DataGroupProjects;
  static DataGroupSet const DataGroupProjectDependencies;
  static DataGroupSet const DataGroupSolutionConfigurations;
  static DataGroupSet const DataGroupProjectConfigurations;
  static DataGroupSet const DataGroupSolutionFilters;
  static DataGroupSet const DataGroupGenericGlobalSections;
  static DataGroupSet const DataGroupAll;

  bool Parse(std::istream& input, cmSlnData& output,
             DataGroupSet dataGroups = DataGroupAll);

  bool ParseFile(std::string const& file, cmSlnData& output,
                 DataGroupSet dataGroups = DataGroupAll);

  ParseResult GetParseResult() const;

  size_t GetParseResultLine() const;

  bool GetParseHadBOM() const;

protected:
  class State;

  friend class State;
  class ParsedLine;

  struct ResultData
  {
    ParseResult Result = ResultOK;
    size_t ResultLine = 0;
    bool HadBOM;

    ResultData();
    void Clear();
    void SetError(ParseResult error, size_t line);
  } LastResult;

  bool IsDataGroupSetSupported(DataGroupSet dataGroups) const;

  bool ParseImpl(std::istream& input, cmSlnData& output, State& state);

  bool ParseBOM(std::istream& input, std::string& line, State& state);

  bool ParseMultiValueTag(std::string const& line, ParsedLine& parsedLine,
                          State& state);

  bool ParseSingleValueTag(std::string const& line, ParsedLine& parsedLine,
                           State& state);

  bool ParseKeyValuePair(std::string const& line, ParsedLine& parsedLine,
                         State& state);

  bool ParseTag(cm::string_view fullTag, ParsedLine& parsedLine, State& state);

  bool ParseValue(std::string const& value, ParsedLine& parsedLine);
};
