/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>

#include "cmValue.h"

class cmCPackIFWGenerator;
class cmXMLWriter;

/** \class cmCPackIFWCommon
 * \brief A base class for CPack IFW generator implementation subclasses
 */
class cmCPackIFWCommon
{
public:
  // Constructor

  /**
   * Construct Part
   */
  cmCPackIFWCommon();

public:
  // Internal implementation

  cmValue GetOption(std::string const& op) const;
  bool IsOn(std::string const& op) const;
  bool IsSetToOff(std::string const& op) const;
  bool IsSetToEmpty(std::string const& op) const;

  /**
   * Compare \a version with QtIFW framework version
   */
  bool IsVersionLess(char const* version) const;

  /**
   * Compare \a version with QtIFW framework version
   */
  bool IsVersionGreater(char const* version) const;

  /**
   * Compare \a version with QtIFW framework version
   */
  bool IsVersionEqual(char const* version) const;

  /** Expand the list argument containing the map of the key-value pairs.
   *  If the number of elements is odd, then the first value is used as the
   *  default value with an empty key.
   *  Any values with the same keys will be permanently overwritten.
   */
  static void ExpandListArgument(std::string const& arg,
                                 std::map<std::string, std::string>& argsOut);

  /** Expand the list argument containing the multimap of the key-value pairs.
   *  If the number of elements is odd, then the first value is used as the
   *  default value with an empty key.
   */
  static void ExpandListArgument(
    std::string const& arg, std::multimap<std::string, std::string>& argsOut);

  cmCPackIFWGenerator* Generator;

protected:
  void WriteGeneratedByToStrim(cmXMLWriter& xout) const;
};

#define cmCPackIFWLogger(logType, msg)                                        \
  do {                                                                        \
    std::ostringstream cmCPackLog_msg;                                        \
    cmCPackLog_msg << msg;                                                    \
    if (Generator) {                                                          \
      Generator->Logger->Log(cmCPackLog::LOG_##logType, __FILE__, __LINE__,   \
                             cmCPackLog_msg.str().c_str());                   \
    }                                                                         \
  } while (false)
