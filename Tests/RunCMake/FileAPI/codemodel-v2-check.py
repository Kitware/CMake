from check_index import *

import sys
import os

def check_objects(o, g):
    assert is_list(o)
    assert len(o) == 1
    check_index_object(o[0], "codemodel", 2, 0, check_object_codemodel(g))

def check_backtrace(t, b, backtrace):
    btg = t["backtraceGraph"]
    for expected in backtrace:
        assert is_int(b)
        node = btg["nodes"][b]
        expected_keys = ["file"]
        assert matches(btg["files"][node["file"]], expected["file"])

        if expected["line"] is not None:
            expected_keys.append("line")
            assert is_int(node["line"], expected["line"])

        if expected["command"] is not None:
            expected_keys.append("command")
            assert is_int(node["command"])
            assert is_string(btg["commands"][node["command"]], expected["command"])

        if expected["hasParent"]:
            expected_keys.append("parent")
            assert is_int(node["parent"])
            b = node["parent"]
        else:
            b = None

        assert sorted(node.keys()) == sorted(expected_keys)

    assert b is None

def check_directory(c):
    def _check(actual, expected):
        assert is_dict(actual)
        expected_keys = ["build", "source", "projectIndex"]
        assert matches(actual["build"], expected["build"])

        assert is_int(actual["projectIndex"])
        assert is_string(c["projects"][actual["projectIndex"]]["name"], expected["projectName"])

        if expected["parentSource"] is not None:
            expected_keys.append("parentIndex")
            assert is_int(actual["parentIndex"])
            assert matches(c["directories"][actual["parentIndex"]]["source"], expected["parentSource"])

        if expected["childSources"] is not None:
            expected_keys.append("childIndexes")
            check_list_match(lambda a, e: matches(c["directories"][a]["source"], e),
                             actual["childIndexes"], expected["childSources"],
                             missing_exception=lambda e: "Child source: %s" % e,
                             extra_exception=lambda a: "Child source: %s" % a["source"])

        if expected["targetIds"] is not None:
            expected_keys.append("targetIndexes")
            check_list_match(lambda a, e: matches(c["targets"][a]["id"], e),
                             actual["targetIndexes"], expected["targetIds"],
                             missing_exception=lambda e: "Target ID: %s" % e,
                             extra_exception=lambda a: "Target ID: %s" % c["targets"][a]["id"])

        if expected["minimumCMakeVersion"] is not None:
            expected_keys.append("minimumCMakeVersion")
            assert is_dict(actual["minimumCMakeVersion"])
            assert sorted(actual["minimumCMakeVersion"].keys()) == ["string"]
            assert is_string(actual["minimumCMakeVersion"]["string"], expected["minimumCMakeVersion"])

        if expected["hasInstallRule"] is not None:
            expected_keys.append("hasInstallRule")
            assert is_bool(actual["hasInstallRule"], expected["hasInstallRule"])

        assert sorted(actual.keys()) == sorted(expected_keys)

    return _check

def check_target_backtrace_graph(t):
    btg = t["backtraceGraph"]
    assert is_dict(btg)
    assert sorted(btg.keys()) == ["commands", "files", "nodes"]
    assert is_list(btg["commands"])

    for c in btg["commands"]:
        assert is_string(c)

    for f in btg["files"]:
        assert is_string(f)

    for n in btg["nodes"]:
        expected_keys = ["file"]
        assert is_dict(n)
        assert is_int(n["file"])
        assert 0 <= n["file"] < len(btg["files"])

        if "line" in n:
            expected_keys.append("line")
            assert is_int(n["line"])

        if "command" in n:
            expected_keys.append("command")
            assert is_int(n["command"])
            assert 0 <= n["command"] < len(btg["commands"])

        if "parent" in n:
            expected_keys.append("parent")
            assert is_int(n["parent"])
            assert 0 <= n["parent"] < len(btg["nodes"])

        assert sorted(n.keys()) == sorted(expected_keys)

