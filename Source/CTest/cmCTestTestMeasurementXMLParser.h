/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <string>

#include "cmXMLParser.h"

class cmCTestTestMeasurementXMLParser : public cmXMLParser
{
public:
  cmCTestTestMeasurementXMLParser() {}
  std::string CharacterData;
  std::string ElementName;
  std::string MeasurementName;
  std::string MeasurementType;

protected:
  void StartElement(const std::string& name, const char** atts) override;
  void EndElement(const std::string& /*name*/) override {}
  void CharacterDataHandler(const char* data, int length) override;
};
