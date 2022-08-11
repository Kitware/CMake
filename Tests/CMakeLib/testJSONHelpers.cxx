#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <cm/optional>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmJSONHelpers.h"

#define ASSERT_TRUE(x)                                                        \
  do {                                                                        \
    if (!(x)) {                                                               \
      std::cout << "ASSERT_TRUE(" #x ") failed on line " << __LINE__ << "\n"; \
      return false;                                                           \
    }                                                                         \
  } while (false)

namespace {
struct ObjectStruct
{
  std::string Field1;
  int Field2;
};

struct InheritedStruct : public ObjectStruct
{
  std::string Field3;
};

enum class ErrorCode
{
  Success,
  InvalidInt,
  InvalidBool,
  InvalidString,
  InvalidSubObject,
  InvalidObject,
  InvalidArray,
  MissingRequired,
};

using JSONHelperBuilder = cmJSONHelperBuilder<ErrorCode>;

auto const IntHelper =
  JSONHelperBuilder::Int(ErrorCode::Success, ErrorCode::InvalidInt, 1);
auto const RequiredIntHelper =
  JSONHelperBuilder::Required<int>(ErrorCode::MissingRequired, IntHelper);
auto const UIntHelper =
  JSONHelperBuilder::UInt(ErrorCode::Success, ErrorCode::InvalidInt, 1);
auto const BoolHelper =
  JSONHelperBuilder::Bool(ErrorCode::Success, ErrorCode::InvalidBool, false);
auto const StringHelper = JSONHelperBuilder::String(
  ErrorCode::Success, ErrorCode::InvalidString, "default");
auto const RequiredStringHelper = JSONHelperBuilder::Required<std::string>(
  ErrorCode::MissingRequired, StringHelper);
auto const StringVectorHelper = JSONHelperBuilder::Vector<std::string>(
  ErrorCode::Success, ErrorCode::InvalidArray, StringHelper);
auto const StringVectorFilterHelper =
  JSONHelperBuilder::VectorFilter<std::string>(
    ErrorCode::Success, ErrorCode::InvalidArray, StringHelper,
    [](const std::string& value) { return value != "ignore"; });
auto const StringMapHelper = JSONHelperBuilder::Map<std::string>(
  ErrorCode::Success, ErrorCode::InvalidObject, StringHelper);
auto const StringMapFilterHelper = JSONHelperBuilder::MapFilter<std::string>(
  ErrorCode::Success, ErrorCode::InvalidObject, StringHelper,
  [](const std::string& key) { return key != "ignore"; });
auto const OptionalStringHelper =
  JSONHelperBuilder::Optional<std::string>(ErrorCode::Success, StringHelper);

bool testInt()
{
  Json::Value v(2);
  int i = 0;
  ASSERT_TRUE(IntHelper(i, &v) == ErrorCode::Success);
  ASSERT_TRUE(i == 2);

  i = 0;
  v = Json::nullValue;
  ASSERT_TRUE(IntHelper(i, &v) == ErrorCode::InvalidInt);

  i = 0;
  ASSERT_TRUE(IntHelper(i, nullptr) == ErrorCode::Success);
  ASSERT_TRUE(i == 1);

  return true;
}

bool testUInt()
{
  Json::Value v(2);
  unsigned int i = 0;
  ASSERT_TRUE(UIntHelper(i, &v) == ErrorCode::Success);
  ASSERT_TRUE(i == 2);

  i = 0;
  v = Json::nullValue;
  ASSERT_TRUE(UIntHelper(i, &v) == ErrorCode::InvalidInt);

  i = 0;
  ASSERT_TRUE(UIntHelper(i, nullptr) == ErrorCode::Success);
  ASSERT_TRUE(i == 1);

  return true;
}

bool testBool()
{
  Json::Value v(true);
  bool b = false;
  ASSERT_TRUE(BoolHelper(b, &v) == ErrorCode::Success);
  ASSERT_TRUE(b);

  b = false;
  v = false;
  ASSERT_TRUE(BoolHelper(b, &v) == ErrorCode::Success);
  ASSERT_TRUE(!b);

  b = false;
  v = 4;
  ASSERT_TRUE(BoolHelper(b, &v) == ErrorCode::InvalidBool);

  b = true;
  ASSERT_TRUE(BoolHelper(b, nullptr) == ErrorCode::Success);
  ASSERT_TRUE(!b);

  return true;
}

bool testString()
{
  Json::Value v("str");
  std::string str = "";
  ASSERT_TRUE(StringHelper(str, &v) == ErrorCode::Success);
  ASSERT_TRUE(str == "str");

  str = "";
  v = Json::nullValue;
  ASSERT_TRUE(StringHelper(str, &v) == ErrorCode::InvalidString);

  str = "";
  ASSERT_TRUE(StringHelper(str, nullptr) == ErrorCode::Success);
  ASSERT_TRUE(str == "default");

  return true;
}

bool testObject()
{
  auto const subhelper =
    JSONHelperBuilder::Object<ObjectStruct>(ErrorCode::Success,
                                            ErrorCode::InvalidSubObject)
      .Bind("subfield"_s, &ObjectStruct::Field2, IntHelper);
  auto const helper = JSONHelperBuilder::Object<ObjectStruct>(
                        ErrorCode::Success, ErrorCode::InvalidObject)
                        .Bind("field1"_s, &ObjectStruct::Field1, StringHelper)
                        .Bind("field2"_s, subhelper)
                        .Bind<std::string>("field3"_s, nullptr, StringHelper);

  Json::Value v(Json::objectValue);
  v["field1"] = "Hello";
  v["field2"] = Json::objectValue;
  v["field2"]["subfield"] = 2;
  v["field3"] = "world!";
  v["extra"] = "extra";

  ObjectStruct s1;
  ASSERT_TRUE(helper(s1, &v) == ErrorCode::Success);
  ASSERT_TRUE(s1.Field1 == "Hello");
  ASSERT_TRUE(s1.Field2 == 2);

  v["field2"]["subfield"] = "wrong";
  ObjectStruct s2;
  ASSERT_TRUE(helper(s2, &v) == ErrorCode::InvalidInt);

  v["field2"].removeMember("subfield");
  ObjectStruct s3;
  ASSERT_TRUE(helper(s3, &v) == ErrorCode::InvalidSubObject);

  v.removeMember("field2");
  ObjectStruct s4;
  ASSERT_TRUE(helper(s4, &v) == ErrorCode::InvalidObject);

  v["field2"] = Json::objectValue;
  v["field2"]["subfield"] = 2;
  v["field3"] = 3;
  ObjectStruct s5;
  ASSERT_TRUE(helper(s5, &v) == ErrorCode::InvalidString);

  v.removeMember("field3");
  ObjectStruct s6;
  ASSERT_TRUE(helper(s6, &v) == ErrorCode::InvalidObject);

  v = "Hello";
  ObjectStruct s7;
  ASSERT_TRUE(helper(s7, &v) == ErrorCode::InvalidObject);

  ObjectStruct s8;
  ASSERT_TRUE(helper(s8, nullptr) == ErrorCode::InvalidObject);

  return true;
}

bool testObjectInherited()
{
  auto const helper =
    JSONHelperBuilder::Object<InheritedStruct>(ErrorCode::Success,
                                               ErrorCode::InvalidObject)
      .Bind("field1"_s, &InheritedStruct::Field1, StringHelper)
      .Bind("field2"_s, &InheritedStruct::Field2, IntHelper)
      .Bind("field3"_s, &InheritedStruct::Field3, StringHelper);

  Json::Value v(Json::objectValue);
  v["field1"] = "Hello";
  v["field2"] = 2;
  v["field3"] = "world!";
  v["extra"] = "extra";

  InheritedStruct s1;
  ASSERT_TRUE(helper(s1, &v) == ErrorCode::Success);
  ASSERT_TRUE(s1.Field1 == "Hello");
  ASSERT_TRUE(s1.Field2 == 2);
  ASSERT_TRUE(s1.Field3 == "world!");

  v["field2"] = "wrong";
  InheritedStruct s2;
  ASSERT_TRUE(helper(s2, &v) == ErrorCode::InvalidInt);

  v.removeMember("field2");
  InheritedStruct s3;
  ASSERT_TRUE(helper(s3, &v) == ErrorCode::InvalidObject);

  v["field2"] = 2;
  v["field3"] = 3;
  InheritedStruct s4;
  ASSERT_TRUE(helper(s4, &v) == ErrorCode::InvalidString);

  v.removeMember("field3");
  InheritedStruct s5;
  ASSERT_TRUE(helper(s5, &v) == ErrorCode::InvalidObject);

  v = "Hello";
  InheritedStruct s6;
  ASSERT_TRUE(helper(s6, &v) == ErrorCode::InvalidObject);

  InheritedStruct s7;
  ASSERT_TRUE(helper(s7, nullptr) == ErrorCode::InvalidObject);

  return true;
}

bool testObjectNoExtra()
{
  auto const helper = JSONHelperBuilder::Object<ObjectStruct>(
                        ErrorCode::Success, ErrorCode::InvalidObject, false)
                        .Bind("field1"_s, &ObjectStruct::Field1, StringHelper)
                        .Bind("field2"_s, &ObjectStruct::Field2, IntHelper);

  Json::Value v(Json::objectValue);
  v["field1"] = "Hello";
  v["field2"] = 2;

  ObjectStruct s1;
  ASSERT_TRUE(helper(s1, &v) == ErrorCode::Success);
  ASSERT_TRUE(s1.Field1 == "Hello");
  ASSERT_TRUE(s1.Field2 == 2);

  v["extra"] = "world!";
  ObjectStruct s2;
  ASSERT_TRUE(helper(s2, &v) == ErrorCode::InvalidObject);

  return true;
}

bool testObjectOptional()
{
  auto const helper =
    JSONHelperBuilder::Object<ObjectStruct>(ErrorCode::Success,
                                            ErrorCode::InvalidObject)
      .Bind("field1"_s, &ObjectStruct::Field1, StringHelper, false)
      .Bind("field2"_s, &ObjectStruct::Field2, IntHelper, false)
      .Bind<std::string>("field3_s", nullptr, StringHelper, false);

  Json::Value v(Json::objectValue);
  v["field1"] = "Hello";
  v["field2"] = 2;
  v["field3"] = "world!";
  v["extra"] = "extra";

  ObjectStruct s1;
  ASSERT_TRUE(helper(s1, &v) == ErrorCode::Success);
  ASSERT_TRUE(s1.Field1 == "Hello");
  ASSERT_TRUE(s1.Field2 == 2);

  v = Json::objectValue;
  ObjectStruct s2;
  ASSERT_TRUE(helper(s2, &v) == ErrorCode::Success);
  ASSERT_TRUE(s2.Field1 == "default");
  ASSERT_TRUE(s2.Field2 == 1);

  ObjectStruct s3;
  ASSERT_TRUE(helper(s3, nullptr) == ErrorCode::Success);
  ASSERT_TRUE(s3.Field1 == "default");
  ASSERT_TRUE(s3.Field2 == 1);

  return true;
}

bool testVector()
{
  Json::Value v(Json::arrayValue);
  v.append("Hello");
  v.append("world!");
  v.append("ignore");

  std::vector<std::string> l{ "default" };
  std::vector<std::string> expected{ "Hello", "world!", "ignore" };
  ASSERT_TRUE(StringVectorHelper(l, &v) == ErrorCode::Success);
  ASSERT_TRUE(l == expected);

  v[1] = 2;
  l = { "default" };
  ASSERT_TRUE(StringVectorHelper(l, &v) == ErrorCode::InvalidString);

  v = "Hello";
  l = { "default" };
  ASSERT_TRUE(StringVectorHelper(l, &v) == ErrorCode::InvalidArray);

  l = { "default" };
  ASSERT_TRUE(StringVectorHelper(l, nullptr) == ErrorCode::Success);
  ASSERT_TRUE(l.empty());

  return true;
}

bool testVectorFilter()
{
  Json::Value v(Json::arrayValue);
  v.append("Hello");
  v.append("world!");
  v.append("ignore");

  std::vector<std::string> l{ "default" };
  std::vector<std::string> expected{
    "Hello",
    "world!",
  };
  ASSERT_TRUE(StringVectorFilterHelper(l, &v) == ErrorCode::Success);
  ASSERT_TRUE(l == expected);

  v[1] = 2;
  l = { "default" };
  ASSERT_TRUE(StringVectorFilterHelper(l, &v) == ErrorCode::InvalidString);

  v = "Hello";
  l = { "default" };
  ASSERT_TRUE(StringVectorFilterHelper(l, &v) == ErrorCode::InvalidArray);

  l = { "default" };
  ASSERT_TRUE(StringVectorFilterHelper(l, nullptr) == ErrorCode::Success);
  ASSERT_TRUE(l.empty());

  return true;
}

bool testMap()
{
  Json::Value v(Json::objectValue);
  v["field1"] = "Hello";
  v["field2"] = "world!";
  v["ignore"] = "ignore";

  std::map<std::string, std::string> m{ { "key", "default" } };
  std::map<std::string, std::string> expected{ { "field1", "Hello" },
                                               { "field2", "world!" },
                                               { "ignore", "ignore" } };
  ASSERT_TRUE(StringMapHelper(m, &v) == ErrorCode::Success);
  ASSERT_TRUE(m == expected);

  v = Json::arrayValue;
  m = { { "key", "default" } };
  ASSERT_TRUE(StringMapHelper(m, &v) == ErrorCode::InvalidObject);

  m = { { "key", "default" } };
  ASSERT_TRUE(StringMapHelper(m, nullptr) == ErrorCode::Success);
  ASSERT_TRUE(m.empty());

  return true;
}

bool testMapFilter()
{
  Json::Value v(Json::objectValue);
  v["field1"] = "Hello";
  v["field2"] = "world!";
  v["ignore"] = "ignore";

  std::map<std::string, std::string> m{ { "key", "default" } };
  std::map<std::string, std::string> expected{ { "field1", "Hello" },
                                               { "field2", "world!" } };
  ASSERT_TRUE(StringMapFilterHelper(m, &v) == ErrorCode::Success);
  ASSERT_TRUE(m == expected);

  v = Json::arrayValue;
  m = { { "key", "default" } };
  ASSERT_TRUE(StringMapFilterHelper(m, &v) == ErrorCode::InvalidObject);

  m = { { "key", "default" } };
  ASSERT_TRUE(StringMapFilterHelper(m, nullptr) == ErrorCode::Success);
  ASSERT_TRUE(m.empty());

  return true;
}

bool testOptional()
{
  Json::Value v = "Hello";

  cm::optional<std::string> str{ "default" };
  ASSERT_TRUE(OptionalStringHelper(str, &v) == ErrorCode::Success);
  ASSERT_TRUE(str == "Hello");

  str.emplace("default");
  ASSERT_TRUE(OptionalStringHelper(str, nullptr) == ErrorCode::Success);
  ASSERT_TRUE(str == cm::nullopt);

  return true;
}

bool testRequired()
{
  Json::Value v = "Hello";

  std::string str = "default";
  int i = 1;
  ASSERT_TRUE(RequiredStringHelper(str, &v) == ErrorCode::Success);
  ASSERT_TRUE(str == "Hello");
  ASSERT_TRUE(RequiredIntHelper(i, &v) == ErrorCode::InvalidInt);

  v = 2;
  str = "default";
  i = 1;
  ASSERT_TRUE(RequiredStringHelper(str, &v) == ErrorCode::InvalidString);
  ASSERT_TRUE(RequiredIntHelper(i, &v) == ErrorCode::Success);
  ASSERT_TRUE(i == 2);

  str = "default";
  i = 1;
  ASSERT_TRUE(RequiredStringHelper(str, nullptr) ==
              ErrorCode::MissingRequired);
  ASSERT_TRUE(RequiredIntHelper(i, nullptr) == ErrorCode::MissingRequired);

  return true;
}
}

int testJSONHelpers(int /*unused*/, char* /*unused*/ [])
{
  if (!testInt()) {
    return 1;
  }
  if (!testUInt()) {
    return 1;
  }
  if (!testBool()) {
    return 1;
  }
  if (!testString()) {
    return 1;
  }
  if (!testObject()) {
    return 1;
  }
  if (!testObjectInherited()) {
    return 1;
  }
  if (!testObjectNoExtra()) {
    return 1;
  }
  if (!testObjectOptional()) {
    return 1;
  }
  if (!testVector()) {
    return 1;
  }
  if (!testVectorFilter()) {
    return 1;
  }
  if (!testMap()) {
    return 1;
  }
  if (!testMapFilter()) {
    return 1;
  }
  if (!testOptional()) {
    return 1;
  }
  if (!testRequired()) {
    return 1;
  }
  return 0;
}
