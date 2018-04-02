/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmXMLWiter_h
#define cmXMLWiter_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmXMLSafe.h"

#include <chrono>
#include <ctime>
#include <ostream>
#include <stack>
#include <string>
#include <vector>

class cmXMLWriter
{
  CM_DISABLE_COPY(cmXMLWriter)

public:
  cmXMLWriter(std::ostream& output, std::size_t level = 0);
  ~cmXMLWriter();

  void StartDocument(const char* encoding = "UTF-8");
  void EndDocument();

  void StartElement(std::string const& name);
  void EndElement();

  void BreakAttributes();

  template <typename T>
  void Attribute(const char* name, T const& value)
  {
    this->PreAttribute();
    this->Output << name << "=\"" << SafeAttribute(value) << '"';
  }

  void Element(const char* name);

  template <typename T>
  void Element(std::string const& name, T const& value)
  {
    this->StartElement(name);
    this->Content(value);
    this->EndElement();
  }

  template <typename T>
  void Content(T const& content)
  {
    this->PreContent();
    this->Output << SafeContent(content);
  }

  void Comment(const char* comment);

  void CData(std::string const& data);

  void Doctype(const char* doctype);

  void ProcessingInstruction(const char* target, const char* data);

  void FragmentFile(const char* fname);

  void SetIndentationElement(std::string const& element);

private:
  void ConditionalLineBreak(bool condition, std::size_t indent);

  void PreAttribute();
  void PreContent();

  void CloseStartElement();

private:
  static cmXMLSafe SafeAttribute(const char* value)
  {
    return cmXMLSafe(value);
  }

  static cmXMLSafe SafeAttribute(std::string const& value)
  {
    return cmXMLSafe(value);
  }

  template <typename T>
  static T SafeAttribute(T value)
  {
    return value;
  }

  static cmXMLSafe SafeContent(const char* value)
  {
    return cmXMLSafe(value).Quotes(false);
  }

  static cmXMLSafe SafeContent(std::string const& value)
  {
    return cmXMLSafe(value).Quotes(false);
  }

  /*
   * Convert a std::chrono::system::time_point to the number of seconds since
   * the UN*X epoch.
   *
   * It would be tempting to convert a time_point to number of seconds by
   * using time_since_epoch(). Unfortunately the C++11 standard does not
   * specify what the epoch of the system_clock must be.
   * Therefore we must assume it is an arbitrary point in time. Instead of this
   * method, it is recommended to convert it by means of the to_time_t method.
   */
  static std::time_t SafeContent(
    std::chrono::system_clock::time_point const& value)
  {
    return std::chrono::system_clock::to_time_t(value);
  }

  template <typename T>
  static T SafeContent(T value)
  {
    return value;
  }

private:
  std::ostream& Output;
  std::stack<std::string, std::vector<std::string>> Elements;
  std::string IndentationElement;
  std::size_t Level;
  bool ElementOpen;
  bool BreakAttrib;
  bool IsContent;
};

#endif
