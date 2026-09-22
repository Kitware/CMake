# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Resolve the Maven dependencies listed in CPACK_APK_MAVEN_DEPENDENCIES the
# way Gradle resolves the runtime classpath of an Android application, and
# download them.  Included by CPackAPK.cmake.
#
# Gradle module metadata (`.module`) is used when published: the runtime
# variant for Android is selected and `available-at` redirections, which
# Kotlin Multiplatform libraries use, are followed.  Otherwise the POM is
# used.  The highest version requested wins, in the order of Gradle, only
# the modules at their selected versions requesting versions, and
# dependency constraints, including those of platforms, can raise versions
# or give them to dependencies that have none.  Exclusions apply below a
# module when they apply on every path to it.

if(NOT DEFINED CPACK_APK_MAVEN_REPOSITORIES)
  set(CPACK_APK_MAVEN_REPOSITORIES
    https://maven.google.com
    https://repo.maven.apache.org/maven2
    )
endif()
set(_maven_repositories)
# Relative paths are relative to the directory cpack runs in, like those of
# the other variables.
file(REAL_PATH "." _cpack_directory)
foreach(_repository IN LISTS CPACK_APK_MAVEN_REPOSITORIES)
  # A local directory is a repository as well.  It need not exist.
  if(NOT _repository MATCHES "^[A-Za-z][A-Za-z0-9+.-]*://")
    cmake_path(ABSOLUTE_PATH _repository
      BASE_DIRECTORY "${_cpack_directory}" NORMALIZE)
    if(NOT _repository MATCHES "^/")
      string(PREPEND _repository "/")
    endif()
    string(PREPEND _repository "file://")
  endif()
  string(REGEX REPLACE "/+$" "" _repository "${_repository}")
  list(APPEND _maven_repositories "${_repository}")
endforeach()

if(NOT CPACK_APK_MAVEN_CACHE_DIRECTORY)
  # Next to the top level directory, which is removed after packaging.
  set(CPACK_APK_MAVEN_CACHE_DIRECTORY
    "${CPACK_PACKAGE_DIRECTORY}/_CPack_Packages/maven")
endif()

# Download `url` to `file` unless it exists.  Sets `result` to TRUE on
# success.  An optional `<algorithm>=<hash>` argument is verified.
function(_cpack_apk_maven_download url file result)
  if(EXISTS "${file}")
    set(${result} TRUE PARENT_SCOPE)
    return()
  endif()
  file(DOWNLOAD "${url}" "${file}.part" STATUS _status)
  list(GET _status 0 _code)
  if(NOT _code EQUAL 0)
    file(REMOVE "${file}.part")
    set(${result} FALSE PARENT_SCOPE)
    return()
  endif()
  if(ARGV3 MATCHES "^([A-Z0-9]+)=(.+)$")
    set(_expected "${CMAKE_MATCH_2}")
    file(${CMAKE_MATCH_1} "${file}.part" _actual)
    if(NOT _actual STREQUAL _expected)
      file(REMOVE "${file}.part")
      message(FATAL_ERROR
        "CPack APK: The file downloaded from\n"
        "  ${url}\n"
        "does not have the ${CMAKE_MATCH_1} hash the repository publishes.\n"
        "  Expected: ${_expected}\n"
        "  Actual:   ${_actual}")
    endif()
  endif()
  file(RENAME "${file}.part" "${file}")
  # Repositories are tried in turn, so only the successful download is
  # reported.
  message(STATUS "CPack APK: Downloaded ${url}")
  set(${result} TRUE PARENT_SCOPE)
endfunction()