def check_target(c):
    def _check(actual, expected):
        assert is_dict(actual)
        assert sorted(actual.keys()) == ["directoryIndex", "id", "jsonFile", "name", "projectIndex"]
        assert is_int(actual["directoryIndex"])
        assert matches(c["directories"][actual["directoryIndex"]]["source"], expected["directorySource"])
        assert is_string(actual["name"], expected["name"])
        assert is_string(actual["jsonFile"])
        assert is_int(actual["projectIndex"])
        assert is_string(c["projects"][actual["projectIndex"]]["name"], expected["projectName"])

        filepath = os.path.join(reply_dir, actual["jsonFile"])
        with open(filepath) as f:
            obj = json.load(f)

        expected_keys = ["name", "id", "type", "backtraceGraph", "paths", "sources"]
        assert is_dict(obj)
        assert is_string(obj["name"], expected["name"])
        assert matches(obj["id"], expected["id"])
        assert is_string(obj["type"], expected["type"])
        check_target_backtrace_graph(obj)

        assert is_dict(obj["paths"])
        assert sorted(obj["paths"].keys()) == ["build", "source"]
        assert matches(obj["paths"]["build"], expected["build"])
        assert matches(obj["paths"]["source"], expected["source"])

        def check_source(actual, expected):
            assert is_dict(actual)
            expected_keys = ["path"]

            if expected["compileGroupLanguage"] is not None:
                expected_keys.append("compileGroupIndex")
                assert is_string(obj["compileGroups"][actual["compileGroupIndex"]]["language"], expected["compileGroupLanguage"])

            if expected["sourceGroupName"] is not None:
                expected_keys.append("sourceGroupIndex")
                assert is_string(obj["sourceGroups"][actual["sourceGroupIndex"]]["name"], expected["sourceGroupName"])

            if expected["isGenerated"] is not None:
                expected_keys.append("isGenerated")
                assert is_bool(actual["isGenerated"], expected["isGenerated"])

            if expected["backtrace"] is not None:
                expected_keys.append("backtrace")
                check_backtrace(obj, actual["backtrace"], expected["backtrace"])

            assert sorted(actual.keys()) == sorted(expected_keys)

        check_list_match(lambda a, e: matches(a["path"], e["path"]), obj["sources"],
                         expected["sources"], check=check_source,
                         check_exception=lambda a, e: "Source file: %s" % a["path"],
                         missing_exception=lambda e: "Source file: %s" % e["path"],
                         extra_exception=lambda a: "Source file: %s" % a["path"])

        if expected["backtrace"] is not None:
            expected_keys.append("backtrace")
            check_backtrace(obj, obj["backtrace"], expected["backtrace"])

        if expected["folder"] is not None:
            expected_keys.append("folder")
            assert is_dict(obj["folder"])
            assert sorted(obj["folder"].keys()) == ["name"]
            assert is_string(obj["folder"]["name"], expected["folder"])

        if expected["nameOnDisk"] is not None:
            expected_keys.append("nameOnDisk")
            assert matches(obj["nameOnDisk"], expected["nameOnDisk"])

        if expected["artifacts"] is not None:
            expected_keys.append("artifacts")

            def check_artifact(actual, expected):
                assert is_dict(actual)
                assert sorted(actual.keys()) == ["path"]

            check_list_match(lambda a, e: matches(a["path"], e["path"]),
                             obj["artifacts"], expected["artifacts"],
                             check=check_artifact,
                             check_exception=lambda a, e: "Artifact: %s" % a["path"],
                             missing_exception=lambda e: "Artifact: %s" % e["path"],
                             extra_exception=lambda a: "Artifact: %s" % a["path"])

        if expected["isGeneratorProvided"] is not None:
            expected_keys.append("isGeneratorProvided")
            assert is_bool(obj["isGeneratorProvided"], expected["isGeneratorProvided"])

        if expected["install"] is not None:
            expected_keys.append("install")
            assert is_dict(obj["install"])
            assert sorted(obj["install"].keys()) == ["destinations", "prefix"]

            assert is_dict(obj["install"]["prefix"])
            assert sorted(obj["install"]["prefix"].keys()) == ["path"]
            assert matches(obj["install"]["prefix"]["path"], expected["install"]["prefix"])

            def check_install_destination(actual, expected):
                assert is_dict(actual)
                expected_keys = ["path"]

                if expected["backtrace"] is not None:
                    expected_keys.append("backtrace")
                    check_backtrace(obj, actual["backtrace"], expected["backtrace"])

                assert sorted(actual.keys()) == sorted(expected_keys)

            check_list_match(lambda a, e: matches(a["path"], e["path"]),
                             obj["install"]["destinations"], expected["install"]["destinations"],
                             check=check_install_destination,
                             check_exception=lambda a, e: "Install path: %s" % a["path"],
                             missing_exception=lambda e: "Install path: %s" % e["path"],
                             extra_exception=lambda a: "Install path: %s" % a["path"])

        if expected["link"] is not None:
            expected_keys.append("link")
            assert is_dict(obj["link"])
            link_keys = ["language"]

            assert is_string(obj["link"]["language"], expected["link"]["language"])

            if "commandFragments" in obj["link"]:
                link_keys.append("commandFragments")
                assert is_list(obj["link"]["commandFragments"])
                for f in obj["link"]["commandFragments"]:
                    assert is_dict(f)
                    assert sorted(f.keys()) == ["fragment", "role"] or sorted(f.keys()) == ["backtrace", "fragment", "role"]
                    assert is_string(f["fragment"])
                    assert is_string(f["role"])
                    assert f["role"] in ("flags", "libraries", "libraryPath", "frameworkPath")

            if expected["link"]["commandFragments"] is not None:
                def check_link_command_fragments(actual, expected):
                    assert is_dict(actual)
                    expected_keys = ["fragment", "role"]

                    if expected["backtrace"] is not None:
                        expected_keys.append("backtrace")
                        assert matches(actual["fragment"], expected["fragment"])
                        assert actual["role"] == expected["role"]
                        check_backtrace(obj, actual["backtrace"], expected["backtrace"])

                    assert sorted(actual.keys()) == sorted(expected_keys)

                check_list_match(lambda a, e: matches(a["fragment"], e["fragment"]),
                                 obj["link"]["commandFragments"], expected["link"]["commandFragments"],
                                 check=check_link_command_fragments,
                                 check_exception=lambda a, e: "Link fragment: %s" % a["fragment"],
                                 missing_exception=lambda e: "Link fragment: %s" % e["fragment"],
                                 extra_exception=lambda a: "Link fragment: %s" % a["fragment"],
                                 allow_extra=True)

            if expected["link"]["lto"] is not None:
                link_keys.append("lto")
                assert is_bool(obj["link"]["lto"], expected["link"]["lto"])

            # FIXME: Properly test sysroot
            if "sysroot" in obj["link"]:
                link_keys.append("sysroot")
                assert is_string(obj["link"]["sysroot"])

            assert sorted(obj["link"].keys()) == sorted(link_keys)

        if expected["archive"] is not None:
            expected_keys.append("archive")
            assert is_dict(obj["archive"])
            archive_keys = []

            # FIXME: Properly test commandFragments
            if "commandFragments" in obj["archive"]:
                archive_keys.append("commandFragments")
                assert is_list(obj["archive"]["commandFragments"])
                for f in obj["archive"]["commandFragments"]:
                    assert is_dict(f)
                    assert sorted(f.keys()) == ["fragment", "role"]
                    assert is_string(f["fragment"])
                    assert is_string(f["role"])
                    assert f["role"] in ("flags")

            if expected["archive"]["lto"] is not None:
                archive_keys.append("lto")
                assert is_bool(obj["archive"]["lto"], expected["archive"]["lto"])

            assert sorted(obj["archive"].keys()) == sorted(archive_keys)

        if expected["dependencies"] is not None:
            expected_keys.append("dependencies")

            def check_dependency(actual, expected):
                assert is_dict(actual)
                expected_keys = ["id"]

                if expected["backtrace"] is not None:
                    expected_keys.append("backtrace")
                    check_backtrace(obj, actual["backtrace"], expected["backtrace"])

                assert sorted(actual.keys()) == sorted(expected_keys)

            check_list_match(lambda a, e: matches(a["id"], e["id"]),
                             obj["dependencies"], expected["dependencies"],
                             check=check_dependency,
                             check_exception=lambda a, e: "Dependency ID: %s" % a["id"],
                             missing_exception=lambda e: "Dependency ID: %s" % e["id"],
                             extra_exception=lambda a: "Dependency ID: %s" % a["id"])

        if expected["sourceGroups"] is not None:
            expected_keys.append("sourceGroups")

            def check_source_group(actual, expected):
                assert is_dict(actual)
                assert sorted(actual.keys()) == ["name", "sourceIndexes"]

                check_list_match(lambda a, e: matches(obj["sources"][a]["path"], e),
                                 actual["sourceIndexes"], expected["sourcePaths"],
                                 missing_exception=lambda e: "Source path: %s" % e,
                                 extra_exception=lambda a: "Source path: %s" % obj["sources"][a]["path"])

            check_list_match(lambda a, e: is_string(a["name"], e["name"]),
                             obj["sourceGroups"], expected["sourceGroups"],
                             check=check_source_group,
                             check_exception=lambda a, e: "Source group: %s" % a["name"],
                             missing_exception=lambda e: "Source group: %s" % e["name"],
                             extra_exception=lambda a: "Source group: %s" % a["name"])

        if expected["compileGroups"] is not None:
            expected_keys.append("compileGroups")

            def check_compile_group(actual, expected):
                assert is_dict(actual)
                expected_keys = ["sourceIndexes", "language"]

                check_list_match(lambda a, e: matches(obj["sources"][a]["path"], e),
                                 actual["sourceIndexes"], expected["sourcePaths"],
                                 missing_exception=lambda e: "Source path: %s" % e,
                                 extra_exception=lambda a: "Source path: %s" % obj["sources"][a]["path"])

                if "compileCommandFragments" in actual:
                    expected_keys.append("compileCommandFragments")
                    assert is_list(actual["compileCommandFragments"])
                    for f in actual["compileCommandFragments"]:
                        assert is_dict(f)
                        assert is_string(f["fragment"])

                if expected["compileCommandFragments"] is not None:
                    def check_compile_command_fragments(actual, expected):
                        assert is_dict(actual)
                        expected_keys = ["fragment"]

                        if expected["backtrace"] is not None:
                            expected_keys.append("backtrace")
                            assert actual["fragment"] == expected["fragment"]
                            check_backtrace(obj, actual["backtrace"], expected["backtrace"])

                        assert sorted(actual.keys()) == sorted(expected_keys)

                    check_list_match(lambda a, e: is_string(a["fragment"], e["fragment"]),
                                     actual["compileCommandFragments"], expected["compileCommandFragments"],
                                     check=check_compile_command_fragments,
                                     check_exception=lambda a, e: "Compile fragment: %s" % a["fragment"],
                                     missing_exception=lambda e: "Compile fragment: %s" % e["fragment"],
                                     extra_exception=lambda a: "Compile fragment: %s" % a["fragment"],
                                     allow_extra=True)

                if expected["includes"] is not None:
                    expected_keys.append("includes")

                    def check_include(actual, expected):
                        assert is_dict(actual)
                        expected_keys = ["path"]

                        if expected["isSystem"] is not None:
                            expected_keys.append("isSystem")
                            assert is_bool(actual["isSystem"], expected["isSystem"])

                        if expected["backtrace"] is not None:
                            expected_keys.append("backtrace")
                            check_backtrace(obj, actual["backtrace"], expected["backtrace"])

                        assert sorted(actual.keys()) == sorted(expected_keys)

                    check_list_match(lambda a, e: matches(a["path"], e["path"]),
                                     actual["includes"], expected["includes"],
                                     check=check_include,
                                     check_exception=lambda a, e: "Include path: %s" % a["path"],
                                     missing_exception=lambda e: "Include path: %s" % e["path"],
                                     extra_exception=lambda a: "Include path: %s" % a["path"])

                if expected["defines"] is not None:
                    expected_keys.append("defines")

                    def check_define(actual, expected):
                        assert is_dict(actual)
                        expected_keys = ["define"]

                        if expected["backtrace"] is not None:
                            expected_keys.append("backtrace")
                            check_backtrace(obj, actual["backtrace"], expected["backtrace"])

                        assert sorted(actual.keys()) == sorted(expected_keys)

                    check_list_match(lambda a, e: is_string(a["define"], e["define"]),
                                     actual["defines"], expected["defines"],
                                     check=check_define,
                                     check_exception=lambda a, e: "Define: %s" % a["define"],
                                     missing_exception=lambda e: "Define: %s" % e["define"],
                                     extra_exception=lambda a: "Define: %s" % a["define"])

                # FIXME: Properly test sysroot
                if "sysroot" in actual:
                    expected_keys.append("sysroot")
                    assert is_string(actual["sysroot"])

                assert sorted(actual.keys()) == sorted(expected_keys)

            check_list_match(lambda a, e: is_string(a["language"], e["language"]),
                             obj["compileGroups"], expected["compileGroups"],
                             check=check_compile_group,
                             check_exception=lambda a, e: "Compile group: %s" % a["language"],
                             missing_exception=lambda e: "Compile group: %s" % e["language"],
                             extra_exception=lambda a: "Compile group: %s" % a["language"])

        assert sorted(obj.keys()) == sorted(expected_keys)

    return _check

def check_project(c):
    def _check(actual, expected):
        assert is_dict(actual)
        expected_keys = ["name", "directoryIndexes"]

        check_list_match(lambda a, e: matches(c["directories"][a]["source"], e),
                         actual["directoryIndexes"], expected["directorySources"],
                         missing_exception=lambda e: "Directory source: %s" % e,
                         extra_exception=lambda a: "Directory source: %s" % c["directories"][a]["source"])

        if expected["parentName"] is not None:
            expected_keys.append("parentIndex")
            assert is_int(actual["parentIndex"])
            assert is_string(c["projects"][actual["parentIndex"]]["name"], expected["parentName"])

        if expected["childNames"] is not None:
            expected_keys.append("childIndexes")
            check_list_match(lambda a, e: is_string(c["projects"][a]["name"], e),
                             actual["childIndexes"], expected["childNames"],
                             missing_exception=lambda e: "Child name: %s" % e,
                             extra_exception=lambda a: "Child name: %s" % c["projects"][a]["name"])

        if expected["targetIds"] is not None:
            expected_keys.append("targetIndexes")
            check_list_match(lambda a, e: matches(c["targets"][a]["id"], e),
                             actual["targetIndexes"], expected["targetIds"],
                             missing_exception=lambda e: "Target ID: %s" % e,
                             extra_exception=lambda a: "Target ID: %s" % c["targets"][a]["id"])

        assert sorted(actual.keys()) == sorted(expected_keys)

    return _check

