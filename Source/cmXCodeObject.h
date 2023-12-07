/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <cstddef>
#include <iosfwd>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cmext/algorithm>

class cmGeneratorTarget;

class cmXCodeObject
{
public:
  enum Type
  {
    OBJECT_LIST,
    STRING,
    ATTRIBUTE_GROUP,
    OBJECT_REF,
    OBJECT
  };
  enum PBXType
  {
    PBXGroup,
    PBXBuildStyle,
    PBXProject,
    PBXHeadersBuildPhase,
    PBXSourcesBuildPhase,
    PBXFrameworksBuildPhase,
    PBXNativeTarget,
    PBXFileReference,
    PBXBuildFile,
    PBXContainerItemProxy,
    PBXTargetDependency,
    PBXShellScriptBuildPhase,
    PBXResourcesBuildPhase,
    PBXApplicationReference,
    PBXExecutableFileReference,
    PBXLibraryReference,
    PBXToolTarget,
    PBXLibraryTarget,
    PBXAggregateTarget,
    XCBuildConfiguration,
    XCConfigurationList,
    PBXCopyFilesBuildPhase,
    None
  };
  class StringVec : public std::vector<std::string>
  {
  };
  static const char* PBXTypeNames[];
  virtual ~cmXCodeObject();
  cmXCodeObject(PBXType ptype, Type type, std::string id);
  Type GetType() const { return this->TypeValue; }
  PBXType GetIsA() const { return this->IsA; }

  bool IsEmpty() const;

  void SetString(const std::string& s);
  const std::string& GetString() const { return this->String; }

  void AddAttribute(const std::string& name, cmXCodeObject* value)
  {
    this->ObjectAttributes[name] = value;
  }

  void AddAttributeIfNotEmpty(const std::string& name, cmXCodeObject* value)
  {
    if (value && !value->IsEmpty()) {
      AddAttribute(name, value);
    }
  }

  void SetObject(cmXCodeObject* value) { this->Object = value; }
  cmXCodeObject* GetObject() { return this->Object; }
  void AddObject(cmXCodeObject* value) { this->List.push_back(value); }
  size_t GetObjectCount() { return this->List.size(); }
  void InsertObject(size_t position, cmXCodeObject* value)
  {
    if (position < GetObjectCount()) {
      this->List.insert(this->List.begin() + position, value);
    }
  }
  void PrependObject(cmXCodeObject* value)
  {
    this->List.insert(this->List.begin(), value);
  }
  bool HasObject(cmXCodeObject* o) const
  {
    return cm::contains(this->List, o);
  }
  void AddUniqueObject(cmXCodeObject* value)
  {
    if (!cm::contains(this->List, value)) {
      this->List.push_back(value);
    }
  }
  static void Indent(int level, std::ostream& out);
  void Print(std::ostream& out);
  void PrintAttribute(std::ostream& out, int level,
                      const std::string& separator, int factor,
                      const std::string& name, const cmXCodeObject* object,
                      const cmXCodeObject* parent);
  virtual void PrintComment(std::ostream&) {}

  static void PrintList(std::vector<cmXCodeObject*> const&, std::ostream& out);
  const std::string& GetId() const { return this->Id; }
  void SetId(const std::string& id) { this->Id = id; }
  cmGeneratorTarget* GetTarget() const { return this->Target; }
  void SetTarget(cmGeneratorTarget* t) { this->Target = t; }
  const std::string& GetComment() const { return this->Comment; }
  bool HasComment() const { return (!this->Comment.empty()); }
  cmXCodeObject* GetAttribute(const char* name) const
  {
    auto const i = this->ObjectAttributes.find(name);
    if (i != this->ObjectAttributes.end()) {
      return i->second;
    }
    return nullptr;
  }
  // search the attribute list for an object of the specified type
  cmXCodeObject* GetObject(cmXCodeObject::PBXType t) const
  {
    for (auto* o : this->List) {
      if (o->IsA == t) {
        return o;
      }
    }
    return nullptr;
  }

  void CopyAttributes(cmXCodeObject*);

  void AddDependLibrary(const std::string& configName, const std::string& l)
  {
    this->DependLibraries[configName].push_back(l);
  }
  std::map<std::string, StringVec> const& GetDependLibraries() const
  {
    return this->DependLibraries;
  }
  void AddDependTarget(const std::string& configName, const std::string& tName)
  {
    this->DependTargets[configName].push_back(tName);
  }
  std::map<std::string, StringVec> const& GetDependTargets() const
  {
    return this->DependTargets;
  }
  std::vector<cmXCodeObject*> const& GetObjectList() const
  {
    return this->List;
  }
  void SetComment(const std::string& c) { this->Comment = c; }
  static void PrintString(std::ostream& os, const std::string& String);

protected:
  void PrintString(std::ostream& os) const;

  cmGeneratorTarget* Target;
  Type TypeValue;
  std::string Id;
  PBXType IsA;
  int Version;
  std::string Comment;
  std::string String;
  cmXCodeObject* Object;
  std::vector<cmXCodeObject*> List;
  std::map<std::string, StringVec> DependLibraries;
  std::map<std::string, StringVec> DependTargets;
  std::map<std::string, cmXCodeObject*> ObjectAttributes;
};
