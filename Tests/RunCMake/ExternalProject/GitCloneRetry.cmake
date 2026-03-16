include(ExternalProject)

set(CMAKE_EP_GIT_CLONE_RETRY_COUNT 3)
set(CMAKE_EP_GIT_CLONE_RETRY_DELAY 1)

# The GIT_REPOSITORY path does not exist, so git clone will always fail,
# which is what triggers the retries.
ExternalProject_Add(TestGitRetry
  GIT_REPOSITORY "${CMAKE_CURRENT_BINARY_DIR}/TestGitRetry.git"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
