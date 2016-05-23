#!/usr/bin/env bash
#=============================================================================
# Copyright 2015-2016 Kitware, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#=============================================================================

usage='usage: clang-format.bash [<options>] [--]

    --help                     Print usage plus more detailed help.

    --clang-format <tool>      Use given clang-format tool.

    --amend                    Filter files changed by HEAD.
    --cached                   Filter files locally staged for commit.
    --modified                 Filter files locally modified from HEAD.
    --tracked                  Filter files tracked by Git.
'

help="$usage"'
Example to format locally modified files:

    Utilities/Scripts/clang-format.bash --modified

Example to format locally modified files staged for commit:

    Utilities/Scripts/clang-format.bash --cached

Example to format the current topic:

    git filter-branch \
      --tree-filter "Utilities/Scripts/clang-format.bash --amend" \
      master..

Example to format all files:

    Utilities/Scripts/clang-format.bash --tracked
'

die() {
    echo "$@" 1>&2; exit 1
}

#-----------------------------------------------------------------------------

# Parse command-line arguments.
clang_format=''
mode=''
while test "$#" != 0; do
    case "$1" in
    --amend) mode="amend" ;;
    --cached) mode="cached" ;;
    --clang-format) shift; clang_format="$1" ;;
    --help) echo "$help"; exit 0 ;;
    --modified) mode="modified" ;;
    --tracked) mode="tracked" ;;
    --) shift ; break ;;
    -*) die "$usage" ;;
    *) break ;;
    esac
    shift
done
test "$#" = 0 || die "$usage"

# Find a default tool.
tools='
  clang-format
  clang-format-3.8
'
if test "x$clang_format" = "x"; then
    for tool in $tools; do
        if type -p "$tool" >/dev/null; then
            clang_format="$tool"
            break
        fi
    done
fi

# Verify that we have a tool.
if ! type -p "$clang_format" >/dev/null; then
    echo "Unable to locate '$clang_format'"
    exit 1
fi

# Select listing mode.
case "$mode" in
    '')       echo "$usage"; exit 0 ;;
    amend)    git_ls='git diff-tree  --diff-filter=AM --name-only HEAD -r --no-commit-id' ;;
    cached)   git_ls='git diff-index --diff-filter=AM --name-only HEAD --cached' ;;
    modified) git_ls='git diff-index --diff-filter=AM --name-only HEAD' ;;
    tracked)  git_ls='git ls-files' ;;
    *) die "invalid mode: $mode" ;;
esac

# Filter sources to which our style should apply.
$git_ls -z -- '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hh' '*.hpp' '*.hxx' |

  # Exclude lexer/parser generator input and output.
  egrep -z -v '^Source/cmCommandArgumentLexer\.' |
  egrep -z -v '^Source/cmCommandArgumentParser(\.y|\.cxx|Tokens\.h)' |
  egrep -z -v '^Source/cmDependsJavaLexer\.' |
  egrep -z -v '^Source/cmDependsJavaParser(\.y|\.cxx|Tokens\.h)' |
  egrep -z -v '^Source/cmExprLexer\.' |
  egrep -z -v '^Source/cmExprParser(\.y|\.cxx|Tokens\.h)' |
  egrep -z -v '^Source/cmFortranLexer\.' |
  egrep -z -v '^Source/cmFortranParser(\.y|\.cxx|Tokens\.h)' |
  egrep -z -v '^Source/cmListFileLexer(\.in\.l|\.c)' |

  # Exclude third-party sources.
  egrep -z -v '^Source/(cm_sha2|bindexplib)' |
  egrep -z -v '^Source/(kwsys|CursesDialog/form)/' |
  egrep -z -v '^Utilities/(KW|cm).*/' |

  # Exclude reference content.
  egrep -z -v '^Tests/Module/GenerateExportHeader/reference/' |

  # Exclude manually-formatted sources (e.g. with long lines).
  egrep -z -v '^Tests/PositionIndependentTargets/pic_test.h' |

  # Exclude sources with encoding not suported by clang-format.
  egrep -z -v '^Tests/RunCMake/CommandLine/cmake_depends/test_UTF-16LE.h' |

  # Update sources in-place.
  xargs -0 "$clang_format" -i