# Download `name` of `group:artifact:version` from the first repository
# that has it, into the cache.  Sets `result` to the local path, or to an
# empty string when no repository has it.
function(_cpack_apk_maven_fetch group artifact version name result)
  string(REPLACE "." "/" _group_path "${group}")
  set(_relative "${_group_path}/${artifact}/${version}/${name}")
  set(_file "${CPACK_APK_MAVEN_CACHE_DIRECTORY}/${_relative}")
  foreach(_repository IN LISTS _maven_repositories)
    _cpack_apk_maven_download("${_repository}/${_relative}" "${_file}" _ok
      ${ARGN})
    if(_ok)
      set(${result} "${_file}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${result} "" PARENT_SCOPE)
endfunction()

# Split `group:artifact:version` into the variables named by the other
# arguments.
function(_cpack_apk_maven_split coordinates group_var artifact_var version_var)
  string(REPLACE ":" ";" _parts "${coordinates}")
  list(LENGTH _parts _length)
  if(NOT _length EQUAL 3)
    message(FATAL_ERROR
      "CPack APK: '${coordinates}' is not of the form "
      "<group>:<artifact>:<version>.")
  endif()
  list(GET _parts 0 _group)
  list(GET _parts 1 _artifact)
  list(GET _parts 2 _version)
  set(${group_var} "${_group}" PARENT_SCOPE)
  set(${artifact_var} "${_artifact}" PARENT_SCOPE)
  set(${version_var} "${_version}" PARENT_SCOPE)
endfunction()

# Strip the brackets of a hard version requirement, `[1.2.3]` -> `1.2.3`.
function(_cpack_apk_maven_plain_version version context result)
  string(REGEX REPLACE "^\\[([^,]*)\\]$" "\\1" _version "${version}")
  if(_version MATCHES "[][(),]")
    message(FATAL_ERROR
      "CPack APK: ${context} requires the version range '${version}'.  "
      "Version ranges are not supported.")
  endif()
  set(${result} "${_version}" PARENT_SCOPE)
endfunction()

# Split `version` into the parts Gradle compares: separated by `.`, `-`,
# `_` and `+`, and where digits and other characters meet.
function(_cpack_apk_maven_version_parts version result)
  string(REGEX REPLACE "[-_+]" "." _version "${version}")
  string(REGEX REPLACE "([0-9])([^0-9.])" "\\1.\\2" _version "${_version}")
  string(REGEX REPLACE "([^0-9.])([0-9])" "\\1.\\2" _version "${_version}")
  string(REGEX REPLACE "\\.\\.+" "." _version "${_version}")
  string(REGEX REPLACE "^\\.|\\.$" "" _version "${_version}")
  string(REPLACE "." ";" _version "${_version}")
  set(${result} "${_version}" PARENT_SCOPE)
endfunction()

# Rank of the version part `part`: `dev` is below the other qualifiers,
# `rc`, `snapshot`, `final`, `ga`, `release` and `sp` are above them, in
# this order, and numbers are above all qualifiers.
function(_cpack_apk_maven_version_rank part result)
  if(part MATCHES "^[0-9]+$")
    set(${result} 8 PARENT_SCOPE)
    return()
  endif()
  string(TOLOWER "${part}" _part)
  list(FIND _cpack_apk_maven_qualifiers "${_part}" _rank)
  if(_rank EQUAL -1)
    set(_rank 1)
  endif()
  set(${result} ${_rank} PARENT_SCOPE)
endfunction()
set(_cpack_apk_maven_qualifiers dev _other_ rc snapshot final ga release sp)

# Set `result` to 1, 0 or -1 as the version `a` is higher than, equal to or
# lower than `b`, the way Gradle orders versions.  `1.0-alpha` is lower
# than `1.0`, which is lower than `1.0.1`.
function(_cpack_apk_maven_version_compare a b result)
  _cpack_apk_maven_version_parts("${a}" _a)
  _cpack_apk_maven_version_parts("${b}" _b)
  list(LENGTH _a _a_length)
  list(LENGTH _b _b_length)
  set(_length ${_a_length})
  if(_b_length LESS _length)
    set(_length ${_b_length})
  endif()
  set(_i 0)
  while(_i LESS _length)
    list(GET _a ${_i} _a_part)
    list(GET _b ${_i} _b_part)
    _cpack_apk_maven_version_rank("${_a_part}" _a_rank)
    _cpack_apk_maven_version_rank("${_b_part}" _b_rank)
    if(_a_rank GREATER _b_rank)
      set(${result} 1 PARENT_SCOPE)
      return()
    elseif(_a_rank LESS _b_rank)
      set(${result} -1 PARENT_SCOPE)
      return()
    elseif(_a_rank EQUAL 8)
      if(_a_part GREATER _b_part)
        set(${result} 1 PARENT_SCOPE)
        return()
      elseif(_a_part LESS _b_part)
        set(${result} -1 PARENT_SCOPE)
        return()
      endif()
    elseif(_a_rank EQUAL 1)
      if(_a_part STRGREATER _b_part)
        set(${result} 1 PARENT_SCOPE)
        return()
      elseif(_a_part STRLESS _b_part)
        set(${result} -1 PARENT_SCOPE)
        return()
      endif()
    endif()
    math(EXPR _i "${_i} + 1")
  endwhile()
  # The longer version is higher if its next part is a number, and lower
  # otherwise.
  if(_a_length EQUAL _b_length)
    set(${result} 0 PARENT_SCOPE)
    return()
  elseif(_a_length GREATER _b_length)
    list(GET _a ${_length} _part)
    set(_sign 1)
  else()
    list(GET _b ${_length} _part)
    set(_sign -1)
  endif()
  if(NOT _part MATCHES "^[0-9]+$")
    math(EXPR _sign "-${_sign}")
  endif()
  set(${result} ${_sign} PARENT_SCOPE)
endfunction()

# Remove from `xml` the text from each `open` to the following `close`.
function(_cpack_apk_maven_xml_remove xml open close result)
  string(LENGTH "${close}" _close_length)
  while(TRUE)
    string(FIND "${xml}" "${open}" _begin)
    if(_begin EQUAL -1)
      break()
    endif()
    string(SUBSTRING "${xml}" ${_begin} -1 _tail)
    string(FIND "${_tail}" "${close}" _end)
    if(_end EQUAL -1)
      break()
    endif()
    math(EXPR _end "${_end} + ${_close_length}")
    string(SUBSTRING "${xml}" 0 ${_begin} _head)
    string(SUBSTRING "${_tail}" ${_end} -1 _tail)
    set(xml "${_head}${_tail}")
  endwhile()
  set(${result} "${xml}" PARENT_SCOPE)
endfunction()

# Set `result` to the content of the first element `tag` of `xml`, or to
# an empty string.
function(_cpack_apk_maven_xml_element xml tag result)
  set(_content "")
  string(FIND "${xml}" "<${tag}>" _begin)
  if(_begin GREATER -1)
    string(LENGTH "<${tag}>" _length)
    math(EXPR _begin "${_begin} + ${_length}")
    string(SUBSTRING "${xml}" ${_begin} -1 _content)
    string(FIND "${_content}" "</${tag}>" _end)
    if(_end GREATER -1)
      string(SUBSTRING "${_content}" 0 ${_end} _content)
    endif()
  endif()
  set(${result} "${_content}" PARENT_SCOPE)
endfunction()

# Set `result` to TRUE if `group:artifact` matches one of the `excludes`,
# `group:artifact` patterns in which either part may be `*`.
function(_cpack_apk_maven_excluded group artifact excludes result)
  foreach(_exclude IN LISTS excludes)
    if(NOT _exclude MATCHES "^([^:]*):(.*)$")
      continue()
    endif()
    set(_group "${CMAKE_MATCH_1}")
    set(_artifact "${CMAKE_MATCH_2}")
    if((_group STREQUAL "*" OR _group STREQUAL group)
        AND (_artifact STREQUAL "*" OR _artifact STREQUAL artifact))
      set(${result} TRUE PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${result} FALSE PARENT_SCOPE)
endfunction()

# Index of the variant of the module metadata `json` that Gradle picks for
# the runtime classpath of an Android application, or an empty string.
# `kind` is `library`, or `platform` for a platform, which only has
# dependency constraints.
function(_cpack_apk_maven_variant json kind result)
  set(_best "")
  set(_best_score 0)
  string(JSON _count LENGTH "${json}" variants)
  if(_count EQUAL 0)
    set(${result} "" PARENT_SCOPE)
    return()
  endif()
  math(EXPR _last "${_count} - 1")
  foreach(_i RANGE ${_last})
    foreach(_attribute IN ITEMS usage category platform environment build_type)
      set(_${_attribute} "")
    endforeach()
    foreach(_pair IN ITEMS
        usage=org.gradle.usage
        category=org.gradle.category
        platform=org.jetbrains.kotlin.platform.type
        environment=org.gradle.jvm.environment
        build_type=com.android.build.api.attributes.BuildTypeAttr)
      string(REPLACE "=" ";" _pair "${_pair}")
      list(GET _pair 0 _attribute)
      list(GET _pair 1 _name)
      string(JSON _value ERROR_VARIABLE _error
        GET "${json}" variants ${_i} attributes ${_name})
      if(NOT _error)
        set(_${_attribute} "${_value}")
      endif()
    endforeach()
    if(NOT _usage STREQUAL "java-runtime")
      continue()
    endif()
    if(kind STREQUAL "platform")
      if(NOT _category MATCHES "^(enforced-)?platform$")
        continue()
      endif()
    elseif(_category AND NOT _category STREQUAL "library")
      continue()
    endif()
    set(_score 2)
    if(_platform STREQUAL "androidJvm")
      math(EXPR _score "${_score} + 8")
    elseif(_platform STREQUAL "jvm")
      math(EXPR _score "${_score} + 4")
    elseif(_platform)
      # Kotlin/Native, JavaScript, WebAssembly or common metadata.
      continue()
    endif()
    if(_environment STREQUAL "android")
      math(EXPR _score "${_score} + 2")
    endif()
    # Libraries publishing all their Android build types have a debug and a
    # release variant; the application is packaged for release.
    if(_build_type STREQUAL "" OR _build_type STREQUAL "release")
      math(EXPR _score "${_score} + 1")
    endif()
    if(_score GREATER _best_score)
      set(_best ${_i})
      set(_best_score ${_score})
    endif()
  endforeach()
  set(${result} "${_best}" PARENT_SCOPE)
endfunction()

# Read the Gradle module metadata `json` of `coordinates`, as a `kind`.
function(_cpack_apk_maven_module json kind coordinates)
  _cpack_apk_maven_variant("${json}" ${kind} _variant)
  if(_variant STREQUAL "")
    if(kind STREQUAL "platform")
      set(_what "platform variant")
    else()
      set(_what "variant")
    endif()
    message(FATAL_ERROR
      "CPack APK: The Maven module '${coordinates}' has no ${_what} for the "
      "Java runtime.")
  endif()
  set(_deps)
  set(_constraints)
  set(_redirect)
  set(_files)
  string(JSON _available ERROR_VARIABLE _error
    GET "${json}" variants ${_variant} available-at)
  if(NOT _error)
    string(JSON _g GET "${_available}" group)
    string(JSON _m GET "${_available}" module)
    string(JSON _v GET "${_available}" version)
    set(_redirect "${_g}:${_m}:${_v}")
  else()
    foreach(_kind IN ITEMS dependencies dependencyConstraints)
      string(JSON _count ERROR_VARIABLE _error
        LENGTH "${json}" variants ${_variant} ${_kind})
      if(_error OR _count EQUAL 0)
        continue()
      endif()
      math(EXPR _last "${_count} - 1")
      foreach(_i RANGE ${_last})
        string(JSON _dep GET "${json}" variants ${_variant} ${_kind} ${_i})
        string(JSON _g GET "${_dep}" group)
        string(JSON _m GET "${_dep}" module)
        set(_v)
        foreach(_key IN ITEMS strictly requires prefers)
          string(JSON _v ERROR_VARIABLE _error GET "${_dep}" version ${_key})
          if(NOT _error)
            break()
          endif()
          set(_v)
        endforeach()
        if(_v)
          _cpack_apk_maven_plain_version("${_v}"
            "The Maven module '${coordinates}'" _v)
        endif()
        if(_kind STREQUAL "dependencyConstraints")
          # A constraint without a version, which only rejects versions,
          # does not raise one.
          if(_v)
            list(APPEND _constraints "${_g}:${_m}:${_v}")
          endif()
          continue()
        endif()
        # A platform has no artifacts, but its constraints give versions,
        # including to dependencies that have none.
        set(_dep_kind library)
        string(JSON _category ERROR_VARIABLE _error
          GET "${_dep}" attributes org.gradle.category)
        if(NOT _error AND _category MATCHES "platform")
          set(_dep_kind platform)
        endif()
        set(_excludes)
        string(JSON _excludes_count ERROR_VARIABLE _error
          LENGTH "${_dep}" excludes)
        if(NOT _error AND _excludes_count GREATER 0)
          math(EXPR _excludes_last "${_excludes_count} - 1")
          foreach(_j RANGE ${_excludes_last})
            foreach(_field IN ITEMS group module)
              string(JSON _${_field} ERROR_VARIABLE _error
                GET "${_dep}" excludes ${_j} ${_field})
              if(_error)
                set(_${_field} "*")
              endif()
            endforeach()
            list(APPEND _excludes "${_group}:${_module}")
          endforeach()
        endif()
        list(JOIN _excludes "," _excludes)
        list(APPEND _deps "${_dep_kind}|${_g}:${_m}:${_v}|${_excludes}")
      endforeach()
    endforeach()
    string(JSON _count ERROR_VARIABLE _error
      LENGTH "${json}" variants ${_variant} files)
    if(NOT _error AND _count GREATER 0)
      math(EXPR _last "${_count} - 1")
      foreach(_i RANGE ${_last})
        string(JSON _url GET "${json}" variants ${_variant} files ${_i} url)
        string(JSON _sha256 GET "${json}" variants ${_variant} files ${_i} sha256)
        string(TOLOWER "${_sha256}" _sha256)
        list(APPEND _files "${_url}|SHA256=${_sha256}")
      endforeach()
    endif()
  endif()
  set(_maven_deps "${_deps}" PARENT_SCOPE)
  set(_maven_constraints "${_constraints}" PARENT_SCOPE)
  set(_maven_redirect "${_redirect}" PARENT_SCOPE)
  set(_maven_files "${_files}" PARENT_SCOPE)
endfunction()

# The `<dependency>` elements of `xml`, as
# `group|artifact|version|scope|optional|excludes`, where `excludes` are
# the `group:artifact` of its exclusions, joined by `,`.  The `[` and `]`
# of version ranges would keep lists from being split, so they are given
# as `<lb>` and `<rb>`, which the elements cannot hold.
function(_cpack_apk_maven_pom_dependencies xml result)
  set(_space "[ \t\r\n]*")
  set(_deps)
  string(LENGTH "<dependency>" _open_length)
  string(LENGTH "<exclusion>" _exclusion_open_length)
  while(TRUE)
    string(FIND "${xml}" "<dependency>" _begin)
    if(_begin EQUAL -1)
      break()
    endif()
    math(EXPR _begin "${_begin} + ${_open_length}")
    string(SUBSTRING "${xml}" ${_begin} -1 xml)
    string(FIND "${xml}" "</dependency>" _end)
    string(SUBSTRING "${xml}" 0 ${_end} _chunk)
    # The transitive dependencies to leave out.
    _cpack_apk_maven_xml_element("${_chunk}" exclusions _exclusions)
    _cpack_apk_maven_xml_remove("${_chunk}" "<exclusions>" "</exclusions>"
      _chunk)
    set(_excludes)
    while(TRUE)
      string(FIND "${_exclusions}" "<exclusion>" _begin)
      if(_begin EQUAL -1)
        break()
      endif()
      math(EXPR _begin "${_begin} + ${_exclusion_open_length}")
      string(SUBSTRING "${_exclusions}" ${_begin} -1 _exclusions)
      string(FIND "${_exclusions}" "</exclusion>" _end)
      string(SUBSTRING "${_exclusions}" 0 ${_end} _exclusion)
      foreach(_tag IN ITEMS groupId artifactId)
        set(_${_tag} "*")
        if(_exclusion MATCHES "<${_tag}>${_space}([^< \t\r\n]+)${_space}</${_tag}>")
          set(_${_tag} "${CMAKE_MATCH_1}")
        endif()
      endforeach()
      string(APPEND _excludes ",${_groupId}:${_artifactId}")
    endwhile()
    string(REGEX REPLACE "^," "" _excludes "${_excludes}")
    foreach(_tag IN ITEMS groupId artifactId version scope optional)
      set(_${_tag})
      if(_chunk MATCHES "<${_tag}>${_space}([^< \t\r\n]*)${_space}</${_tag}>")
        set(_${_tag} "${CMAKE_MATCH_1}")
      endif()
    endforeach()
    set(_entry
      "${_groupId}|${_artifactId}|${_version}|${_scope}|${_optional}|${_excludes}")
    string(REPLACE "[" "<lb>" _entry "${_entry}")
    string(REPLACE "]" "<rb>" _entry "${_entry}")
    list(APPEND _deps "${_entry}")
  endwhile()
  set(${result} "${_deps}" PARENT_SCOPE)
endfunction()

# Set `result` to the version `value` given by
# `_cpack_apk_maven_pom_dependencies`, with its `[` and `]` restored.
function(_cpack_apk_maven_pom_dependency_version value result)
  string(REPLACE "<lb>" "[" value "${value}")
  string(REPLACE "<rb>" "]" value "${value}")
  set(${result} "${value}" PARENT_SCOPE)
endfunction()

# Set `result` to `value`, the `what` of `dependency` in the POM of
# `group:artifact:version`, whose `<properties>` element holds `properties`,
# with the properties it refers to replaced.
function(_cpack_apk_maven_pom_property value properties group artifact version
    dependency what result)
  set(_space "[ \t\r\n]*")
  # Properties of this POM; those of parent POMs are not known.
  while(value MATCHES "\\$\\{([^}]*)\\}")
    set(_property "${CMAKE_MATCH_1}")
    if(_property MATCHES "^((project|pom)\\.)?groupId$")
      set(_value "${group}")
    elseif(_property MATCHES "^((project|pom)\\.)?artifactId$")
      set(_value "${artifact}")
    elseif(_property MATCHES "^((project|pom)\\.)?version$")
      set(_value "${version}")
    elseif(properties MATCHES "<${_property}>${_space}([^<]*[^< \t\r\n])${_space}</${_property}>")
      set(_value "${CMAKE_MATCH_1}")
    else()
      message(FATAL_ERROR
        "CPack APK: The ${what} of '${dependency}' in the POM of "
        "'${group}:${artifact}:${version}' refers to the unknown property "
        "'${_property}'.  Parent POMs are not supported.")
    endif()
    string(REPLACE "\${${_property}}" "${_value}" value "${value}")
  endwhile()
  set(${result} "${value}" PARENT_SCOPE)
endfunction()

# Set `result` to the version `value` of `dependency` in the POM of
# `group:artifact:version`, whose `<properties>` element holds
# `properties`.
function(_cpack_apk_maven_pom_version value properties group artifact version
    dependency result)
  _cpack_apk_maven_pom_property("${value}" "${properties}"
    ${group} ${artifact} ${version} "${dependency}" version value)
  if(NOT value STREQUAL "")
    _cpack_apk_maven_plain_version("${value}"
      "The POM of '${group}:${artifact}:${version}'" value)
  endif()
  set(${result} "${value}" PARENT_SCOPE)
endfunction()

# Replace the properties that the group and artifact held by the variables
# named `group_var` and `artifact_var` refer to, in the POM of
# `group:artifact:version`, whose `<properties>` element holds `properties`.
# Modules often name the group of their siblings `${project.groupId}`.
function(_cpack_apk_maven_pom_coordinates group_var artifact_var properties
    group artifact version)
  set(_dependency "${${group_var}}:${${artifact_var}}")
  _cpack_apk_maven_pom_property("${${group_var}}" "${properties}"
    ${group} ${artifact} ${version} "${_dependency}" group _group)
  _cpack_apk_maven_pom_property("${${artifact_var}}" "${properties}"
    ${group} ${artifact} ${version} "${_dependency}" artifact _artifact)
  set(${group_var} "${_group}" PARENT_SCOPE)
  set(${artifact_var} "${_artifact}" PARENT_SCOPE)
endfunction()

# Read the POM `pom` of `group:artifact:version`, as a `kind`.  A platform
# has the versions the POM manages as dependency constraints, and the
# platforms it imports as dependencies.
function(_cpack_apk_maven_pom pom kind group artifact version)
  file(READ "${pom}" _content)
  string(REPLACE ";" "," _content "${_content}")
  # Only the elements of the project itself count, not those in comments,
  # profiles or the build, such as the dependencies of plugins.
  _cpack_apk_maven_xml_remove("${_content}" "<!--" "-->" _content)
  foreach(_tag IN ITEMS profiles build reporting)
    _cpack_apk_maven_xml_remove("${_content}" "<${_tag}>" "</${_tag}>"
      _content)
  endforeach()
  _cpack_apk_maven_xml_element("${_content}" properties _properties)
  _cpack_apk_maven_xml_element("${_content}" dependencyManagement _managed)
  _cpack_apk_maven_xml_remove("${_content}"
    "<dependencyManagement>" "</dependencyManagement>" _content)

  set(_deps)
  set(_constraints)
  _cpack_apk_maven_pom_dependencies("${_managed}" _managed)
  foreach(_entry IN LISTS _managed)
    string(REPLACE "|" ";" _fields "${_entry}")
    list(GET _fields 0 _g)
    list(GET _fields 1 _a)
    list(GET _fields 2 _v)
    list(GET _fields 3 _scope)
    _cpack_apk_maven_pom_dependency_version("${_v}" _v)
    _cpack_apk_maven_pom_coordinates(_g _a "${_properties}"
      ${group} ${artifact} ${version})
    if(kind STREQUAL "platform")
      _cpack_apk_maven_pom_version("${_v}" "${_properties}"
        ${group} ${artifact} ${version} "${_g}:${_a}" _v)
      if(_v STREQUAL "")
        continue()
      endif()
      if(_scope STREQUAL "import")
        list(APPEND _deps "platform|${_g}:${_a}:${_v}|")
      else()
        list(APPEND _constraints "${_g}:${_a}:${_v}")
      endif()
    elseif(NOT _scope STREQUAL "import")
      # The versions of the dependencies that do not give one.
      string(MAKE_C_IDENTIFIER "${_g}:${_a}" _id)
      set(_managed_${_id} "${_v}")
    endif()
  endforeach()

  set(_files)
  if(kind STREQUAL "library")
    _cpack_apk_maven_pom_dependencies("${_content}" _dependencies)
    foreach(_entry IN LISTS _dependencies)
      string(REPLACE "|" ";" _fields "${_entry}")
      list(GET _fields 0 _g)
      list(GET _fields 1 _a)
      list(GET _fields 2 _v)
      list(GET _fields 3 _scope)
      list(GET _fields 4 _optional)
      list(GET _fields 5 _excludes)
      _cpack_apk_maven_pom_dependency_version("${_v}" _v)
      _cpack_apk_maven_pom_coordinates(_g _a "${_properties}"
        ${group} ${artifact} ${version})
      string(REPLACE "," ";" _excludes "${_excludes}")
      set(_resolved_excludes)
      foreach(_exclude IN LISTS _excludes)
        string(REPLACE ":" ";" _exclude "${_exclude}")
        list(GET _exclude 0 _eg)
        list(GET _exclude 1 _ea)
        _cpack_apk_maven_pom_coordinates(_eg _ea "${_properties}"
          ${group} ${artifact} ${version})
        list(APPEND _resolved_excludes "${_eg}:${_ea}")
      endforeach()
      list(JOIN _resolved_excludes "," _excludes)
      if(_scope AND NOT _scope MATCHES "^(compile|runtime)$")
        continue()
      endif()
      if(_optional STREQUAL "true")
        continue()
      endif()
      if(_v STREQUAL "")
        string(MAKE_C_IDENTIFIER "${_g}:${_a}" _id)
        set(_v "${_managed_${_id}}")
      endif()
      _cpack_apk_maven_pom_version("${_v}" "${_properties}"
        ${group} ${artifact} ${version} "${_g}:${_a}" _v)
      if(_v STREQUAL "")
        message(FATAL_ERROR
          "CPack APK: The POM of '${group}:${artifact}:${version}' does not "
          "give the version of '${_g}:${_a}'.  Parent POMs and imported "
          "BOMs are not supported.")
      endif()
      list(APPEND _deps "library|${_g}:${_a}:${_v}|${_excludes}")
    endforeach()

    set(_space "[ \t\r\n]*")
    set(_packaging jar)
    if(_content MATCHES "<packaging>${_space}([^< \t\r\n]*)")
      set(_packaging "${CMAKE_MATCH_1}")
    endif()
    if(_packaging STREQUAL "bundle")
      set(_packaging jar)
    endif()
    if(NOT _packaging STREQUAL "pom")
      set(_name "${artifact}-${version}.${_packaging}")
      _cpack_apk_maven_fetch(${group} ${artifact} ${version} "${_name}.sha1"
        _sha1_file)
      if(_sha1_file)
        file(STRINGS "${_sha1_file}" _sha1 LIMIT_COUNT 1 REGEX "^[0-9a-fA-F]+")
        string(REGEX MATCH "^[0-9a-fA-F]+" _sha1 "${_sha1}")
        string(TOLOWER "${_sha1}" _sha1)
        list(APPEND _files "${_name}|SHA1=${_sha1}")
      else()
        list(APPEND _files "${_name}|")
      endif()
    endif()
  endif()
  set(_maven_deps "${_deps}" PARENT_SCOPE)
  set(_maven_constraints "${_constraints}" PARENT_SCOPE)
  set(_maven_redirect "" PARENT_SCOPE)
  set(_maven_files "${_files}" PARENT_SCOPE)
endfunction()

# Read the metadata of `group:artifact:version` as a `kind`, `library` or
# `platform`, and set in the caller:
#   _maven_deps         its dependencies, `kind|group:artifact:version|excludes`,
#                       where the version may be empty and `excludes` are
#                       `group:artifact` patterns joined by `,`
#   _maven_constraints  its dependency constraints, `group:artifact:version`
#   _maven_redirect     the `group:artifact:version` it moved to, if any
#   _maven_files        its files, `name|algorithm=hash`
function(_cpack_apk_maven_metadata kind group artifact version)
  set(_coordinates "${group}:${artifact}:${version}")
  _cpack_apk_maven_fetch(${group} ${artifact} ${version}
    "${artifact}-${version}.module" _module)
  if(_module)
    file(READ "${_module}" _json)
    _cpack_apk_maven_module("${_json}" ${kind} "${_coordinates}")
  else()
    _cpack_apk_maven_fetch(${group} ${artifact} ${version}
      "${artifact}-${version}.pom" _pom)
    if(NOT _pom)
      list(JOIN _maven_repositories ", " _repositories)
      message(FATAL_ERROR
        "CPack APK: The Maven artifact '${_coordinates}' was not found in "
        "any of: ${_repositories}")
    endif()
    _cpack_apk_maven_pom("${_pom}" ${kind} ${group} ${artifact} ${version})
  endif()
  foreach(_field IN ITEMS deps constraints redirect files)
    set(_maven_${_field} "${_maven_${_field}}" PARENT_SCOPE)
  endforeach()
endfunction()

# Set `result` to TRUE if the version `a` is higher than `b`, or `b` is
# empty.
function(_cpack_apk_maven_version_higher a b result)
  set(_higher TRUE)
  if(NOT b STREQUAL "")
    _cpack_apk_maven_version_compare("${a}" "${b}" _order)
    if(NOT _order EQUAL 1)
      set(_higher FALSE)
    endif()
  endif()
  set(${result} ${_higher} PARENT_SCOPE)
endfunction()

# Resolve CPACK_APK_MAVEN_DEPENDENCIES and set `result` to the downloaded
# archives.
function(_cpack_apk_maven_resolve result)
  list(JOIN CPACK_APK_MAVEN_DEPENDENCIES ", " _dependencies)
  message(STATUS "CPack APK: Resolving Maven dependencies: ${_dependencies}")
  # The graph is walked again whenever the version of a module, or the
  # exclusions that apply below it, change after it was visited, until
  # they are stable.  Exclusions only shrink.  Each walk starts from the
  # versions the modules were requested at, and constrained to, in the
  # previous one, so that, as with Gradle, the versions a module no longer
  # selected requests do not count.  Should those versions cycle, they are
  # only raised from then on.
  set(_ids)
  set(_states)
  set(_monotonic FALSE)
  set(_walk TRUE)
  while(_walk)
    set(_walk FALSE)
    set(_visited)
    set(_visited_modules)
    set(_selected_files)
    set(_pending)
    foreach(_id IN LISTS _ids)
      set(_selected_${_id} "${_floor_${_id}}")
      set(_constraint_${_id} "${_floor_constraint_${_id}}")
      set(_requested_${_id})
      set(_constrained_${_id})
    endforeach()
    # Entries are `kind|group:artifact:version|excludes`.
    set(_queue)
    foreach(_dependency IN LISTS CPACK_APK_MAVEN_DEPENDENCIES)
      list(APPEND _queue "library|${_dependency}|")
    endforeach()
    while(_queue)
      while(_queue)
        list(POP_FRONT _queue _entry)
        string(REPLACE "|" ";" _fields "${_entry}")
        list(GET _fields 0 _kind)
        list(GET _fields 1 _coordinates)
        list(GET _fields 2 _incoming)
        string(REPLACE "," ";" _incoming "${_incoming}")
        _cpack_apk_maven_split("${_coordinates}" _g _a _v)
        string(MAKE_C_IDENTIFIER "${_g}:${_a}" _id)
        list(APPEND _ids "${_id}")
        if(NOT _v STREQUAL "")
          _cpack_apk_maven_version_higher("${_v}" "${_requested_${_id}}"
            _higher)
          if(_higher)
            set(_requested_${_id} "${_v}")
          endif()
        endif()
        foreach(_candidate IN ITEMS "${_v}" "${_constraint_${_id}}")
          if(_candidate STREQUAL "")
            continue()
          endif()
          _cpack_apk_maven_version_higher("${_candidate}"
            "${_selected_${_id}}" _higher)
          if(_higher)
            set(_selected_${_id} "${_candidate}")
            if("${_g}:${_a}" IN_LIST _visited_modules)
              set(_walk TRUE)
            endif()
          endif()
        endforeach()

        # Exclusions apply below a module if they apply on every path to it.
        string(MAKE_C_IDENTIFIER "${_kind}:${_g}:${_a}" _node)
        if(NOT DEFINED _excludes_${_node})
          set(_excludes_${_node} "${_incoming}")
        else()
          set(_common)
          foreach(_exclude IN LISTS _excludes_${_node})
            if(_exclude IN_LIST _incoming)
              list(APPEND _common "${_exclude}")
            endif()
          endforeach()
          if(NOT "${_common}" STREQUAL "${_excludes_${_node}}")
            set(_excludes_${_node} "${_common}")
            if("${_kind}|${_g}:${_a}" IN_LIST _visited)
              set(_walk TRUE)
            endif()
          endif()
        endif()

        if("${_kind}|${_g}:${_a}" IN_LIST _visited)
          continue()
        endif()
        if("${_selected_${_id}}" STREQUAL "")
          # Required without a version, which a platform or a dependency
          # constraint may give later.
          list(APPEND _pending "${_entry}")
          continue()
        endif()
        list(APPEND _visited "${_kind}|${_g}:${_a}")
        list(APPEND _visited_modules "${_g}:${_a}")
        set(_v "${_selected_${_id}}")

        string(MAKE_C_IDENTIFIER "${_kind}:${_g}:${_a}:${_v}" _key)
        if(NOT DEFINED _metadata_${_key}_files)
          _cpack_apk_maven_metadata(${_kind} ${_g} ${_a} ${_v})
          foreach(_field IN ITEMS deps constraints redirect files)
            set(_metadata_${_key}_${_field} "${_maven_${_field}}")
          endforeach()
        endif()

        foreach(_constraint IN LISTS _metadata_${_key}_constraints)
          _cpack_apk_maven_split("${_constraint}" _cg _ca _cv)
          string(MAKE_C_IDENTIFIER "${_cg}:${_ca}" _cid)
          list(APPEND _ids "${_cid}")
          _cpack_apk_maven_version_higher("${_cv}" "${_constrained_${_cid}}"
            _higher)
          if(_higher)
            set(_constrained_${_cid} "${_cv}")
          endif()
          _cpack_apk_maven_version_higher("${_cv}" "${_constraint_${_cid}}"
            _higher)
          if(_higher)
            set(_constraint_${_cid} "${_cv}")
            if("${_cg}:${_ca}" IN_LIST _visited_modules)
              _cpack_apk_maven_version_higher("${_cv}" "${_selected_${_cid}}"
                _higher)
              if(_higher)
                set(_walk TRUE)
              endif()
            endif()
          endif()
        endforeach()

        set(_excludes "${_excludes_${_node}}")
        foreach(_dep IN LISTS _metadata_${_key}_deps)
          string(REPLACE "|" ";" _fields "${_dep}")
          list(GET _fields 0 _dep_kind)
          list(GET _fields 1 _dep_coordinates)
          list(GET _fields 2 _dep_excludes)
          _cpack_apk_maven_split("${_dep_coordinates}" _dg _da _dv)
          _cpack_apk_maven_excluded(${_dg} ${_da} "${_excludes}" _excluded)
          if(_excluded)
            continue()
          endif()
          string(REPLACE "," ";" _dep_excludes "${_dep_excludes}")
          set(_below ${_excludes} ${_dep_excludes})
          list(REMOVE_DUPLICATES _below)
          list(JOIN _below "," _below)
          list(APPEND _queue "${_dep_kind}|${_dep_coordinates}|${_below}")
        endforeach()
        if(_metadata_${_key}_redirect)
          list(JOIN _excludes "," _below)
          list(APPEND _queue
            "${_kind}|${_metadata_${_key}_redirect}|${_below}")
        endif()
        foreach(_file IN LISTS _metadata_${_key}_files)
          list(APPEND _selected_files "${_g}:${_a}:${_v}|${_file}")
        endforeach()
      endwhile()

      # Modules required without a version may have been given one since.
      set(_waiting)
      foreach(_entry IN LISTS _pending)
        string(REPLACE "|" ";" _fields "${_entry}")
        list(GET _fields 1 _coordinates)
        _cpack_apk_maven_split("${_coordinates}" _g _a _v)
        string(MAKE_C_IDENTIFIER "${_g}:${_a}" _id)
        if(NOT "${_selected_${_id}}${_constraint_${_id}}" STREQUAL "")
          list(APPEND _queue "${_entry}")
        else()
          list(APPEND _waiting "${_entry}")
        endif()
      endforeach()
      set(_pending "${_waiting}")
    endwhile()

    # The versions the next walk starts from.
    list(REMOVE_DUPLICATES _ids)
    set(_changed FALSE)
    set(_state)
    foreach(_id IN LISTS _ids)
      foreach(_floor_kind IN ITEMS requested constrained)
        if(_floor_kind STREQUAL "requested")
          set(_floor_var _floor_${_id})
        else()
          set(_floor_var _floor_constraint_${_id})
        endif()
        set(_floor "${_${_floor_kind}_${_id}}")
        if(_monotonic)
          _cpack_apk_maven_version_higher("${${_floor_var}}" "${_floor}"
            _higher)
          if(_higher)
            set(_floor "${${_floor_var}}")
          endif()
        endif()
        if(NOT "${_floor}" STREQUAL "${${_floor_var}}")
          set(${_floor_var} "${_floor}")
          set(_changed TRUE)
        endif()
        list(APPEND _state "${_floor}")
      endforeach()
    endforeach()
    if(_changed)
      set(_walk TRUE)
      if(NOT _monotonic)
        string(MD5 _state "${_ids};${_state}")
        if(_state IN_LIST _states)
          set(_monotonic TRUE)
        else()
          list(APPEND _states "${_state}")
        endif()
      endif()
    endif()

    if(_pending AND NOT _walk)
      set(_modules)
      foreach(_entry IN LISTS _pending)
        string(REPLACE "|" ";" _fields "${_entry}")
        list(GET _fields 1 _coordinates)
        _cpack_apk_maven_split("${_coordinates}" _g _a _v)
        list(APPEND _modules "${_g}:${_a}")
      endforeach()
      list(REMOVE_DUPLICATES _modules)
      list(JOIN _modules "\n  " _modules)
      message(FATAL_ERROR
        "CPack APK: The Maven artifacts\n  ${_modules}\nare required "
        "without a version, and no platform or dependency constraint gives "
        "one.")
    endif()
  endwhile()

  set(_archives)
  foreach(_entry IN LISTS _selected_files)
    string(REPLACE "|" ";" _fields "${_entry}")
    list(GET _fields 0 _coordinates)
    list(GET _fields 1 _name)
    list(GET _fields 2 _hash)
    _cpack_apk_maven_split("${_coordinates}" _g _a _v)
    _cpack_apk_maven_fetch(${_g} ${_a} ${_v} "${_name}" _path ${_hash})
    if(NOT _path)
      message(FATAL_ERROR
        "CPack APK: '${_name}' of the Maven artifact '${_coordinates}' could "
        "not be downloaded.")
    endif()
    # Only code archives are consumed; a module may publish other files.
    if(_path MATCHES "\\.(aar|jar)$")
      list(APPEND _archives "${_path}")
      _cpack_apk_debug("Maven dependency: ${_path}")
    endif()
  endforeach()
  set(${result} "${_archives}" PARENT_SCOPE)
endfunction()
