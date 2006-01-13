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
                 None
  };
  class StringVec: public std::vector<cmStdString> {};
  static const char* PBXTypeNames[];
  virtual ~cmXCodeObject();
  cmXCodeObject(PBXType ptype, Type type);
  Type GetType() { return m_Type;}
  PBXType GetIsA() { return m_IsA;}

  void SetString(const char* s);
  const char* GetString() 
    {
      return m_String.c_str();
    }
  
  void AddAttribute(const char* name, cmXCodeObject* value)
    {
      m_ObjectAttributes[name] = value;
    }
  
  void SetObject(cmXCodeObject* value)
    {
      m_Object = value;
    }
  cmXCodeObject* GetObject()
    {
      return m_Object;
    }
  void AddObject(cmXCodeObject* value)
    {
      m_List.push_back(value);
    }
  bool HasObject(cmXCodeObject* o)
    {
      return !(std::find(m_List.begin(), m_List.end(), o) == m_List.end());
    }
  void AddUniqueObject(cmXCodeObject* value)
    {
      if(std::find(m_List.begin(), m_List.end(), value) == m_List.end())
        {
        m_List.push_back(value);
        }
    }
  static void Indent(int level, std::ostream& out);
  void Print(std::ostream& out);
  virtual void PrintComment(std::ostream&) {};

  static void PrintList(std::vector<cmXCodeObject*> const&, std::ostream& out);
  const char* GetId()
    {
      return m_Id.c_str();
    }
  cmTarget* GetcmTarget()
    {
      return m_cmTarget;
    }
  void SetcmTarget(cmTarget* t)
    {
      m_cmTarget = t;
    }
  const char* GetComment() {return m_Comment.c_str();}
  bool HasComment() { return (m_Comment.size() !=  0);}
  cmXCodeObject* GetObject(const char* name)
    {
      if(m_ObjectAttributes.count(name))
        {
        return m_ObjectAttributes[name];
        }
      return 0;
    }
  // serach the attribute list for an object of the specified type
  cmXCodeObject* GetObject(cmXCodeObject::PBXType t)
    {
      for(std::vector<cmXCodeObject*>::iterator i = m_List.begin();
          i != m_List.end(); ++i)
        {
        cmXCodeObject* o = *i;
        if(o->m_IsA == t)
          {
          return o;
          }
        }
      return 0;
    }
  
  cmXCodeObject* GetPBXTargetDependency()
    {
      return m_PBXTargetDependency;
    }
  void SetPBXTargetDependency(cmXCodeObject* d)
    {
      m_PBXTargetDependency = d;
    }
  void CopyAttributes(cmXCodeObject* );
  
  void AddDependLibrary(const char* configName,
                        const char* l)
    {
      if(!configName)
        {
        configName = "";
        }
      m_DependLibraries[configName].push_back(l);
    }
  std::map<cmStdString, StringVec> const& GetDependLibraries()
    {
      return m_DependLibraries;
    }
  std::vector<cmXCodeObject*> const& GetObjectList() { return m_List;}
  void SetComment(const char* c) { m_Comment = c;}
protected:
  cmTarget* m_cmTarget;
  Type m_Type;
  cmStdString m_Id;
  PBXType m_IsA;
  int m_Version;
  cmStdString m_Comment;
  cmStdString m_String;
  cmXCodeObject* m_Object;
  cmXCodeObject* m_PBXTargetDependency;
  std::vector<cmXCodeObject*> m_List;
  std::map<cmStdString, StringVec> m_DependLibraries;
  std::map<cmStdString, cmXCodeObject*> m_ObjectAttributes;
};
#endif
