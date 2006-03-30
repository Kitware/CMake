#include <cmsys/Process.h>
#include "cmStandardIncludes.h"

int WaitForLine(cmsysProcess* process, std::string& line,
                double timeout,
                std::vector<char>& out,
                std::vector<char>& err)
{
  line = "";
  std::vector<char>::iterator outiter = out.begin();
  std::vector<char>::iterator erriter = err.begin();
  while(1)
    {
    // Check for a newline in stdout.
    for(;outiter != out.end(); ++outiter)
      {
      if((*outiter == '\r') && ((outiter+1) == out.end()))
        {
        break;
        }
      else if(*outiter == '\n' || *outiter == '\0')
        {
        int length = outiter-out.begin();
        if(length > 1 && *(outiter-1) == '\r')
          {
          --length;
          }
        if(length > 0)
          {
          line.append(&out[0], length);
          }
        out.erase(out.begin(), outiter+1);
        return cmsysProcess_Pipe_STDOUT;
        }
      }

    // Check for a newline in stderr.
    for(;erriter != err.end(); ++erriter)
      {
      if((*erriter == '\r') && ((erriter+1) == err.end()))
        {
        break;
        }
      else if(*erriter == '\n' || *erriter == '\0')
        {
        int length = erriter-err.begin();
        if(length > 1 && *(erriter-1) == '\r')
          {
          --length;
          }
        if(length > 0)
          {
          line.append(&err[0], length);
          }
        err.erase(err.begin(), erriter+1);
        return cmsysProcess_Pipe_STDERR;
        }
      }

    // No newlines found.  Wait for more data from the process.
    int length;
    char* data;
    int pipe = cmsysProcess_WaitForData(process, &data, &length, &timeout);
    if(pipe == cmsysProcess_Pipe_Timeout)
      {
      // Timeout has been exceeded.
      return pipe;
      }
    else if(pipe == cmsysProcess_Pipe_STDOUT)
      {
      // Append to the stdout buffer.
      std::vector<char>::size_type size = out.size();
      out.insert(out.end(), data, data+length);
      outiter = out.begin()+size;
      }
    else if(pipe == cmsysProcess_Pipe_STDERR)
      {
      // Append to the stderr buffer.
      std::vector<char>::size_type size = err.size();
      err.insert(err.end(), data, data+length);
      erriter = err.begin()+size;
      }
    else if(pipe == cmsysProcess_Pipe_None)
      {
      // Both stdout and stderr pipes have broken.  Return leftover data.
      if(!out.empty())
        {
        line.append(&out[0], outiter-out.begin());
        out.erase(out.begin(), out.end());
        return cmsysProcess_Pipe_STDOUT;
        }
      else if(!err.empty())
        {
        line.append(&err[0], erriter-err.begin());
        err.erase(err.begin(), err.end());
        return cmsysProcess_Pipe_STDERR;
        }
      else
        {
        return cmsysProcess_Pipe_None;
        }
      }
    }
}

int RunXCode(std::vector<const char*>& argv, bool& hitbug)
{
  hitbug = false;
  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  cmsysProcess_SetTimeout(cp, 0);
  cmsysProcess_Execute(cp);
  std::vector<char> out;
  std::vector<char> err;
  std::string line;
  int pipe =WaitForLine(cp, line, 0, out, err);
  while(pipe != cmsysProcess_Pipe_None)
    {
    if(line.find("/bin/sh: bad interpreter: Text file busy") 
       != line.npos)
      {
      hitbug = true;
      std::cerr << "Found xcodebuild bug: " << line << "\n";
      }
    // if the bug is hit, no more output should be generated
    // because it may contain bogus errors
    if(!hitbug)
      {
      if(pipe == cmsysProcess_Pipe_STDERR)
        {
        std::cerr << line;
        }
      else if(pipe == cmsysProcess_Pipe_STDOUT)
        {
        std::cout << line;
        }
      pipe =WaitForLine(cp, line, 0, out, err);
      }
    }
  cmsysProcess_WaitForExit(cp, 0);
  if(cmsysProcess_GetState(cp) == cmsysProcess_State_Exited)
    {
    return cmsysProcess_GetExitValue(cp);
    }
  if(cmsysProcess_GetState(cp) == cmsysProcess_State_Error)
    {
    std::cerr << "error\n";
    return -1;
    }
  return -1;
}

int main(int ac, char*av[])
{ 
  std::vector<const char*> argv;
  argv.push_back("xcodebuild");
  for(int i =1; i < ac; i++)
    {
    argv.push_back(av[i]);
    }
  argv.push_back(0);
  bool hitbug = true;
  int ret;
  while(hitbug)
    {
    ret = RunXCode(argv, hitbug);
    }
  std::cerr << "ret " << ret << "\n";
  if(ret < 0)
    {
    return 255;
    }
  return ret;
}

