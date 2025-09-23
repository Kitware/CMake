#!/usr/bin/env python3
"""
This script inserts "versionadded" directives into .rst documents found in the
Help/ directory and module documentation comments found in the Modules/
directory. It can be run from any directory within the CMake repository.

Each file is assigned a CMake version in which it first appears,
according to the git version tags.

Options:

  --overwrite     Replace existing "versionadded" directives.
                  Default: existing directives are left unchanged.

  --baseline      Files present in this tag don't need a version directive.
                  Default: v3.0.0

  --since         Files present in this tag will be ignored.
                  Only newer files will be operated on.
                  Default: v3.0.0

  --next-version  The next CMake version, which hasn't been tagged yet.
                  Default: extracted from Source/CMakeVersion.cmake
"""
import re
import pathlib
import subprocess
import argparse

tag_re = re.compile(r'^v[34]\.(\d+)\.(\d+)(?:-rc(\d+))?$')
path_re = re.compile(r'Help/(?!dev|guide|manual|cpack_|release).*\.rst|Modules/[^/]*\.cmake$')

def git_root():
    """Return the root of the .git repository from the current directory."""
    result = subprocess.run(
        ['git', 'rev-parse', '--show-toplevel'], check=True, universal_newlines=True, capture_output=True)
    return pathlib.Path(result.stdout.strip())

def git_tags():
    """Return a list of CMake version tags from the repository."""
    result = subprocess.run(['git', 'tag'], check=True, universal_newlines=True, capture_output=True)
    return [tag for tag in result.stdout.splitlines() if tag_re.match(tag)]

def git_list_tree(ref):
    """Return a list of help and module files in a given git reference."""
    result = subprocess.run(
        ['git', 'ls-tree', '-r', '--full-name', '--name-only', ref, ':/'],
        check=True, universal_newlines=True, capture_output=True)
    return [path for path in result.stdout.splitlines() if path_re.match(path)]

def tag_version(tag):
    """Extract a clean CMake version from a git version tag."""
    return re.sub(r'^v|\.0(-rc\d+)?$', '', tag)

def tag_sortkey(tag):
    """Sorting key for a git version tag."""
    return tuple(int(part or '1000') for part in tag_re.match(tag).groups())

def make_version_map(baseline, since, next_version):
    """Map repository file paths to CMake versions in which they first appear."""
    versions = {}
    if next_version:
        for path in git_list_tree('HEAD'):
            versions[path] = next_version
    for tag in sorted(git_tags(), key=tag_sortkey, reverse=True):
        version = tag_version(tag)
        for path in git_list_tree(tag):
            versions[path] = version
    if baseline:
        for path in git_list_tree(baseline):
            versions[path] = None
    if since:
        for path in git_list_tree(since):
            versions.pop(path, None)
    return versions

cmake_version_re = re.compile(
    rb'set\(CMake_VERSION_MAJOR\s+(\d+)\)\s+set\(CMake_VERSION_MINOR\s+(\d+)\)\s+set\(CMake_VERSION_PATCH\s+(\d+)\)', re.S)

def cmake_version(path):
    """Extract the current MAJOR.MINOR CMake version from CMakeVersion.cmake found at `path`."""
    match = cmake_version_re.search(path.read_bytes())
    major, minor, patch = map(int, match.groups())
    minor += patch > 20000000  # nightly version will become the next minor
    return f'{major}.{minor}'

stamp_re = re.compile(
    rb'(?P<PREFIX>(^|\[\.rst:\r?\n)[^\r\n]+\r?\n[*^\-=#]+(?P<NL>\r?\n))(?P<STAMP>\s*\.\. versionadded::[^\r\n]*\r?\n)?')
stamp_pattern_add = rb'\g<PREFIX>\g<NL>.. versionadded:: VERSION\g<NL>'
stamp_pattern_remove = rb'\g<PREFIX>'

def update_file(path, version, overwrite):
    try:
        data = path.read_bytes()
    except FileNotFoundError as e:
        return False

    def _replacement(match):
        if not overwrite and match.start('STAMP') != -1:
            return match.group()
        if version:
            pattern = stamp_pattern_add.replace(b'VERSION', version.encode('utf-8'))
        else:
            pattern = stamp_pattern_remove
        return match.expand(pattern)

    new_data, nrepl = stamp_re.subn(_replacement, data, 1)
    if nrepl and new_data != data:
        path.write_bytes(new_data)
        return True
    return False

def update_repo(repo_root, version_map, overwrite):
    total = 0
    for path, version in version_map.items():
        if update_file(repo_root / path, version, overwrite):
            print(f"Version {version or '<none>':6} for {path}")
            total += 1
    print(f"Updated {total} file(s)")

def main():
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument('--overwrite', action='store_true', help="overwrite existing version tags")
    parser.add_argument('--baseline', metavar='TAG', default='v3.0.0',
        help="files present in this tag don't need a version directive (default: v3.0.0)")
    parser.add_argument('--since', metavar='TAG',
        help="apply changes only to files added after this tag")
    parser.add_argument('--next-version', metavar='VER',
        help="version for files not present in any tag (default: from CMakeVersion.cmake)")
    args = parser.parse_args()

    try:
        repo_root = git_root()
        next_version = args.next_version or cmake_version(repo_root / 'Source/CMakeVersion.cmake')
        version_map = make_version_map(args.baseline, args.since, next_version)
        update_repo(repo_root, version_map, args.overwrite)
    except subprocess.CalledProcessError as e:
        print(f"Command '{' '.join(e.cmd)}' returned code {e.returncode}:\n{e.stderr.strip()}")

if __name__ == '__main__':
    main()