def gen_check_directories(c, g):
    expected = [
        {
            "source": "^\\.$",
            "build": "^\\.$",
            "parentSource": None,
            "childSources": [
                "^alias$",
                "^custom$",
                "^cxx$",
                "^imported$",
                "^object$",
                "^.*/Tests/RunCMake/FileAPIExternalSource$",
                "^dir$",
            ],
            "targetIds": [
                "^ALL_BUILD::@6890427a1f51a3e7e1df$",
                "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
                "^c_exe::@6890427a1f51a3e7e1df$",
                "^c_lib::@6890427a1f51a3e7e1df$",
                "^c_shared_exe::@6890427a1f51a3e7e1df$",
                "^c_shared_lib::@6890427a1f51a3e7e1df$",
                "^c_static_exe::@6890427a1f51a3e7e1df$",
                "^c_static_lib::@6890427a1f51a3e7e1df$",
                "^interface_exe::@6890427a1f51a3e7e1df$",
            ],
            "projectName": "codemodel-v2",
            "minimumCMakeVersion": "3.12",
            "hasInstallRule": True,
        },
        {
            "source": "^alias$",
            "build": "^alias$",
            "parentSource": "^\\.$",
            "childSources": None,
            "targetIds": [
                "^ALL_BUILD::@53632cba2752272bb008$",
                "^ZERO_CHECK::@53632cba2752272bb008$",
                "^c_alias_exe::@53632cba2752272bb008$",
                "^cxx_alias_exe::@53632cba2752272bb008$",
            ],
            "projectName": "Alias",
            "minimumCMakeVersion": "3.12",
            "hasInstallRule": None,
        },
        {
            "source": "^custom$",
            "build": "^custom$",
            "parentSource": "^\\.$",
            "childSources": None,
            "targetIds": [
                "^ALL_BUILD::@c11385ffed57b860da63$",
                "^ZERO_CHECK::@c11385ffed57b860da63$",
                "^custom_exe::@c11385ffed57b860da63$",
                "^custom_tgt::@c11385ffed57b860da63$",
            ],
            "projectName": "Custom",
            "minimumCMakeVersion": "3.12",
            "hasInstallRule": None,
        },
        {
            "source": "^cxx$",
            "build": "^cxx$",
            "parentSource": "^\\.$",
            "childSources": None,
            "targetIds": [
                "^ALL_BUILD::@a56b12a3f5c0529fb296$",
                "^ZERO_CHECK::@a56b12a3f5c0529fb296$",
                "^cxx_exe::@a56b12a3f5c0529fb296$",
                "^cxx_lib::@a56b12a3f5c0529fb296$",
                "^cxx_shared_exe::@a56b12a3f5c0529fb296$",
                "^cxx_shared_lib::@a56b12a3f5c0529fb296$",
                "^cxx_static_exe::@a56b12a3f5c0529fb296$",
                "^cxx_static_lib::@a56b12a3f5c0529fb296$",
            ],
            "projectName": "Cxx",
            "minimumCMakeVersion": "3.12",
            "hasInstallRule": None,
        },
        {
            "source": "^imported$",
            "build": "^imported$",
            "parentSource": "^\\.$",
            "childSources": None,
            "targetIds": [
                "^ALL_BUILD::@ba7eb709d0b48779c6c8$",
                "^ZERO_CHECK::@ba7eb709d0b48779c6c8$",
                "^link_imported_exe::@ba7eb709d0b48779c6c8$",
                "^link_imported_interface_exe::@ba7eb709d0b48779c6c8$",
                "^link_imported_object_exe::@ba7eb709d0b48779c6c8$",
                "^link_imported_shared_exe::@ba7eb709d0b48779c6c8$",
                "^link_imported_static_exe::@ba7eb709d0b48779c6c8$",
            ],
            "projectName": "Imported",
            "minimumCMakeVersion": "3.12",
            "hasInstallRule": None,
        },
        {
            "source": "^object$",
            "build": "^object$",
            "parentSource": "^\\.$",
            "childSources": None,
            "targetIds": [
                "^ALL_BUILD::@5ed5358f70faf8d8af7a$",
                "^ZERO_CHECK::@5ed5358f70faf8d8af7a$",
                "^c_object_exe::@5ed5358f70faf8d8af7a$",
                "^c_object_lib::@5ed5358f70faf8d8af7a$",
                "^cxx_object_exe::@5ed5358f70faf8d8af7a$",
                "^cxx_object_lib::@5ed5358f70faf8d8af7a$",
            ],
            "projectName": "Object",
            "minimumCMakeVersion": "3.13",
            "hasInstallRule": True,
        },
        {
            "source": "^dir$",
            "build": "^dir$",
            "parentSource": "^\\.$",
            "childSources": [
                "^dir/dir$",
            ],
            "targetIds": None,
            "projectName": "codemodel-v2",
            "minimumCMakeVersion": "3.12",
            "hasInstallRule": None,
        },
        {
            "source": "^dir/dir$",
            "build": "^dir/dir$",
            "parentSource": "^dir$",
            "childSources": None,
            "targetIds": None,
            "projectName": "codemodel-v2",
            "minimumCMakeVersion": "3.12",
            "hasInstallRule": None,
        },
        {
            "source": "^.*/Tests/RunCMake/FileAPIExternalSource$",
            "build": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild$",
            "parentSource": "^\\.$",
            "childSources": None,
            "targetIds": [
                "^ALL_BUILD::@[0-9a-f]+$",
                "^ZERO_CHECK::@[0-9a-f]+$",
                "^generated_exe::@[0-9a-f]+$",
            ],
            "projectName": "External",
            "minimumCMakeVersion": "3.12",
            "hasInstallRule": None,
        },
    ]

    if matches(g["name"], "^Visual Studio "):
        for e in expected:
            if e["parentSource"] is not None:
                e["targetIds"] = filter_list(lambda t: not matches(t, "^\\^ZERO_CHECK"), e["targetIds"])

    elif g["name"] == "Xcode":
        if ';' in os.environ.get("CMAKE_OSX_ARCHITECTURES", ""):
            for e in expected:
                e["targetIds"] = filter_list(lambda t: not matches(t, "^\\^(link_imported_object_exe)"), e["targetIds"])

    else:
        for e in expected:
            e["targetIds"] = filter_list(lambda t: not matches(t, "^\\^(ALL_BUILD|ZERO_CHECK)"), e["targetIds"])

    return expected

def check_directories(c, g):
    check_list_match(lambda a, e: matches(a["source"], e["source"]), c["directories"], gen_check_directories(c, g),
                     check=check_directory(c),
                     check_exception=lambda a, e: "Directory source: %s" % a["source"],
                     missing_exception=lambda e: "Directory source: %s" % e["source"],
                     extra_exception=lambda a: "Directory source: %s" % a["source"])

