/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <cmConfigure.h> // IWYU pragma: keep

#include <string>

#include <cm3p/uv.h>

#include "cmSystemTools.h"

#include "testCommon.h"

static bool test_uv_fs_mkdtemp()
{
  std::cout << "test_uv_fs_mkdtemp()\n";
  static std::string const kTemplate = "test-uv_fs_mkdtemp-XXXXXX";
  std::string tempDir;
  uv_fs_t tempDirReq;
  tempDirReq.data = &tempDir;
  uv_loop_t* loop = uv_default_loop();
  int r =
    uv_fs_mkdtemp(loop, &tempDirReq, kTemplate.c_str(), [](uv_fs_t* req) {
      if (req->data && req->path) {
        *static_cast<std::string*>(req->data) = req->path;
      }
    });
  ASSERT_EQUAL(r, 0);
  uv_run(loop, UV_RUN_DEFAULT);
  uv_fs_req_cleanup(&tempDirReq);
  uv_loop_close(loop);
  if (!cmSystemTools::FileIsDirectory(tempDir)) {
    std::cout << "cmSystemTools::MakeTempDirectory did not create \""
              << tempDir << '\n';
    return false;
  }
  cmSystemTools::RemoveADirectory(tempDir);
  ASSERT_TRUE(tempDir != kTemplate);
  return true;
}

int testUVPatches(int /*unused*/, char* /*unused*/[])
{
  return runTests({
    test_uv_fs_mkdtemp,
  });
}
