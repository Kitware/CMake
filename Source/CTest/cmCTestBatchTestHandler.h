#ifndef cmCTestBatchTestHandler_h
#define cmCTestBatchTestHandler_h

#include <cmStandardIncludes.h>
#include <cmCTestTestHandler.h>
#include <cmCTestMultiProcessHandler.h>
#include <cmCTestRunTest.h>

/** \class cmCTestBatchTestHandler
 * \brief run parallel ctest
 *
 * cmCTestBatchTestHandler 
 */
class cmCTestBatchTestHandler : public cmCTestMultiProcessHandler
{
public:
  ~cmCTestBatchTestHandler();
  virtual void RunTests();
protected:
  void WriteBatchScript();
  void WriteSrunArgs(int test, std::fstream& fout);
  void WriteTestCommand(int test, std::fstream& fout);

  void SubmitBatchScript();

  std::string Script;
};

#endif
