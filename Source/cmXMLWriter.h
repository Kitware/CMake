/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <chrono>
#include <cstddef> // IWYU pragma: keep
#include <ctime>
#include <ostream>
#include <stack>
#include <string>
#include <vector>

#include "cmXMLSafe.h"

class cmXMLWriter
{
public:
  cmXMLWriter(std::ostream& output, std::size_t level = 0);
  ~cmXMLWriter();

  cmXMLWriter(cmXMLWriter const&) = delete;
  cmXMLWriter& operator=(cmXMLWriter const&) = delete;

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
  void ConditionalLineBreak(bool condition);

  void PreAttribute();
  void PreContent();

  void CloseStartElement();

  static cmXMLSafe SafeAttribute(const char* value) { return { value }; }

  static cmXMLSafe SafeAttribute(std::string const& value)
  {
    return { value };
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

  std::ostream& Output;
  std::stack<std::string, std::vector<std::string>> Elements;
  std::string IndentationElement;
  std::size_t Level;
  std::size_t Indent = 0;
  bool ElementOpen = false;
  bool BreakAttrib = false;
  bool IsContent = false;
};

class cmXMLElement; // IWYU pragma: keep

class cmXMLDocument
{
public:
  cmXMLDocument(cmXMLWriter& xml)
    : xmlwr(xml)
  {
    this->xmlwr.StartDocument();
  }
  ~cmXMLDocument() { this->xmlwr.EndDocument(); }
  cmXMLDocument(const cmXMLDocument&) = delete;
  cmXMLDocument& operator=(const cmXMLDocument&) = delete;

private:
  friend class cmXMLElement;
  cmXMLWriter& xmlwr;
};

class cmXMLElement
{
public:
  cmXMLElement(cmXMLWriter& xml, const char* tag)
    : xmlwr(xml)
  {
    this->xmlwr.StartElement(tag);
  }
  cmXMLElement(cmXMLElement& par, const char* tag)
    : xmlwr(par.xmlwr)
  {
    this->xmlwr.StartElement(tag);
  }
  cmXMLElement(cmXMLDocument& doc, const char* tag)
    : xmlwr(doc.xmlwr)
  {
    this->xmlwr.StartElement(tag);
  }
  ~cmXMLElement() { this->xmlwr.EndElement(); }

  cmXMLElement(const cmXMLElement&) = delete;
  cmXMLElement& operator=(const cmXMLElement&) = delete;

  template <typename T>
  cmXMLElement& Attribute(const char* name, T const& value)
  {
    this->xmlwr.Attribute(name, value);
    return *this;
  }
  template <typename T>
  void Content(T const& content)
  {
    this->xmlwr.Content(content);
  }
  template <typename T>
  void Element(std::string const& name, T const& value)
  {
    this->xmlwr.Element(name, value);
  }
  void Comment(const char* comment) { this->xmlwr.Comment(comment); }

private:
  cmXMLWriter& xmlwr;
};
