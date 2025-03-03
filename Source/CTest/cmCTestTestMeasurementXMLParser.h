/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

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
  void StartElement(std::string const& name, char const** atts) override;
  void EndElement(std::string const& /*name*/) override {}
  void CharacterDataHandler(char const* data, int length) override;
};
