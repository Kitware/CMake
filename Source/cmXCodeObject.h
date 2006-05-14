#ifndef cmXCodeObject_h
#define cmXCodeObject_h

#include "cmStandardIncludes.h"
class cmTarget;

class cmXCodeObject
{
public:
  enum Type { OBJECT_LIST, STRING, ATTRIBUTE_GROUP, OBJECT_REF, OBJECT };
  enum PBXType { PBXGroup, PBXBuildStyle, PBXProject, PBXHeadersBuildPhase, 
                 PBXSourcesBuildPhase, PBXFrameworksBuildPhase, 
                 PBXNativeTarget, PBXFileReference, PBXBuildFile, 
                 PBXContainerItemProxy, PBXTargetDependency,
                 PBXShellScriptBuildPhase, PBXResourcesBuildPhase,
                 PBXApplicationReference, PBXExecutableFileReference, 
                 PBXLibraryReference, PBXToolTarget, PBXLibraryTarget, 
                 PBXAggregateTarget,XCBuildConfiguration,XCConfigurationList,
                 PBXCopyFilesBuildPhase,
                 None
  };
  class StringVec: public std::vector<cmStdString> {};
  static const char* PBXTypeNames[];
  virtual ~cmXCodeObject();
  cmXCodeObject(PBXType ptype, Type type);
  Type GetType() { return this->TypeValue;}
  PBXType GetIsA() { return this->IsA;}

  void SetString(const char* s);
  const char* GetString() 
    {
      return this->String.c_str();
    }
  
  void AddAttribute(const char* name, cmXCodeObject* value)
    {
      this->ObjectAttributes[name] = value;
    }
  
  void SetObject(cmXCodeObject* value)
    {
      this->Object = value;
    }
  cmXCodeObject* GetObject()
    {
      return this->Object;
    }
  void AddObject(cmXCodeObject* value)
    {
      this->List.push_back(value);
    }
  bool HasObject(cmXCodeObject* o)
    {
    return !(std::find(this->List.begin(), this->List.end(), o) 
             == this->List.end());
    }
  void AddUniqueObject(cmXCodeObject* value)
    {
    if(std::find(this->List.begin(), this->List.end(), value) 
       == this->List.end())
        {
        this->List.push_back(value);
        }
    }
  static void Indent(int level, std::ostream& out);
  void Print(std::ostream& out);
  virtual void PrintComment(std::ostream&) {};

  static void PrintList(std::vector<cmXCodeObject*> const&, 
                        std::ostream& out);
  const char* GetId()
    {
      return this->Id.c_str();
    }
  cmTarget* GetTarget()
    {
      return this->Target;
    }
  void SetTarget(cmTarget* t)
    {
      this->Target = t;
    }
  const char* GetComment() {return this->Comment.c_str();}
  bool HasComment() { return (this->Comment.size() !=  0);}
  cmXCodeObject* GetObject(const char* name)
    {
      if(this->ObjectAttributes.count(name))
        {
        return this->ObjectAttributes[name];
        }
      return 0;
    }
  // serach the attribute list for an object of the specified type
  cmXCodeObject* GetObject(cmXCodeObject::PBXType t)
    {
      for(std::vector<cmXCodeObject*>::iterator i = this->List.begin();
          i != this->List.end(); ++i)
        {
        cmXCodeObject* o = *i;
        if(o->IsA == t)
          {
          return o;
          }
        }
      return 0;
    }
  
  cmXCodeObject* GetPBXTargetDependency()
    {
      return this->PBXTargetDependencyValue;
    }
  void SetPBXTargetDependency(cmXCodeObject* d)
    {
      this->PBXTargetDependencyValue = d;
    }
  void CopyAttributes(cmXCodeObject* );
  
  void AddDependLibrary(const char* configName,
                        const char* l)
    {
      if(!configName)
        {
        configName = "";
        }
      this->DependLibraries[configName].push_back(l);
    }
  std::map<cmStdString, StringVec> const& GetDependLibraries()
    {
      return this->DependLibraries;
    }
  std::vector<cmXCodeObject*> const& GetObjectList() { return this->List;}
  void SetComment(const char* c) { this->Comment = c;}
protected:
  cmTarget* Target;
  Type TypeValue;
  cmStdString Id;
  PBXType IsA;
  int Version;
  cmStdString Comment;
  cmStdString String;
  cmXCodeObject* Object;
  cmXCodeObject* PBXTargetDependencyValue;
  std::vector<cmXCodeObject*> List;
  std::map<cmStdString, StringVec> DependLibraries;
  std::map<cmStdString, cmXCodeObject*> ObjectAttributes;
};
#endif
