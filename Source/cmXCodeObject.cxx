#include "cmXCodeObject.h"
#include "cmSystemTools.h"

const char* cmXCodeObject::PBXTypeNames[] = {
    "PBXGroup", "PBXBuildStyle", "PBXProject", "PBXHeadersBuildPhase", 
    "PBXSourcesBuildPhase", "PBXFrameworksBuildPhase", "PBXNativeTarget",
    "PBXFileReference", "PBXBuildFile", "PBXContainerItemProxy", 
    "PBXTargetDependency", "PBXShellScriptBuildPhase", 
    "PBXResourcesBuildPhase", "PBXApplicationReference",
    "PBXExecutableFileReference", "PBXLibraryReference", "PBXToolTarget",
    "PBXLibraryTarget", "PBXAggregateTarget",
    "None"
  };

cmXCodeObject::~cmXCodeObject()
{
}


//----------------------------------------------------------------------------
cmXCodeObject::cmXCodeObject(PBXType ptype, Type type)
{
//  m_cmTarget = 0;
  m_IsA = ptype;
  cmOStringStream str;
  str << (void*)this;
  str << (void*)this;
  str << (void*)this;
  m_Id = str.str();
  cmSystemTools::ReplaceString(m_Id, "0x", "");
  if(m_Id.size() < 24)
    {
    int diff = 24 - m_Id.size();
    for(int i =0; i < diff; ++i)
      {
      m_Id += "0";
      }
    }
  m_Type = type;
  if(m_Type == OBJECT)
    {
    this->AddAttribute("isa", 0);
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
    cmXCodeObject::Indent(3, out);
    if(i->first == "isa")
      { 
      out << i->first << " = " << PBXTypeNames[m_IsA] << ";\n";
      }
    else if(object->m_Type == OBJECT_LIST)
      {
      out << i->first << " = (\n";
      for(unsigned int k = 0; k < i->second->m_List.size(); k++)
        {
        cmXCodeObject::Indent(4, out);
        out << i->second->m_List[k]->m_Id << ",\n";
        } 
      cmXCodeObject::Indent(3, out);
      out << ");\n";
      }
    else if(object->m_Type == ATTRIBUTE_GROUP)
      {
      std::map<cmStdString, cmXCodeObject*>::iterator j;
      out << i->first << " = {\n";
      for(j = object->m_ObjectAttributes.begin(); j != object->m_ObjectAttributes.end(); ++j)
        {
        cmXCodeObject::Indent(4, out);
        out << j->first << " = " << j->second->m_String << ";\n";
        }
      cmXCodeObject::Indent(3, out);
      out << "};\n";
      }
    else if(object->m_Type == OBJECT_REF)
      {
      out << i->first << " = " << object->m_Object->m_Id << ";\n";
      }
    else if(object->m_Type == STRING)
      {
      out << i->first << " = " << object->m_String << ";\n";
      }
    else
      {
      out << "what is this?? " << i->first << "\n";
      }
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
