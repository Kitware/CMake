#include "kwsysPrivate.h"
#include KWSYS_HEADER(SystemInformation.hxx)
#include KWSYS_HEADER(ios/iostream)



// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "SystemInformation.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

#define printMethod(inof, m) kwsys_ios::cout << #m << ": " \
<< info.m() << "\n"
int testSystemInformation(int, char*[])
{
  kwsys::SystemInformation info;
  printMethod(info, GetVendorString);
  info.RunCPUCheck();
  info.RunOSCheck();
  info.RunMemoryCheck();
  printMethod(info, GetVendorString);
  printMethod(info, GetVendorID);
  printMethod(info, GetTypeID);
  printMethod(info, GetFamilyID);
  printMethod(info, GetModelID);
  printMethod(info, GetExtendedProcessorName);
  printMethod(info, GetProcessorSerialNumber);
  printMethod(info, GetProcessorCacheSize);
  printMethod(info, GetLogicalProcessorsPerPhysical);
  printMethod(info, GetProcessorClockFrequency);
  printMethod(info, GetProcessorAPICID);
  printMethod(info, GetOSName);
  printMethod(info, GetHostname);
  printMethod(info, GetOSRelease);
  printMethod(info, GetOSVersion);
  printMethod(info, GetOSPlatform);
  printMethod(info, Is64Bits);
  printMethod(info, GetNumberOfLogicalCPU);
  printMethod(info, GetNumberOfPhysicalCPU);
  printMethod(info, DoesCPUSupportCPUID);
  printMethod(info, GetTotalVirtualMemory);
  printMethod(info, GetAvailableVirtualMemory);
  printMethod(info, GetTotalPhysicalMemory);
  printMethod(info, GetAvailablePhysicalMemory);
  
  //int GetProcessorCacheXSize(long int);
//  bool DoesCPUSupportFeature(long int);
  return 0;
}