def gen_check_targets(c, g, inSource):
    expected = [
        {
            "name": "ALL_BUILD",
            "id": "^ALL_BUILD::@6890427a1f51a3e7e1df$",
            "directorySource": "^\\.$",
            "projectName": "codemodel-v2",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/CMakeFiles/ALL_BUILD$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/CMakeFiles/ALL_BUILD\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/CMakeFiles/ALL_BUILD$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/CMakeFiles/ALL_BUILD\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^\\.$",
            "source": "^\\.$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
                {
                    "id": "^interface_exe::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
                {
                    "id": "^c_lib::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
                {
                    "id": "^c_exe::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
                {
                    "id": "^c_shared_lib::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
                {
                    "id": "^c_shared_exe::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
                {
                    "id": "^c_static_lib::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
                {
                    "id": "^c_static_exe::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
                {
                    "id": "^c_alias_exe::@53632cba2752272bb008$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_alias_exe::@53632cba2752272bb008$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_lib::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_exe::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_shared_lib::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_shared_exe::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_static_lib::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_static_exe::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^c_object_lib::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
                {
                    "id": "^c_object_exe::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_object_lib::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_object_exe::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
                {
                    "id": "^link_imported_exe::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
                {
                    "id": "^link_imported_shared_exe::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
                {
                    "id": "^link_imported_static_exe::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
                {
                    "id": "^link_imported_object_exe::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
                {
                    "id": "^link_imported_interface_exe::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
                {
                    "id": "^custom_exe::@c11385ffed57b860da63$",
                    "backtrace": None,
                },
                {
                    "id": "^generated_exe::@[0-9a-f]+$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ZERO_CHECK",
            "id": "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
            "directorySource": "^\\.$",
            "projectName": "codemodel-v2",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/CMakeFiles/ZERO_CHECK$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/CMakeFiles/ZERO_CHECK\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/CMakeFiles/ZERO_CHECK$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/CMakeFiles/ZERO_CHECK\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^\\.$",
            "source": "^\\.$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": None,
        },
        {
            "name": "interface_exe",
            "id": "^interface_exe::@6890427a1f51a3e7e1df$",
            "directorySource": "^\\.$",
            "projectName": "codemodel-v2",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^include_test\\.cmake$",
                            "line": 3,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^include_test\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": [
                        {
                            "define": "interface_exe_EXPORTS",
                            "backtrace": None,
                        },
                    ],
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^include_test\\.cmake$",
                    "line": 3,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^include_test\\.cmake$",
                    "line": None,
                    "command": None,
                    "hasParent": True,
                },
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": 3,
                    "command": "include",
                    "hasParent": True,
                },
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": None,
                    "command": None,
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": 3,
                    "command": "include",
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^my_interface_exe\\.myexe$",
            "artifacts": [
                {
                    "path": "^bin/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?my_interface_exe\\.myexe$",
                    "_dllExtra": False,
                },
                {
                    "path": "^lib/my_interface_exe\\.imp$",
                    "_aixExtra": True,
                    "_dllExtra": False,
                },
                {
                    "path": "^lib/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib)?my_interface_exe\\.(dll\\.a|lib)$",
                    "_dllExtra": True,
                },
                {
                    "path": "^bin/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?my_interface_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^\\.$",
            "source": "^\\.$",
            "install": None,
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "c_lib",
            "id": "^c_lib::@6890427a1f51a3e7e1df$",
            "directorySource": "^\\.$",
            "projectName": "codemodel-v2",
            "type": "STATIC_LIBRARY",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": 5,
                            "command": "add_library",
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": 5,
                    "command": "add_library",
                    "hasParent": True,
                },
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": None,
                    "command": None,
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": 3,
                    "command": "include",
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^(lib)?c_lib\\.(a|lib)$",
            "artifacts": [
                {
                    "path": "^((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib)?c_lib\\.(a|lib)$",
                    "_dllExtra": False,
                },
            ],
            "build": "^\\.$",
            "source": "^\\.$",
            "install": None,
            "link": None,
            "archive": {
                "lto": None,
            },
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "c_exe",
            "id": "^c_exe::@6890427a1f51a3e7e1df$",
            "directorySource": "^\\.$",
            "projectName": "codemodel-v2",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": 6,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": 6,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": None,
                    "command": None,
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": 3,
                    "command": "include",
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^c_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^((Debug|Release|RelWithDebInfo|MinSizeRel)/)?c_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^((Debug|Release|RelWithDebInfo|MinSizeRel)/)?c_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^\\.$",
            "source": "^\\.$",
            "install": None,
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^c_lib::@6890427a1f51a3e7e1df$",
                    "backtrace": [
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": 7,
                            "command": "target_link_libraries",
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "id": "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "c_shared_lib",
            "id": "^c_shared_lib::@6890427a1f51a3e7e1df$",
            "directorySource": "^\\.$",
            "projectName": "codemodel-v2",
            "type": "SHARED_LIBRARY",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": 9,
                            "command": "add_library",
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": [
                        {
                            "define": "c_shared_lib_EXPORTS",
                            "backtrace": None,
                        },
                    ],
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": 9,
                    "command": "add_library",
                    "hasParent": True,
                },
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": None,
                    "command": None,
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": 3,
                    "command": "include",
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^(lib|cyg)?c_shared_lib\\.(so|dylib|dll)$",
            "artifacts": [
                {
                    "path": "^lib/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib|cyg)?c_shared_lib\\.(so|dylib|dll)$",
                    "_dllExtra": False,
                },
                {
                    "path": "^((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib)?c_shared_lib\\.(dll\\.a|lib)$",
                    "_dllExtra": True,
                },
                {
                    "path": "^lib/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib|cyg)?c_shared_lib\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^\\.$",
            "source": "^\\.$",
            "install": None,
            "link": {
                "language": "C",
                "lto": True,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "c_shared_exe",
            "id": "^c_shared_exe::@6890427a1f51a3e7e1df$",
            "directorySource": "^\\.$",
            "projectName": "codemodel-v2",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": 10,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
					"compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": 10,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": None,
                    "command": None,
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": 3,
                    "command": "include",
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^c_shared_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^((Debug|Release|RelWithDebInfo|MinSizeRel)/)?c_shared_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^((Debug|Release|RelWithDebInfo|MinSizeRel)/)?c_shared_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^\\.$",
            "source": "^\\.$",
            "install": None,
            "link": {
                "language": "C",
                "lto": True,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^c_shared_lib::@6890427a1f51a3e7e1df$",
                    "backtrace": [
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": 11,
                            "command": "target_link_libraries",
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "id": "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "c_static_lib",
            "id": "^c_static_lib::@6890427a1f51a3e7e1df$",
            "directorySource": "^\\.$",
            "projectName": "codemodel-v2",
            "type": "STATIC_LIBRARY",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": 13,
                            "command": "add_library",
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": 13,
                    "command": "add_library",
                    "hasParent": True,
                },
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": None,
                    "command": None,
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": 3,
                    "command": "include",
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^(lib)?c_static_lib\\.(a|lib)$",
            "artifacts": [
                {
                    "path": "^((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib)?c_static_lib\\.(a|lib)$",
                    "_dllExtra": False,
                },
            ],
            "build": "^\\.$",
            "source": "^\\.$",
            "install": None,
            "link": None,
            "archive": {
                "lto": True,
            },
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "c_static_exe",
            "id": "^c_static_exe::@6890427a1f51a3e7e1df$",
            "directorySource": "^\\.$",
            "projectName": "codemodel-v2",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": 14,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": 14,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^codemodel-v2\\.cmake$",
                    "line": None,
                    "command": None,
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": 3,
                    "command": "include",
                    "hasParent": True,
                },
                {
                    "file": "^CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^c_static_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^((Debug|Release|RelWithDebInfo|MinSizeRel)/)?c_static_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^((Debug|Release|RelWithDebInfo|MinSizeRel)/)?c_static_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^\\.$",
            "source": "^\\.$",
            "install": None,
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^c_static_lib::@6890427a1f51a3e7e1df$",
                    "backtrace": [
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": 15,
                            "command": "target_link_libraries",
                            "hasParent": True,
                        },
                        {
                            "file": "^codemodel-v2\\.cmake$",
                            "line": None,
                            "command": None,
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": 3,
                            "command": "include",
                            "hasParent": True,
                        },
                        {
                            "file": "^CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "id": "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ALL_BUILD",
            "id": "^ALL_BUILD::@a56b12a3f5c0529fb296$",
            "directorySource": "^cxx$",
            "projectName": "Cxx",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/cxx/CMakeFiles/ALL_BUILD$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/cxx/CMakeFiles/ALL_BUILD\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/cxx/CMakeFiles/ALL_BUILD$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/cxx/CMakeFiles/ALL_BUILD\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^cxx$",
            "source": "^cxx$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_lib::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_exe::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_shared_lib::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_shared_exe::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_static_lib::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_static_exe::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ZERO_CHECK",
            "id": "^ZERO_CHECK::@a56b12a3f5c0529fb296$",
            "directorySource": "^cxx$",
            "projectName": "Cxx",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/cxx/CMakeFiles/ZERO_CHECK$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/cxx/CMakeFiles/ZERO_CHECK\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/cxx/CMakeFiles/ZERO_CHECK$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/cxx/CMakeFiles/ZERO_CHECK\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^cxx$",
            "source": "^cxx$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": None,
        },
        {
            "name": "cxx_lib",
            "id": "^cxx_lib::@a56b12a3f5c0529fb296$",
            "directorySource": "^cxx$",
            "projectName": "Cxx",
            "type": "STATIC_LIBRARY",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.cxx$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "CXX",
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": 4,
                            "command": "add_library",
                            "hasParent": True,
                        },
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "CXX",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": 4,
                    "command": "add_library",
                    "hasParent": True,
                },
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^(lib)?cxx_lib\\.(a|lib)$",
            "artifacts": [
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib)?cxx_lib\\.(a|lib)$",
                    "_dllExtra": False,
                },
            ],
            "build": "^cxx$",
            "source": "^cxx$",
            "install": None,
            "link": None,
            "archive": {
                "lto": None,
            },
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "cxx_exe",
            "id": "^cxx_exe::@a56b12a3f5c0529fb296$",
            "directorySource": "^cxx$",
            "projectName": "Cxx",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.cxx$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "CXX",
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": 5,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "CXX",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": [
                        {
                            "fragment" : "TargetCompileOptions",
							"backtrace": [
                                {
                                    "file": "^cxx/CMakeLists\\.txt$",
                                    "line": 17,
                                    "command": "target_compile_options",
                                    "hasParent": True,
                                },
								{
                                    "file" : "^cxx/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        }
                    ],
                },
            ],
            "backtrace": [
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": 5,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": "bin",
            "nameOnDisk": "^cxx_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?cxx_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?cxx_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^cxx$",
            "source": "^cxx$",
            "install": {
                "prefix": "^(/usr/local|[A-Za-z]:.*/codemodel-v2)$",
                "destinations": [
                    {
                        "path": "bin",
                        "backtrace": [
                            {
                                "file": "^codemodel-v2\\.cmake$",
                                "line": 37,
                                "command": "install",
                                "hasParent": True,
                            },
                            {
                                "file": "^codemodel-v2\\.cmake$",
                                "line": None,
                                "command": None,
                                "hasParent": True,
                            },
                            {
                                "file": "^CMakeLists\\.txt$",
                                "line": 3,
                                "command": "include",
                                "hasParent": True,
                            },
                            {
                                "file": "^CMakeLists\\.txt$",
                                "line": None,
                                "command": None,
                                "hasParent": False,
                            },
                        ],
                    },
                ],
            },
            "link": {
                "language": "CXX",
                "lto": None,
                "commandFragments": [
                    {
                        "fragment" : "TargetLinkOptions",
                        "role" : "flags",
                        "backtrace": [
                            {
                                "file": "^cxx/CMakeLists\\.txt$",
                                "line": 18,
                                "command": "target_link_options",
                                "hasParent": True,
                            },
                            {
                                "file" : "^cxx/CMakeLists\\.txt$",
                                "line": None,
                                "command": None,
                                "hasParent": False,
                            },
                        ],
                    },
                    {
                        "fragment" : ".*TargetLinkDir\\\"?$",
                        "role" : "libraryPath",
                        "backtrace": [
                            {
                                "file": "^cxx/CMakeLists\\.txt$",
                                "line": 19,
                                "command": "target_link_directories",
                                "hasParent": True,
                            },
                            {
                                "file" : "^cxx/CMakeLists\\.txt$",
                                "line": None,
                                "command": None,
                                "hasParent": False,
                            },
                        ],
                    },
                    {
                        "fragment" : ".*cxx_lib.*",
                        "role" : "libraries",
                        "backtrace": [
                            {
                                "file": "^cxx/CMakeLists\\.txt$",
                                "line": 6,
                                "command": "target_link_libraries",
                                "hasParent": True,
                            },
                            {
                                "file" : "^cxx/CMakeLists\\.txt$",
                                "line": None,
                                "command": None,
                                "hasParent": False,
                            },
                        ],
                    },
                ],
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^cxx_lib::@a56b12a3f5c0529fb296$",
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": 6,
                            "command": "target_link_libraries",
                            "hasParent": True,
                        },
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "id": "^ZERO_CHECK::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "cxx_shared_lib",
            "id": "^cxx_shared_lib::@a56b12a3f5c0529fb296$",
            "directorySource": "^cxx$",
            "projectName": "Cxx",
            "type": "SHARED_LIBRARY",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.cxx$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "CXX",
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": 9,
                            "command": "add_library",
                            "hasParent": True,
                        },
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "CXX",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                    "includes": None,
                    "defines": [
                        {
                            "define": "cxx_shared_lib_EXPORTS",
                            "backtrace": None,
                        },
                    ],
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": 9,
                    "command": "add_library",
                    "hasParent": True,
                },
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^(lib|cyg)?cxx_shared_lib\\.(so|dylib|dll)$",
            "artifacts": [
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib|cyg)?cxx_shared_lib\\.(so|dylib|dll)$",
                    "_dllExtra": False,
                },
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib)?cxx_shared_lib\\.(dll\\.a|lib)$",
                    "_dllExtra": True,
                },
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib|cyg)?cxx_shared_lib\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^cxx$",
            "source": "^cxx$",
            "install": None,
            "link": {
                "language": "CXX",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "cxx_shared_exe",
            "id": "^cxx_shared_exe::@a56b12a3f5c0529fb296$",
            "directorySource": "^cxx$",
            "projectName": "Cxx",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.cxx$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "CXX",
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": 10,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "CXX",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": 10,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^cxx_shared_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?cxx_shared_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?cxx_shared_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^cxx$",
            "source": "^cxx$",
            "install": None,
            "link": {
                "language": "CXX",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^cxx_shared_lib::@a56b12a3f5c0529fb296$",
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": 11,
                            "command": "target_link_libraries",
                            "hasParent": True,
                        },
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "id": "^ZERO_CHECK::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "cxx_static_lib",
            "id": "^cxx_static_lib::@a56b12a3f5c0529fb296$",
            "directorySource": "^cxx$",
            "projectName": "Cxx",
            "type": "STATIC_LIBRARY",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.cxx$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "CXX",
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": 13,
                            "command": "add_library",
                            "hasParent": True,
                        },
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "CXX",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": 13,
                    "command": "add_library",
                    "hasParent": True,
                },
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^(lib)?cxx_static_lib\\.(a|lib)$",
            "artifacts": [
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?(lib)?cxx_static_lib\\.(a|lib)$",
                    "_dllExtra": False,
                },
            ],
            "build": "^cxx$",
            "source": "^cxx$",
            "install": None,
            "link": None,
            "archive": {
                "lto": None,
            },
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "cxx_static_exe",
            "id": "^cxx_static_exe::@a56b12a3f5c0529fb296$",
            "directorySource": "^cxx$",
            "projectName": "Cxx",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.cxx$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "CXX",
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": 14,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "CXX",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": 14,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^cxx/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^cxx_static_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?cxx_static_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^cxx/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?cxx_static_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^cxx$",
            "source": "^cxx$",
            "install": None,
            "link": {
                "language": "CXX",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^cxx_static_lib::@a56b12a3f5c0529fb296$",
                    "backtrace": [
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": 15,
                            "command": "target_link_libraries",
                            "hasParent": True,
                        },
                        {
                            "file": "^cxx/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "id": "^ZERO_CHECK::@a56b12a3f5c0529fb296$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ALL_BUILD",
            "id": "^ALL_BUILD::@53632cba2752272bb008$",
            "directorySource": "^alias$",
            "projectName": "Alias",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/alias/CMakeFiles/ALL_BUILD$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/alias/CMakeFiles/ALL_BUILD\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/alias/CMakeFiles/ALL_BUILD$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/alias/CMakeFiles/ALL_BUILD\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^alias/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^alias$",
            "source": "^alias$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@53632cba2752272bb008$",
                    "backtrace": None,
                },
                {
                    "id": "^c_alias_exe::@53632cba2752272bb008$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_alias_exe::@53632cba2752272bb008$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ZERO_CHECK",
            "id": "^ZERO_CHECK::@53632cba2752272bb008$",
            "directorySource": "^alias$",
            "projectName": "Alias",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/alias/CMakeFiles/ZERO_CHECK$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/alias/CMakeFiles/ZERO_CHECK\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/alias/CMakeFiles/ZERO_CHECK$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/alias/CMakeFiles/ZERO_CHECK\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^alias/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^alias$",
            "source": "^alias$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": None,
        },
        {
            "name": "c_alias_exe",
            "id": "^c_alias_exe::@53632cba2752272bb008$",
            "directorySource": "^alias$",
            "projectName": "Alias",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": 5,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^alias/CMakeLists\\.txt$",
                    "line": 5,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^alias/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^c_alias_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^alias/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?c_alias_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^alias/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?c_alias_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^alias$",
            "source": "^alias$",
            "install": None,
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^c_lib::@6890427a1f51a3e7e1df$",
                    "backtrace": [
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": 6,
                            "command": "target_link_libraries",
                            "hasParent": True,
                        },
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "id": "^ZERO_CHECK::@53632cba2752272bb008$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "cxx_alias_exe",
            "id": "^cxx_alias_exe::@53632cba2752272bb008$",
            "directorySource": "^alias$",
            "projectName": "Alias",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.cxx$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "CXX",
                    "backtrace": [
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": 9,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "CXX",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^alias/CMakeLists\\.txt$",
                    "line": 9,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^alias/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^cxx_alias_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^alias/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?cxx_alias_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^alias/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?cxx_alias_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^alias$",
            "source": "^alias$",
            "install": None,
            "link": {
                "language": "CXX",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^cxx_lib::@a56b12a3f5c0529fb296$",
                    "backtrace": [
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": 10,
                            "command": "target_link_libraries",
                            "hasParent": True,
                        },
                        {
                            "file": "^alias/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "id": "^ZERO_CHECK::@53632cba2752272bb008$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ALL_BUILD",
            "id": "^ALL_BUILD::@5ed5358f70faf8d8af7a$",
            "directorySource": "^object$",
            "projectName": "Object",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/CMakeFiles/ALL_BUILD$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/CMakeFiles/ALL_BUILD\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/CMakeFiles/ALL_BUILD$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/CMakeFiles/ALL_BUILD\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^object/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^object$",
            "source": "^object$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
                {
                    "id": "^c_object_lib::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
                {
                    "id": "^c_object_exe::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_object_lib::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
                {
                    "id": "^cxx_object_exe::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ZERO_CHECK",
            "id": "^ZERO_CHECK::@5ed5358f70faf8d8af7a$",
            "directorySource": "^object$",
            "projectName": "Object",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/CMakeFiles/ZERO_CHECK$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/CMakeFiles/ZERO_CHECK\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/CMakeFiles/ZERO_CHECK$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/CMakeFiles/ZERO_CHECK\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^object/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^object$",
            "source": "^object$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": None,
        },
        {
            "name": "c_object_lib",
            "id": "^c_object_lib::@5ed5358f70faf8d8af7a$",
            "directorySource": "^object$",
            "projectName": "Object",
            "type": "OBJECT_LIBRARY",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": 5,
                            "command": "add_library",
                            "hasParent": True,
                        },
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^object/CMakeLists\\.txt$",
                    "line": 5,
                    "command": "add_library",
                    "hasParent": True,
                },
                {
                    "file": "^object/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": [
                {
                    "path": "^object/.*/empty(\\.c)?\\.o(bj)?$",
                    "_dllExtra": False,
                },
            ],
            "build": "^object$",
            "source": "^object$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "c_object_exe",
            "id": "^c_object_exe::@5ed5358f70faf8d8af7a$",
            "directorySource": "^object$",
            "projectName": "Object",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": 6,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/.*/empty(\\.c)?\\.o(bj)?$",
                    "isGenerated": True,
                    "sourceGroupName": "Object Libraries",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": 7,
                            "command": "target_link_libraries",
                            "hasParent": True,
                        },
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
                {
                    "name": "Object Libraries",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/.*/empty(\\.c)?\\.o(bj)?$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^object/CMakeLists\\.txt$",
                    "line": 6,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^object/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^c_object_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^object/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?c_object_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^object/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?c_object_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^object$",
            "source": "^object$",
            "install": {
                "prefix": "^(/usr/local|[A-Za-z]:.*/codemodel-v2)$",
                "destinations": [
                    {
                        "path": "bin",
                        "backtrace": [
                            {
                                "file": "^object/CMakeLists\\.txt$",
                                "line": 13,
                                "command": "install",
                                "hasParent": True,
                            },
                            {
                                "file": "^object/CMakeLists\\.txt$",
                                "line": None,
                                "command": None,
                                "hasParent": False,
                            },
                        ],
                    },
                ],
            },
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^c_object_lib::@5ed5358f70faf8d8af7a$",
                    # FIXME: Add a backtrace here when it becomes available.
                    # You'll know when it's available, because this test will
                    # fail.
                    "backtrace": None,
                },
                {
                    "id": "^ZERO_CHECK::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "cxx_object_lib",
            "id": "^cxx_object_lib::@5ed5358f70faf8d8af7a$",
            "directorySource": "^object$",
            "projectName": "Object",
            "type": "OBJECT_LIBRARY",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.cxx$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "CXX",
                    "backtrace": [
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": 9,
                            "command": "add_library",
                            "hasParent": True,
                        },
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "CXX",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^object/CMakeLists\\.txt$",
                    "line": 9,
                    "command": "add_library",
                    "hasParent": True,
                },
                {
                    "file": "^object/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": [
                {
                    "path": "^object/.*/empty(\\.cxx)?\\.o(bj)?$",
                    "_dllExtra": False,
                },
            ],
            "build": "^object$",
            "source": "^object$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "cxx_object_exe",
            "id": "^cxx_object_exe::@5ed5358f70faf8d8af7a$",
            "directorySource": "^object$",
            "projectName": "Object",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.cxx$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "CXX",
                    "backtrace": [
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": 10,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/.*/empty(\\.cxx)?\\.o(bj)?$",
                    "isGenerated": True,
                    "sourceGroupName": "Object Libraries",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": 11,
                            "command": "target_link_libraries",
                            "hasParent": True,
                        },
                        {
                            "file": "^object/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                },
                {
                    "name": "Object Libraries",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/object/.*/empty(\\.cxx)?\\.o(bj)?$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "CXX",
                    "sourcePaths": [
                        "^empty\\.cxx$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^object/CMakeLists\\.txt$",
                    "line": 10,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^object/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^cxx_object_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^object/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?cxx_object_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^object/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?cxx_object_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^object$",
            "source": "^object$",
            "install": {
                "prefix": "^(/usr/local|[A-Za-z]:.*/codemodel-v2)$",
                "destinations": [
                    {
                        "path": "bin",
                        "backtrace": [
                            {
                                "file": "^object/CMakeLists\\.txt$",
                                "line": 13,
                                "command": "install",
                                "hasParent": True,
                            },
                            {
                                "file": "^object/CMakeLists\\.txt$",
                                "line": None,
                                "command": None,
                                "hasParent": False,
                            },
                        ],
                    },
                ],
            },
            "link": {
                "language": "CXX",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^cxx_object_lib::@5ed5358f70faf8d8af7a$",
                    # FIXME: Add a backtrace here when it becomes available.
                    # You'll know when it's available, because this test will
                    # fail.
                    "backtrace": None,
                },
                {
                    "id": "^ZERO_CHECK::@5ed5358f70faf8d8af7a$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ALL_BUILD",
            "id": "^ALL_BUILD::@ba7eb709d0b48779c6c8$",
            "directorySource": "^imported$",
            "projectName": "Imported",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/imported/CMakeFiles/ALL_BUILD$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/imported/CMakeFiles/ALL_BUILD\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/imported/CMakeFiles/ALL_BUILD$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/imported/CMakeFiles/ALL_BUILD\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^imported$",
            "source": "^imported$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
                {
                    "id": "^link_imported_exe::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
                {
                    "id": "^link_imported_shared_exe::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
                {
                    "id": "^link_imported_static_exe::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
                {
                    "id": "^link_imported_object_exe::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
                {
                    "id": "^link_imported_interface_exe::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ZERO_CHECK",
            "id": "^ZERO_CHECK::@ba7eb709d0b48779c6c8$",
            "directorySource": "^imported$",
            "projectName": "Imported",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/imported/CMakeFiles/ZERO_CHECK$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/imported/CMakeFiles/ZERO_CHECK\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/imported/CMakeFiles/ZERO_CHECK$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/imported/CMakeFiles/ZERO_CHECK\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^imported$",
            "source": "^imported$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": None,
        },
        {
            "name": "link_imported_exe",
            "id": "^link_imported_exe::@ba7eb709d0b48779c6c8$",
            "directorySource": "^imported$",
            "projectName": "Imported",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": 5,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": 5,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^link_imported_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^imported/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?link_imported_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^imported/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?link_imported_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^imported$",
            "source": "^imported$",
            "install": None,
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "link_imported_shared_exe",
            "id": "^link_imported_shared_exe::@ba7eb709d0b48779c6c8$",
            "directorySource": "^imported$",
            "projectName": "Imported",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": 9,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": 9,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^link_imported_shared_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^imported/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?link_imported_shared_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^imported/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?link_imported_shared_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^imported$",
            "source": "^imported$",
            "install": None,
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "link_imported_static_exe",
            "id": "^link_imported_static_exe::@ba7eb709d0b48779c6c8$",
            "directorySource": "^imported$",
            "projectName": "Imported",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": 13,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": 13,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^link_imported_static_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^imported/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?link_imported_static_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^imported/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?link_imported_static_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^imported$",
            "source": "^imported$",
            "install": None,
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "link_imported_object_exe",
            "id": "^link_imported_object_exe::@ba7eb709d0b48779c6c8$",
            "directorySource": "^imported$",
            "projectName": "Imported",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": 18,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": 18,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^link_imported_object_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^imported/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?link_imported_object_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^imported/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?link_imported_object_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^imported$",
            "source": "^imported$",
            "install": None,
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "link_imported_interface_exe",
            "id": "^link_imported_interface_exe::@ba7eb709d0b48779c6c8$",
            "directorySource": "^imported$",
            "projectName": "Imported",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": 23,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^imported/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": 23,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^imported/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^link_imported_interface_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^imported/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?link_imported_interface_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^imported/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?link_imported_interface_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^imported$",
            "source": "^imported$",
            "install": None,
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@ba7eb709d0b48779c6c8$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ALL_BUILD",
            "id": "^ALL_BUILD::@c11385ffed57b860da63$",
            "directorySource": "^custom$",
            "projectName": "Custom",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/custom/CMakeFiles/ALL_BUILD$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/custom/CMakeFiles/ALL_BUILD\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/custom/CMakeFiles/ALL_BUILD$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/custom/CMakeFiles/ALL_BUILD\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^custom/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^custom$",
            "source": "^custom$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@c11385ffed57b860da63$",
                    "backtrace": None,
                },
                {
                    "id": "^custom_exe::@c11385ffed57b860da63$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ZERO_CHECK",
            "id": "^ZERO_CHECK::@c11385ffed57b860da63$",
            "directorySource": "^custom$",
            "projectName": "Custom",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/custom/CMakeFiles/ZERO_CHECK$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/custom/CMakeFiles/ZERO_CHECK\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/custom/CMakeFiles/ZERO_CHECK$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/custom/CMakeFiles/ZERO_CHECK\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^custom/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^custom$",
            "source": "^custom$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": None,
        },
        {
            "name": "custom_tgt",
            "id": "^custom_tgt::@c11385ffed57b860da63$",
            "directorySource": "^custom$",
            "projectName": "Custom",
            "type": "UTILITY",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/custom/CMakeFiles/custom_tgt$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": 3,
                            "command": "add_custom_target",
                            "hasParent": True,
                        },
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/(custom/)?CMakeFiles/([0-9a-f]+/)?custom_tgt\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/custom/CMakeFiles/custom_tgt$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/(custom/)?CMakeFiles/([0-9a-f]+/)?custom_tgt\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^custom/CMakeLists\\.txt$",
                    "line": 3,
                    "command": "add_custom_target",
                    "hasParent": True,
                },
                {
                    "file": "^custom/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^custom$",
            "source": "^custom$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@c11385ffed57b860da63$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "custom_exe",
            "id": "^custom_exe::@c11385ffed57b860da63$",
            "directorySource": "^custom$",
            "projectName": "Custom",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": 4,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^empty\\.c$",
                    ],
                    "includes": None,
                    "defines": None,
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^custom/CMakeLists\\.txt$",
                    "line": 4,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^custom/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^custom_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^custom/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?custom_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^custom/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?custom_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^custom$",
            "source": "^custom$",
            "install": None,
            "link": {
                "language": "C",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^custom_tgt::@c11385ffed57b860da63$",
                    "backtrace": [
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": 5,
                            "command": "add_dependencies",
                            "hasParent": True,
                        },
                        {
                            "file": "^custom/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "id": "^ZERO_CHECK::@c11385ffed57b860da63$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ALL_BUILD",
            "id": "^ALL_BUILD::@[0-9a-f]+$",
            "directorySource": "^.*/Tests/RunCMake/FileAPIExternalSource$",
            "projectName": "External",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/CMakeFiles/ALL_BUILD$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/CMakeFiles/ALL_BUILD\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/CMakeFiles/ALL_BUILD$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/CMakeFiles/ALL_BUILD\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild$",
            "source": "^.*/Tests/RunCMake/FileAPIExternalSource$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@[0-9a-f]+$",
                    "backtrace": None,
                },
                {
                    "id": "^generated_exe::@[0-9a-f]+$",
                    "backtrace": None,
                },
            ],
        },
        {
            "name": "ZERO_CHECK",
            "id": "^ZERO_CHECK::@[0-9a-f]+$",
            "directorySource": "^.*/Tests/RunCMake/FileAPIExternalSource$",
            "projectName": "External",
            "type": "UTILITY",
            "isGeneratorProvided": True,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/CMakeFiles/ZERO_CHECK$",
                    "isGenerated": True,
                    "sourceGroupName": "",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/CMakeFiles/ZERO_CHECK\\.rule$",
                    "isGenerated": True,
                    "sourceGroupName": "CMake Rules",
                    "compileGroupLanguage": None,
                    "backtrace": [
                        {
                            "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/CMakeFiles/ZERO_CHECK$",
                    ],
                },
                {
                    "name": "CMake Rules",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/CMakeFiles/ZERO_CHECK\\.rule$",
                    ],
                },
            ],
            "compileGroups": None,
            "backtrace": [
                {
                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": None,
            "artifacts": None,
            "build": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild$",
            "source": "^.*/Tests/RunCMake/FileAPIExternalSource$",
            "install": None,
            "link": None,
            "archive": None,
            "dependencies": None,
        },
        {
            "name": "generated_exe",
            "id": "^generated_exe::@[0-9a-f]+$",
            "directorySource": "^.*/Tests/RunCMake/FileAPIExternalSource$",
            "projectName": "External",
            "type": "EXECUTABLE",
            "isGeneratorProvided": None,
            "sources": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPIExternalSource/empty\\.c$",
                    "isGenerated": None,
                    "sourceGroupName": "Source Files",
                    "compileGroupLanguage": "C",
                    "backtrace": [
                        {
                            "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                            "line": 5,
                            "command": "add_executable",
                            "hasParent": True,
                        },
                        {
                            "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/generated\\.cxx$",
                    "isGenerated": True,
                    "sourceGroupName": "Generated Source Files",
                    "compileGroupLanguage": "CXX",
                    "backtrace": [
                        {
                            "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                            "line": 6,
                            "command": "target_sources",
                            "hasParent": True,
                        },
                        {
                            "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                            "line": None,
                            "command": None,
                            "hasParent": False,
                        },
                    ],
                },
            ],
            "sourceGroups": [
                {
                    "name": "Source Files",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPIExternalSource/empty\\.c$",
                    ],
                },
                {
                    "name": "Generated Source Files",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/generated\\.cxx$",
                    ],
                },
            ],
            "compileGroups": [
                {
                    "language": "C",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPIExternalSource/empty\\.c$",
                    ],
                    "includes": [
                        {
                            "path": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild$",
                            "isSystem": None,
                            "backtrace": [
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": 10,
                                    "command": "set_property",
                                    "hasParent": True,
                                },
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        },
                        {
                            "path": "^.*/Tests/RunCMake/FileAPIExternalSource$",
                            "isSystem": True,
                            "backtrace": [
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": 11,
                                    "command": "target_include_directories",
                                    "hasParent": True,
                                },
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        },
                    ],
                    "defines": [
                        {
                            "define": "EMPTY_C=1",
                            "backtrace": [
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": 9,
                                    "command": "set_property",
                                    "hasParent": True,
                                },
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        },
                        {
                            "define": "SRC_DUMMY",
                            "backtrace": [
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": 9,
                                    "command": "set_property",
                                    "hasParent": True,
                                },
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        },
                        {
                            "define": "GENERATED_EXE=1",
                            "backtrace": [
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": 12,
                                    "command": "target_compile_definitions",
                                    "hasParent": True,
                                },
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        },
                        {
                            "define": "TGT_DUMMY",
                            "backtrace": [
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": 12,
                                    "command": "target_compile_definitions",
                                    "hasParent": True,
                                },
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        },
                    ],
                    "compileCommandFragments": [
                        {
                            "fragment" : "SRC_COMPILE_OPTIONS_DUMMY",
                            "backtrace": [
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": 13,
                                    "command": "set_source_files_properties",
                                    "hasParent": True,
                                },
                                {
                                    "file" : "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        }
                    ],
                },
                {
                    "language": "CXX",
                    "sourcePaths": [
                        "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/generated\\.cxx$",
                    ],
                    "includes": [
                        {
                            "path": "^.*/Tests/RunCMake/FileAPIExternalSource$",
                            "isSystem": True,
                            "backtrace": [
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": 11,
                                    "command": "target_include_directories",
                                    "hasParent": True,
                                },
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        },
                    ],
                    "defines": [
                        {
                            "define": "GENERATED_EXE=1",
                            "backtrace": [
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": 12,
                                    "command": "target_compile_definitions",
                                    "hasParent": True,
                                },
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        },
                        {
                            "define": "TGT_DUMMY",
                            "backtrace": [
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": 12,
                                    "command": "target_compile_definitions",
                                    "hasParent": True,
                                },
                                {
                                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        },
                    ],
                    "compileCommandFragments": None,
                },
            ],
            "backtrace": [
                {
                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                    "line": 5,
                    "command": "add_executable",
                    "hasParent": True,
                },
                {
                    "file": "^.*/Tests/RunCMake/FileAPIExternalSource/CMakeLists\\.txt$",
                    "line": None,
                    "command": None,
                    "hasParent": False,
                },
            ],
            "folder": None,
            "nameOnDisk": "^generated_exe(\\.exe)?$",
            "artifacts": [
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?generated_exe(\\.exe)?$",
                    "_dllExtra": False,
                },
                {
                    "path": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild/((Debug|Release|RelWithDebInfo|MinSizeRel)/)?generated_exe\\.pdb$",
                    "_dllExtra": True,
                },
            ],
            "build": "^.*/Tests/RunCMake/FileAPI/FileAPIExternalBuild$",
            "source": "^.*/Tests/RunCMake/FileAPIExternalSource$",
            "install": None,
            "link": {
                "language": "CXX",
                "lto": None,
                "commandFragments": None,
            },
            "archive": None,
            "dependencies": [
                {
                    "id": "^ZERO_CHECK::@[0-9a-f]+$",
                    "backtrace": None,
                },
            ],
        },
    ]

    if not os.path.exists(os.path.join(reply_dir, "..", "..", "..", "..", "ipo_enabled.txt")):
        for e in expected:
            try:
                e["link"]["lto"] = None
            except TypeError:  # "link" is not a dict, no problem.
                pass
            try:
                e["archive"]["lto"] = None
            except TypeError:  # "archive" is not a dict, no problem.
                pass

    if inSource:
        for e in expected:
            if e["sources"] is not None:
                for s in e["sources"]:
                    s["path"] = s["path"].replace("^.*/Tests/RunCMake/FileAPI/", "^", 1)
            if e["sourceGroups"] is not None:
                for group in e["sourceGroups"]:
                    group["sourcePaths"] = [p.replace("^.*/Tests/RunCMake/FileAPI/", "^", 1) for p in group["sourcePaths"]]
            if e["compileGroups"] is not None:
                for group in e["compileGroups"]:
                    group["sourcePaths"] = [p.replace("^.*/Tests/RunCMake/FileAPI/", "^", 1) for p in group["sourcePaths"]]

    if matches(g["name"], "^Visual Studio "):
        expected = filter_list(lambda e: e["name"] not in ("ZERO_CHECK") or e["id"] == "^ZERO_CHECK::@6890427a1f51a3e7e1df$", expected)
        for e in expected:
            if e["type"] == "UTILITY":
                if e["id"] == "^ZERO_CHECK::@6890427a1f51a3e7e1df$":
                    e["sources"] = [
                        {
                            "path": "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/CMakeFiles/([0-9a-f]+/)?generate\\.stamp\\.rule$",
                            "isGenerated": True,
                            "sourceGroupName": "CMake Rules",
                            "compileGroupLanguage": None,
                            "backtrace": [
                                {
                                    "file": "^CMakeLists\\.txt$",
                                    "line": None,
                                    "command": None,
                                    "hasParent": False,
                                },
                            ],
                        },
                    ]
                    e["sourceGroups"] = [
                        {
                            "name": "CMake Rules",
                            "sourcePaths": [
                                "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build/CMakeFiles/([0-9a-f]+/)?generate\\.stamp\\.rule$",
                            ],
                        },
                    ]
                elif e["name"] in ("ALL_BUILD"):
                    e["sources"] = []
                    e["sourceGroups"] = None
            if e["dependencies"] is not None:
                for d in e["dependencies"]:
                    if matches(d["id"], "^\\^ZERO_CHECK::@"):
                        d["id"] = "^ZERO_CHECK::@6890427a1f51a3e7e1df$"

    elif g["name"] == "Xcode":
        if ';' in os.environ.get("CMAKE_OSX_ARCHITECTURES", ""):
            expected = filter_list(lambda e: e["name"] not in ("link_imported_object_exe"), expected)
            for e in expected:
                e["dependencies"] = filter_list(lambda d: not matches(d["id"], "^\\^link_imported_object_exe::@"), e["dependencies"])
                if e["name"] in ("c_object_lib", "cxx_object_lib"):
                    e["artifacts"] = None

    else:
        for e in expected:
            e["dependencies"] = filter_list(lambda d: not matches(d["id"], "^\\^ZERO_CHECK::@"), e["dependencies"])

        expected = filter_list(lambda t: t["name"] not in ("ALL_BUILD", "ZERO_CHECK"), expected)

    if sys.platform not in ("win32", "cygwin", "msys"):
        for e in expected:
            e["artifacts"] = filter_list(lambda a: not a["_dllExtra"], e["artifacts"])

    if "aix" not in sys.platform:
        for e in expected:
            e["artifacts"] = filter_list(lambda a: not a.get("_aixExtra", False), e["artifacts"])

    return expected

def check_targets(c, g, inSource):
    check_list_match(lambda a, e: matches(a["id"], e["id"]),
                     c["targets"], gen_check_targets(c, g, inSource),
                     check=check_target(c),
                     check_exception=lambda a, e: "Target ID: %s" % a["id"],
                     missing_exception=lambda e: "Target ID: %s" % e["id"],
                     extra_exception=lambda a: "Target ID: %s" % a["id"])

def gen_check_projects(c, g):
    expected = [
        {
            "name": "codemodel-v2",
            "parentName": None,
            "childNames": [
                "Alias",
                "Custom",
                "Cxx",
                "Imported",
                "Object",
                "External",
            ],
            "directorySources": [
                "^\\.$",
                "^dir$",
                "^dir/dir$",
            ],
            "targetIds": [
                "^ALL_BUILD::@6890427a1f51a3e7e1df$",
                "^ZERO_CHECK::@6890427a1f51a3e7e1df$",
                "^interface_exe::@6890427a1f51a3e7e1df$",
                "^c_lib::@6890427a1f51a3e7e1df$",
                "^c_exe::@6890427a1f51a3e7e1df$",
                "^c_shared_lib::@6890427a1f51a3e7e1df$",
                "^c_shared_exe::@6890427a1f51a3e7e1df$",
                "^c_static_lib::@6890427a1f51a3e7e1df$",
                "^c_static_exe::@6890427a1f51a3e7e1df$",
            ],
        },
        {
            "name": "Cxx",
            "parentName": "codemodel-v2",
            "childNames": None,
            "directorySources": [
                "^cxx$",
            ],
            "targetIds": [
                "^ALL_BUILD::@a56b12a3f5c0529fb296$",
                "^ZERO_CHECK::@a56b12a3f5c0529fb296$",
                "^cxx_lib::@a56b12a3f5c0529fb296$",
                "^cxx_exe::@a56b12a3f5c0529fb296$",
                "^cxx_shared_lib::@a56b12a3f5c0529fb296$",
                "^cxx_shared_exe::@a56b12a3f5c0529fb296$",
                "^cxx_static_lib::@a56b12a3f5c0529fb296$",
                "^cxx_static_exe::@a56b12a3f5c0529fb296$",
            ],
        },
        {
            "name": "Alias",
            "parentName": "codemodel-v2",
            "childNames": None,
            "directorySources": [
                "^alias$",
            ],
            "targetIds": [
                "^ALL_BUILD::@53632cba2752272bb008$",
                "^ZERO_CHECK::@53632cba2752272bb008$",
                "^c_alias_exe::@53632cba2752272bb008$",
                "^cxx_alias_exe::@53632cba2752272bb008$",
            ],
        },
        {
            "name": "Object",
            "parentName": "codemodel-v2",
            "childNames": None,
            "directorySources": [
                "^object$",
            ],
            "targetIds": [
                "^ALL_BUILD::@5ed5358f70faf8d8af7a$",
                "^ZERO_CHECK::@5ed5358f70faf8d8af7a$",
                "^c_object_lib::@5ed5358f70faf8d8af7a$",
                "^c_object_exe::@5ed5358f70faf8d8af7a$",
                "^cxx_object_lib::@5ed5358f70faf8d8af7a$",
                "^cxx_object_exe::@5ed5358f70faf8d8af7a$",
            ],
        },
        {
            "name": "Imported",
            "parentName": "codemodel-v2",
            "childNames": None,
            "directorySources": [
                "^imported$",
            ],
            "targetIds": [
                "^ALL_BUILD::@ba7eb709d0b48779c6c8$",
                "^ZERO_CHECK::@ba7eb709d0b48779c6c8$",
                "^link_imported_exe::@ba7eb709d0b48779c6c8$",
                "^link_imported_shared_exe::@ba7eb709d0b48779c6c8$",
                "^link_imported_static_exe::@ba7eb709d0b48779c6c8$",
                "^link_imported_object_exe::@ba7eb709d0b48779c6c8$",
                "^link_imported_interface_exe::@ba7eb709d0b48779c6c8$",
            ],
        },
        {
            "name": "Custom",
            "parentName": "codemodel-v2",
            "childNames": None,
            "directorySources": [
                "^custom$",
            ],
            "targetIds": [
                "^ALL_BUILD::@c11385ffed57b860da63$",
                "^ZERO_CHECK::@c11385ffed57b860da63$",
                "^custom_tgt::@c11385ffed57b860da63$",
                "^custom_exe::@c11385ffed57b860da63$",
            ],
        },
        {
            "name": "External",
            "parentName": "codemodel-v2",
            "childNames": None,
            "directorySources": [
                "^.*/Tests/RunCMake/FileAPIExternalSource$",
            ],
            "targetIds": [
                "^ALL_BUILD::@[0-9a-f]+$",
                "^ZERO_CHECK::@[0-9a-f]+$",
                "^generated_exe::@[0-9a-f]+$",
            ],
        },
    ]

    if matches(g["name"], "^Visual Studio "):
        for e in expected:
            if e["parentName"] is not None:
                e["targetIds"] = filter_list(lambda t: not matches(t, "^\\^ZERO_CHECK"), e["targetIds"])

    elif g["name"] == "Xcode":
        if ';' in os.environ.get("CMAKE_OSX_ARCHITECTURES", ""):
            for e in expected:
                e["targetIds"] = filter_list(lambda t: not matches(t, "^\\^(link_imported_object_exe)"), e["targetIds"])

    else:
        for e in expected:
            e["targetIds"] = filter_list(lambda t: not matches(t, "^\\^(ALL_BUILD|ZERO_CHECK)"), e["targetIds"])

    return expected

def check_projects(c, g):
    check_list_match(lambda a, e: is_string(a["name"], e["name"]), c["projects"], gen_check_projects(c, g),
                     check=check_project(c),
                     check_exception=lambda a, e: "Project name: %s" % a["name"],
                     missing_exception=lambda e: "Project name: %s" % e["name"],
                     extra_exception=lambda a: "Project name: %s" % a["name"])

def check_object_codemodel_configuration(c, g, inSource):
    assert sorted(c.keys()) == ["directories", "name", "projects", "targets"]
    assert is_string(c["name"])
    check_directories(c, g)
    check_targets(c, g, inSource)
    check_projects(c, g)

def check_object_codemodel(g):
    def _check(o):
        assert sorted(o.keys()) == ["configurations", "kind", "paths", "version"]
        # The "kind" and "version" members are handled by check_index_object.
        assert is_dict(o["paths"])
        assert sorted(o["paths"].keys()) == ["build", "source"]
        assert matches(o["paths"]["build"], "^.*/Tests/RunCMake/FileAPI/codemodel-v2-build$")
        assert matches(o["paths"]["source"], "^.*/Tests/RunCMake/FileAPI$")

        inSource = os.path.dirname(o["paths"]["build"]) == o["paths"]["source"]

        if g["multiConfig"]:
            assert sorted([c["name"] for c in o["configurations"]]) == ["Debug", "MinSizeRel", "RelWithDebInfo", "Release"]
        else:
            assert len(o["configurations"]) == 1
            assert o["configurations"][0]["name"] in ("", "Debug", "Release", "RelWithDebInfo", "MinSizeRel")

        for c in o["configurations"]:
            check_object_codemodel_configuration(c, g, inSource)
    return _check

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_objects(index["objects"], index["cmake"]["generator"])
