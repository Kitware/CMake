#include "cmXCodeObject.h"
#include "cmSystemTools.h"

const char* cmXCodeObject::PBXTypeNames[] = {
    "PBXGroup", "PBXBuildStyle", "PBXProject", "PBXHeadersBuildPhase", 
    "PBXSourcesBuildPhase", "PBXFrameworksBuildPhase", "PBXNativeTarget",
    "PBXFileReference", "PBXBuildFile", "PBXContainerItemProxy", 
    "PBXTargetDependency", "PBXShellScriptBuildPhase", 
    "PBXResourcesBuildPhase", "PBXApplicationReference",
    "PBXExecutableFileReference", "PBXLibraryReference", "PBXToolTarget",
    "PBXLibraryTarget", "PBXAggregateTarget", "XCBuildConfiguration", 
    "XCConfigurationList",
    "None"
  };

cmXCodeObject::~cmXCodeObject()
{
  this->Version = 15;
}


//----------------------------------------------------------------------------
cmXCodeObject::cmXCodeObject(PBXType ptype, Type type)
{
  this->Version = 15;
  this->PBXTargetDependency = 0;
  this->cmTarget = 0;
  this->Object =0;
  
  this->IsA = ptype;
  if(type == OBJECT)
    {
    cmOStringStream str;
    str << (void*)this;
    str << (void*)this;
    str << (void*)this;
    this->Id = str.str();
    }
  else
    {
    this->Id = "Temporary cmake object, should not be refered to in xcode file";
    }
  cmSystemTools::ReplaceString(this->Id, "0x", "");
  this->Id = cmSystemTools::UpperCase(this->Id);
  if(this->Id.size() < 24)
    {
    int diff = 24 - this->Id.size();
    for(int i =0; i < diff; ++i)
      {
      this->Id += "0";
      }
    }
  this->Type = type;
  if(this->Type == OBJECT)
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
  std::string separator = "\n";
  int indentFactor = 1;
  if(this->Version > 15 && (this->IsA == PBXFileReference || this->IsA == PBXBuildFile))
    {
    separator = " ";
    indentFactor = 0;
    }
  cmXCodeObject::Indent(2*indentFactor, out);
  out << this->Id << " ";
  if(!(this->this->IsA == PBXGroup && this->this->Comment.size() == 0))
    {
    this->PrintComment(out);
    }
  out << " = {";
  if(separator == "\n")
    {
    out << separator;
    }
  std::map<cmStdString, cmXCodeObject*>::iterator i;
  cmXCodeObject::Indent(3*indentFactor, out);
  out << "isa = " << PBXTypeNames[this->IsA]  << ";" << separator;
  for(i = this->ObjectAttributes.begin(); i != this->ObjectAttributes.end(); ++i)
    { 
    cmXCodeObject* object = i->second;
    if(i->first != "isa")
      { 
      cmXCodeObject::Indent(3*indentFactor, out);
      }
    else
      {
      continue;
      }
    if(object->this->Type == OBJECT_LIST)
      {
      out << i->first << " = (" << separator;
      for(unsigned int k = 0; k < i->second->this->List.size(); k++)
        {
        cmXCodeObject::Indent(4*indentFactor, out);
        out << i->second->this->List[k]->this->Id << " ";
        i->second->this->List[k]->PrintComment(out);
        out << "," << separator;
        } 
      cmXCodeObject::Indent(3*indentFactor, out);
      out << ");" << separator;
      }
    else if(object->this->Type == ATTRIBUTE_GROUP)
      {
      std::map<cmStdString, cmXCodeObject*>::iterator j;
      out << i->first << " = {" << separator;
      for(j = object->ObjectAttributes.begin(); j != 
            object->ObjectAttributes.end(); ++j)
        {
        cmXCodeObject::Indent(4 *indentFactor, out);
        out << j->first << " = " << j->second->this->String << ";";
        out << separator;
        }
      cmXCodeObject::Indent(3 *indentFactor, out);
      out << "};" << separator;
      }
    else if(object->this->Type == OBJECT_REF)
      {
      out << i->first << " = " << object->this->Object->this->Id;
      if(object->this->Object->HasComment() && i->first != "remoteGlobalIDString")
        {
        out << " ";
        object->this->Object->PrintComment(out);
        }
      out << ";" << separator;
      }
    else if(object->this->Type == STRING)
      {
      out << i->first << " = " << object->this->String << ";" << separator;
      }
    else
      {
      out << "what is this?? " << i->first << "\n";
      }
    }
  cmXCodeObject::Indent(2*indentFactor, out);
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
    if(objs[i]->this->Type == OBJECT)
      {
      objs[i]->Print(out);
      }
    }
  cmXCodeObject::Indent(1, out);
  out << "};\n";
}


void cmXCodeObject::CopyAttributes(cmXCodeObject* copy)
{
  this->this->ObjectAttributes = copy->this->ObjectAttributes;
  this->this->List = copy->this->List;
  this->this->String = copy->this->String;
  this->this->Object = copy->this->Object;
}

void cmXCodeObject::SetString(const char* s)
{
  std::string ss = s;
  if(ss.size() == 0)
    {
    this->String = "\"\"";
    return;
    }
  bool needQuote = false;
  this->String = "";
  if(ss.find_first_of(" <>.+-=") != ss.npos)
    {
    needQuote = true;
    }
  if(needQuote)
    {
    this->String = "\"";
    }
  this->String += s;
  if(needQuote)
    {
    this->String += "\"";
    }
}
