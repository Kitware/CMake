include(FetchContent)

macro(null_provider method)
  message(STATUS "Null provider called")
  message(STATUS "Provider invoked for method ${method} with args: ${ARGN}")
endmacro()

macro(find_package_provider method package_name)
  message(STATUS "Intercepted find_package(${package_name})")
  message(STATUS "Provider invoked for method ${method} with args: ${ARGN}")
  set(${package_name}_FOUND TRUE)
endmacro()

macro(FetchContentSerial_provider method dep_name)
  message(STATUS "Intercepted FetchContent_MakeAvailable(${dep_name})")
  message(STATUS "Provider invoked for method ${method} with args: ${ARGN}")
  FetchContent_SetPopulated(${dep_name})
endmacro()

macro(redirect_find_package_provider method package_name)
  message(STATUS "Redirecting find_package(${package_name}) to FetchContent_MakeAvailable()")
  message(STATUS "Provider invoked for method ${method} with args: ${ARGN}")
  FetchContent_Declare(${package_name}
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}
    SOURCE_SUBDIR DoesNotExist
  )
  FetchContent_MakeAvailable(${package_name})
  set(${package_name}_FOUND TRUE)
endmacro()

macro(redirect_FetchContentSerial_provider method dep_name)
  message(STATUS "Redirecting FetchContent_MakeAvailable(${dep_name}) to find_package()")
  message(STATUS "Provider invoked for method ${method} with args: ${ARGN}")
  find_package(${dep_name} NO_DEFAULT_PATH
    PATHS ${CMAKE_CURRENT_LIST_DIR}/Finders
    REQUIRED
  )
  FetchContent_SetPopulated(${dep_name})
endmacro()

macro(forward_find_package method package_name)
  message(STATUS "Forwarding find_package(${package_name})")
  message(STATUS "Provider invoked for method ${method} with args: ${ARGN}")
  find_package(${package_name}
    BYPASS_PROVIDER
    PATHS ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles
    ${ARGN}
  )
  message(STATUS "Leaving provider")
endmacro()

macro(skip_transitive_find_package method dep_name)
  message(STATUS "Intercepted find_package(${dep_name})")
  get_property(_ctx GLOBAL PROPERTY STFP_PROVIDER_CONTEXT)
  message(STATUS "Provider invoked in context '${_ctx}'")
  if("${_ctx}" STREQUAL "")
    message(STATUS "Forwarding to find_package(${dep_name})")
    set_property(GLOBAL PROPERTY STFP_PROVIDER_CONTEXT "${dep_name}")
    find_package(${dep_name} ${ARGN})
    set_property(GLOBAL PROPERTY STFP_PROVIDER_CONTEXT "")
  elseif(NOT "${_ctx}" STREQUAL "${dep_name}")
    message(STATUS "Marking ${dep_name} as FOUND")
    set(${dep_name}_FOUND TRUE)
  endif()
  message(STATUS "Leaving provider")
endmacro()

macro(recurse_FetchContent method dep_name)
  message(STATUS "Intercepted FetchContent_MakeAvailable(${dep_name})")
  message(STATUS "Provider invoked for method ${method} with args: ${ARGN}")
  FetchContent_MakeAvailable(${dep_name})
  message(STATUS "Should now be handled")
endmacro()

message(STATUS "Before cmake_language")
cmake_language(
  SET_DEPENDENCY_PROVIDER ${provider_command}
  SUPPORTED_METHODS ${provider_methods}
)
message(STATUS "After cmake_language")
