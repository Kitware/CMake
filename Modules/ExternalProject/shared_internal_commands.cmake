cmake_policy(VERSION 3.25)

# Determine the remote URL of the project containing the working_directory.
# This will leave output_variable unset if the URL can't be determined.
function(_ep_get_git_remote_url output_variable working_directory)
  set("${output_variable}" "" PARENT_SCOPE)

  find_package(Git QUIET REQUIRED)

  execute_process(
    COMMAND ${GIT_EXECUTABLE} symbolic-ref --short HEAD
    WORKING_DIRECTORY "${working_directory}"
    OUTPUT_VARIABLE git_symbolic_ref
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )

  if(NOT git_symbolic_ref STREQUAL "")
    # We are potentially on a branch. See if that branch is associated with
    # an upstream remote (might be just a local one or not a branch at all).
    execute_process(
      COMMAND ${GIT_EXECUTABLE} config branch.${git_symbolic_ref}.remote
      WORKING_DIRECTORY "${working_directory}"
      OUTPUT_VARIABLE git_remote_name
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
  endif()

  if(NOT git_remote_name)
    # Can't select a remote based on a branch. If there's only one remote,
    # or we have multiple remotes but one is called "origin", choose that.
    execute_process(
      COMMAND ${GIT_EXECUTABLE} remote
      WORKING_DIRECTORY "${working_directory}"
      OUTPUT_VARIABLE git_remote_list
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    string(REPLACE "\n" ";" git_remote_list "${git_remote_list}")
    list(LENGTH git_remote_list git_remote_list_length)

    if(git_remote_list_length EQUAL 0)
      message(FATAL_ERROR "Git remote not found in parent project.")
    elseif(git_remote_list_length EQUAL 1)
      list(GET git_remote_list 0 git_remote_name)
    else()
      set(base_warning_msg "Multiple git remotes found for parent project")
      if("origin" IN_LIST git_remote_list)
        message(WARNING "${base_warning_msg}, defaulting to origin.")
        set(git_remote_name "origin")
      else()
        message(FATAL_ERROR "${base_warning_msg}, none of which are origin.")
      endif()
    endif()
  endif()

  if(GIT_VERSION VERSION_LESS 1.7.5)
    set(_git_remote_url_cmd_args config remote.${git_remote_name}.url)
  elseif(GIT_VERSION VERSION_LESS 2.7)
    set(_git_remote_url_cmd_args ls-remote --get-url ${git_remote_name})
  else()
    set(_git_remote_url_cmd_args remote get-url ${git_remote_name})
  endif()

  execute_process(
    COMMAND ${GIT_EXECUTABLE} ${_git_remote_url_cmd_args}
    WORKING_DIRECTORY "${working_directory}"
    OUTPUT_VARIABLE git_remote_url
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL LAST
    ENCODING UTF-8   # Needed to handle non-ascii characters in local paths
  )

  set("${output_variable}" "${git_remote_url}" PARENT_SCOPE)
endfunction()

function(_ep_is_relative_git_remote output_variable remote_url)
  if(remote_url MATCHES "^\\.\\./")
    set("${output_variable}" TRUE PARENT_SCOPE)
  else()
    set("${output_variable}" FALSE PARENT_SCOPE)
  endif()
endfunction()

# Return an absolute remote URL given an existing remote URL and relative path.
# The output_variable will be set to an empty string if an absolute URL
# could not be computed (no error message is output).
function(_ep_resolve_relative_git_remote
  output_variable
  parent_remote_url
  relative_remote_url
)
  set("${output_variable}" "" PARENT_SCOPE)

  if(parent_remote_url STREQUAL "")
    return()
  endif()

  string(REGEX MATCH
    "^(([A-Za-z0-9][A-Za-z0-9+.-]*)://)?(([^/@]+)@)?(\\[[A-Za-z0-9:]+\\]|[^/:]+)?([/:]/?)(.+(\\.git)?/?)$"
    git_remote_url_components
    "${parent_remote_url}"
  )

  set(protocol "${CMAKE_MATCH_1}")
  set(auth "${CMAKE_MATCH_3}")
  set(host "${CMAKE_MATCH_5}")
  set(separator "${CMAKE_MATCH_6}")
  set(path "${CMAKE_MATCH_7}")

  string(REPLACE "/" ";" remote_path_components "${path}")
  string(REPLACE "/" ";" relative_path_components "${relative_remote_url}")

  foreach(relative_path_component IN LISTS relative_path_components)
    if(NOT relative_path_component STREQUAL "..")
      break()
    endif()

    list(LENGTH remote_path_components remote_path_component_count)

    if(remote_path_component_count LESS 1)
      return()
    endif()

    list(POP_BACK remote_path_components)
    list(POP_FRONT relative_path_components)
  endforeach()

  list(APPEND final_path_components ${remote_path_components} ${relative_path_components})
  list(JOIN final_path_components "/" path)

  set("${output_variable}" "${protocol}${auth}${host}${separator}${path}" PARENT_SCOPE)
endfunction()

# The output_variable will be set to the original git_repository if it
# could not be resolved (no error message is output). The original value is
# also returned if it doesn't need to be resolved.
function(_ep_resolve_git_remote
  output_variable
  git_repository
  cmp0150
  cmp0150_old_base_dir
)
  if(git_repository STREQUAL "")
    set("${output_variable}" "" PARENT_SCOPE)
    return()
  endif()

  _ep_is_relative_git_remote(_git_repository_is_relative "${git_repository}")

  if(NOT _git_repository_is_relative)
    set("${output_variable}" "${git_repository}" PARENT_SCOPE)
    return()
  endif()

  if(cmp0150 STREQUAL "NEW")
    _ep_get_git_remote_url(_parent_git_remote_url "${CMAKE_CURRENT_SOURCE_DIR}")
    _ep_resolve_relative_git_remote(_resolved_git_remote_url "${_parent_git_remote_url}" "${git_repository}")

    if(_resolved_git_remote_url STREQUAL "")
      message(FATAL_ERROR
        "Failed to resolve relative git remote URL:\n"
        "  Relative URL: ${git_repository}\n"
        "  Parent URL:   ${_parent_git_remote_url}"
      )
    endif()
    set("${output_variable}" "${_resolved_git_remote_url}" PARENT_SCOPE)
    return()
  elseif(cmp0150 STREQUAL "")
    cmake_policy(GET_WARNING CMP0150 _cmp0150_warning)
    message(AUTHOR_WARNING
      "${_cmp0150_warning}\n"
      "A relative GIT_REPOSITORY path was detected. "
      "This will be interpreted as a local path to where the project is being cloned. "
      "Set GIT_REPOSITORY to an absolute path or set policy CMP0150 to NEW to avoid "
      "this warning."
    )
  endif()

  set("${output_variable}" "${cmp0150_old_base_dir}/${git_repository}" PARENT_SCOPE)
endfunction()
