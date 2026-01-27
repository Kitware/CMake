include(ExternalProject)

#
## Set environment variables on custom commands
#

# Comma list-separator
set(ScriptPath "${CMAKE_CURRENT_SOURCE_DIR}/EnvVars/EchoVar.cmake")
ExternalProject_Add(CustomCommand
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  LIST_SEPARATOR ,
  CONFIGURE_COMMAND ""
  COMMAND
  COMMAND "${CMAKE_COMMAND}" -DMYLIST='a,b,c' -P ${ScriptPath}
  COMMAND "${CMAKE_COMMAND}" -DVARNAME=Stage -P ${ScriptPath}
  COMMAND ""
  COMMAND COMMAND COMMAND
  COMMAND "${CMAKE_COMMAND}" -E echo "" ""
  COMMAND
  COMMAND "${CMAKE_COMMAND}" -DVARNAME=ListVar -P ${ScriptPath}
  CONFIGURE_ENVIRONMENT_MODIFICATION
    Stage=set:config
    ListVar=set:4,5,6
    ListSeparator=set:,
  BUILD_COMMAND "${CMAKE_COMMAND}" -P ${ScriptPath}
  BUILD_ENVIRONMENT_MODIFICATION
    Stage=set:build
    ListVar=set:4,5,6
    ListSeparator=set:,
  INSTALL_COMMAND "${CMAKE_COMMAND}" -P ${ScriptPath}
  INSTALL_ENVIRONMENT_MODIFICATION
    InstallVar=set:install
    Stage=set:install
    ListVar=set:4,5,6
    ListSeparator=set:,
  TEST_COMMAND "${CMAKE_COMMAND}" -P ${ScriptPath}
  TEST_ENVIRONMENT_MODIFICATION
    Stage=set:test
    ListVar=set:4,5,6
    ListSeparator=set:,)

ExternalProject_Add_Step(CustomCommand custom
  DEPENDERS configure
  COMMAND "${CMAKE_COMMAND}" -DVARNAME=CustomVar -P ${ScriptPath}
  COMMAND "${CMAKE_COMMAND}" -DVARNAME=CustomVar2 -P ${ScriptPath}
  COMMAND "${CMAKE_COMMAND}" -P ${ScriptPath}
  ENVIRONMENT_MODIFICATION
    CustomVar=set:custom
    CustomVar2=set:custom2
    Stage=set:custom
    ListVar=set:1,2,3
    ListSeparator=set:,)

#
## Set environment variables on the default commands
#

# No list separator
ExternalProject_Add(DefaultCommand
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/EnvVars"
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  DEPENDS CustomCommand
  CMAKE_ARGS
    -DVARIABLE=ConfigVar
  CONFIGURE_ENVIRONMENT_MODIFICATION
    ConfigVar=string_append:fig # RunCMakeTest.cmake sets to "con"
    ListVar=set:7,8,9
    ListSeparator=set:,
  BUILD_ENVIRONMENT_MODIFICATION
    Stage=set:build
    ListVar=set:7,8,9,10
    ListSeparator=set:,
  INSTALL_COMMAND ""                 # empty install command should not show up
  INSTALL_ENVIRONMENT_MODIFICATION
    Stage=set:install
    Separator=set:,)

ExternalProject_Add(DefaultCommandListSep
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/EnvVars"
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  DEPENDS DefaultCommand
  LIST_SEPARATOR `
  CMAKE_ARGS
    -DVARIABLE=ConfigVar
    -DMYLIST=d`e`f`g
  CONFIGURE_ENVIRONMENT_MODIFICATION
    ConfigVar=set:config
    ListVar=set:9`8`7
    ListSeparator=set:`
  BUILD_COMMAND ""
  INSTALL_COMMAND "")

# Using `:` as a list separator on Windows does not work as it replaces the `:`
# between the drive letter and the filepath with `;`.
if(NOT WIN32)
  # Ensure that using `:` as a list-separator does not break setting environment
  # variables
  ExternalProject_Add(DefaultCommandListColon
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/EnvVars"
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    DEPENDS DefaultCommandListSep
    LIST_SEPARATOR :
    CMAKE_ARGS
      -DVARIABLE=ConfigVar
    CONFIGURE_ENVIRONMENT_MODIFICATION
      ConfigVar=set:config
      ListVar=set:10:11:12
      ListSeparator=set::
    BUILD_ENVIRONMENT_MODIFICATION
      Stage=set:build
      ListVar=set:10:11:12
      ListSeparator=set::
    INSTALL_ENVIRONMENT_MODIFICATION
      Stage=set:install
      ListSeparator=set::
      ListVar=set:10:11:12:13)
endif()
