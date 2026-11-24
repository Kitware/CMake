cmake_minimum_required(VERSION 3.19)
project(FetchGitShallowHash LANGUAGES NONE)

include(ExternalProject)

# repoUrl and gitTag are expected to be set as cache variables

ExternalProject_Add(fetcher
    GIT_REPOSITORY ${repoUrl}
    GIT_TAG ${gitTag}
    GIT_SHALLOW TRUE
    GIT_REMOTE_UPDATE_STRATEGY CHECKOUT
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
