#include "cmXCodeObject.h"
const char* cmXCodeObject::PBXTypeNames[] = {
    "PBXGroup", "PBXBuildStyle", "PBXProject", "PBXHeadersBuildPhase", 
    "PBXSourcesBuildPhase", "PBXFrameworksBuildPhase", "PBXNativeTarget",
    "PBXFileReference", "PBXBuildFile", "PBXContainerItemProxy", "PBXTargetDependency",
    "PBXShellScriptBuildPhase", "PBXResourcesBuildPhase", "PBXApplicationReference",
    "PBXExecutableFileReference", "PBXLibraryReference", "PBXToolTarget", "PBXLibraryTarget",
    "None"
  };

//----------------------------------------------------------------------------
cmXCodeObject::cmXCodeObject(PBXType ptype, Type type)
{
  m_IsA = ptype;
  cmOStringStream str;
  str << (void*)this;
  m_Id = str.str();
  m_Type = type;
  if(m_Type == OBJECT)
    {
    this->AddAttribute("isa", PBXTypeNames[m_IsA]);
    }
}

//----------------------------------------------------------------------------
void cmXCodeObject::Indent(int level, std::ostream& out)
{
  while(level)
    {
    out << "       ";
    level--;
    }
}

//----------------------------------------------------------------------------
void cmXCodeObject::Print(std::ostream& out)
{
  cmXCodeObject::Indent(2, out);
  out << m_Id << " = {\n";
  std::map<cmStdString, cmXCodeObject*>::iterator i;
  for(i = m_ObjectAttributes.begin(); i != m_ObjectAttributes.end(); ++i)
    { 
    cmXCodeObject* object = i->second;
    if(object->m_Type == OBJECT_LIST)
      {
      cmXCodeObject::Indent(3, out);
      out << i->first << " = {\n";
      for(unsigned int k = 0; k < i->second->m_List.size(); k++)
        {
        cmXCodeObject::Indent(4, out);
        out << i->second->m_List[k]->m_Id << ",\n";
        } 
      cmXCodeObject::Indent(3, out);
      out << "};\n";
      }
    else if(object->m_Type == ATTRIBUTE_GROUP)
      {
      std::map<cmStdString, cmStdString>::iterator j;
      cmXCodeObject::Indent(3, out);
      out << i->first << " = {\n";
      for(j = object->m_StringAttributes.begin(); j != object->m_StringAttributes.end(); ++j)
        {
        cmXCodeObject::Indent(4, out);
        out << j->first << " = " << j->second << ";\n";
        }
      cmXCodeObject::Indent(3, out);
      out << "}\n";
      }
    else if(object->m_Type == OBJECT_REF)
      {
      cmXCodeObject::Indent(3, out);
      out << i->first << " = " << object->m_Object->m_Id << ";\n";
      }
        
    }
  std::map<cmStdString, cmStdString>::iterator j;
  for(j = m_StringAttributes.begin(); j != m_StringAttributes.end(); ++j)
    {
    cmXCodeObject::Indent(3, out);
    out << j->first << " = " << j->second << ";\n";
    }
  cmXCodeObject::Indent(2, out);
  out << "};\n";
}
  
//----------------------------------------------------------------------------
void cmXCodeObject::PrintList(std::vector<cmXCodeObject*> const& objs,
                              std::ostream& out)
{ 
  cmXCodeObject::Indent(1, out);
  out << "objects = {\n";
  for(unsigned int i = 0; i < objs.size(); ++i)
    {
    if(objs[i]->m_Type == OBJECT)
      {
      objs[i]->Print(out);
      }
    }
  cmXCodeObject::Indent(1, out);
  out << "};\n";
}
