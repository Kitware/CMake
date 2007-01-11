#include <cmsys/SystemTools.hxx>
#include <cmsys/Process.h>
#include <cmsys/ios/fstream>
#include <cmsys/ios/iostream>

#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

#define MaximumPathLength 1024

#define DebugError(x) \
  ofs << x << cmsys_ios::endl; \
  cmsys_ios::cout << x << cmsys_ios::endl

int main(int argc, char* argv[])
{
  //if ( cmsys::SystemTools::FileExists(
  cmsys_stl::string cwd = cmsys::SystemTools::GetCurrentWorkingDirectory();
  cmsys_ios::ofstream ofs("/tmp/output.txt");

  CFStringRef fileName;
  CFBundleRef appBundle;
  CFURLRef scriptFileURL;
  FSRef fileRef;
  FSSpec fileSpec;
  UInt8 *path;

  //get CF URL for script
  if (! (appBundle = CFBundleGetMainBundle()))
    {
    DebugError("Cannot get main bundle");
    return 1;
    }
  if (! (fileName = CFStringCreateWithCString(NULL, "RuntimeScript",
        kCFStringEncodingASCII)))
    {
    DebugError("CFStringCreateWithCString failed");
    return 1;
    }
  if (! (scriptFileURL = CFBundleCopyResourceURL(appBundle, fileName, NULL,
        NULL)))
    {
    DebugError("CFBundleCopyResourceURL failed");
    return 1;
    }

  //Get file reference from Core Foundation URL
  if (! CFURLGetFSRef(scriptFileURL, &fileRef))
    {
    DebugError("CFURLGetFSRef failed");
    return 1;
    }

  //dispose of the CF variables
  CFRelease(scriptFileURL);
  CFRelease(fileName);

  //convert FSRef to FSSpec
  if (FSGetCatalogInfo(&fileRef, kFSCatInfoNone, NULL, NULL, &fileSpec,
      NULL))
    {
    DebugError("FSGetCatalogInfo failed");
    return 1;
    }

  //create path string
  if (! (path = new UInt8[MaximumPathLength]))
    {
    return 1;
    }

  //create file reference from file spec
  OSErr err = FSpMakeFSRef(&fileSpec, &fileRef);
  if(err) 
    {
    return err;
    }

  // and then convert the FSRef to a path
  if ( FSRefMakePath(&fileRef, path, MaximumPathLength) )
    {
    DebugError("FSRefMakePath failed");
    return 1;
    }
  cmsys_stl::string fullScriptPath = reinterpret_cast<char*>(path);
  delete [] path;


  if (! cmsys::SystemTools::FileExists(fullScriptPath.c_str()))
    {
    return 1;
    }

  cmsys_stl::string scriptDirectory = cmsys::SystemTools::GetFilenamePath(
    fullScriptPath);
  ofs << fullScriptPath.c_str() << cmsys_ios::endl;
  cmsys_stl::vector<const char*> args;
  args.push_back(fullScriptPath.c_str());
  int cc;
  for ( cc = 1; cc < argc; ++ cc )
    {
    args.push_back(argv[cc]);
    }
  args.push_back(0);

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*args.begin());
  cmsysProcess_SetWorkingDirectory(cp, scriptDirectory.c_str());
  cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  cmsysProcess_SetTimeout(cp, 0);
  cmsysProcess_Execute(cp);
  
  std::vector<char> tempOutput;
  char* data;
  int length;
  while(cmsysProcess_WaitForData(cp, &data, &length, 0))
    {
    // Translate NULL characters in the output into valid text.
    // Visual Studio 7 puts these characters in the output of its
    // build process.
    for(int i=0; i < length; ++i)
      {
      if(data[i] == '\0')
        {
        data[i] = ' ';
        }
      }
    cmsys_ios::cout.write(data, length);
    }
  
  cmsysProcess_WaitForExit(cp, 0);
  
  bool result = true;
  if(cmsysProcess_GetState(cp) == cmsysProcess_State_Exited)
    {
    if ( cmsysProcess_GetExitValue(cp) !=  0 )
      {
      result = false;
      }
    }
  else if(cmsysProcess_GetState(cp) == cmsysProcess_State_Exception)
    {
    const char* exception_str = cmsysProcess_GetExceptionString(cp);
    std::cerr << exception_str << std::endl;
    result = false;
    }
  else if(cmsysProcess_GetState(cp) == cmsysProcess_State_Error)
    {
    const char* error_str = cmsysProcess_GetErrorString(cp);
    std::cerr << error_str << std::endl;
    result = false;
    }
  else if(cmsysProcess_GetState(cp) == cmsysProcess_State_Expired)
    {
    const char* error_str = "Process terminated due to timeout\n";
    std::cerr << error_str << std::endl;
    result = false;
    }
  
  cmsysProcess_Delete(cp);

  return 0;
}
