cmake_minimum_required(VERSION 3.19)
project(FetchTags LANGUAGES NONE)

include(ExternalProject)

# repoDir and gitTag are expected to be set as cache variables

ExternalProject_Add(fetcher
    GIT_REPOSITORY ${repoDir}
    GIT_TAG ${gitTag}
    GIT_REMOTE_UPDATE_STRATEGY CHECKOUT
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
