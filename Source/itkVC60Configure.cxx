#include "itkVC60Configure.h"
#include "cmSystemTools.h"
#include "stdlib.h"
#include <windows.h>

void itkVC60Configure::Configure()
{
  this->GenerateITKConfigHeader();
  this->GenerateVNLConfigHeader();
}

void itkVC60Configure::GenerateITKConfigHeader()
{
  // for now just copy the itkConfigure.h.in file into place
  std::string source = m_WhereSource;
  source += "/itkConfigure.h.in";
  std::string destdir = m_WhereBuild;
  std::string dest = destdir;
  dest += "/itkConfigure.h";
  this->CopyFileTo(source.c_str(),
                   destdir.c_str(),
                   dest.c_str());
}

void itkVC60Configure::CopyFileTo(const char* source,
                                  const char* destdir,
                                  const char* dest)
{
  if(!cmSystemTools::MakeDirectory(destdir) )
    {
    std::string error = "Error: can not create directory: ";
    error += destdir;
    MessageBox(0, error.c_str(), "config ERROR", MB_OK);
    }
  if(!CopyFile(source, dest, FALSE))
    {
     std::string error = "Error: can not create : ";
     error += dest;
     MessageBox(0, error.c_str(), "config ERROR", MB_OK);
    }
}


void itkVC60Configure::GenerateVNLConfigHeader()
{
  // Copy the vcl config stuff for vc50 into place
  std::string source = m_WhereSource;
  source += "/Code/Insight3DParty/vxl/vcl/vcl_config-vc60.h ";
  std::string destdir = m_WhereBuild;
  destdir += "/Code/Insight3DParty/vxl/vcl";
  std::string dest = destdir;
  dest += "/vcl_config.h";
  this->CopyFileTo(source.c_str(),
                   destdir.c_str(),
                   dest.c_str());
  
}
