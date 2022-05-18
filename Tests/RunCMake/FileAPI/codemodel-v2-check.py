from check_index import *

import json
import sys
import os

def read_codemodel_json_data(filename):
    abs_filename = os.path.join(os.path.dirname(os.path.realpath(__file__)), "codemodel-v2-data", filename)
    with open(abs_filename, "r") as f:
        return json.load(f)

def check_objects(o, g):
    assert is_list(o)
    assert len(o) == 1
    check_index_object(o[0], "codemodel", 2, 4, check_object_codemodel(g))

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

def check_backtraces(t, actual, expected):
    assert is_list(actual)
    assert is_list(expected)
    assert len(actual) == len(expected)

    i = 0
    while i < len(actual):
        check_backtrace(t, actual[i], expected[i])
        i += 1

def check_directory(c):
    def _check(actual, expected):
        assert is_dict(actual)
        expected_keys = ["build", "jsonFile", "source", "projectIndex"]
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

        assert is_string(actual["jsonFile"])
        filepath = os.path.join(reply_dir, actual["jsonFile"])
        with open(filepath) as f:
            d = json.load(f)

        assert is_dict(d)
        assert sorted(d.keys()) == ["backtraceGraph", "installers", "paths"]

        assert is_string(d["paths"]["source"], actual["source"])
        assert is_string(d["paths"]["build"], actual["build"])

        check_backtrace_graph(d["backtraceGraph"])

        assert is_list(d["installers"])
        assert len(d["installers"]) == len(expected["installers"])
        for a, e in zip(d["installers"], expected["installers"]):
            assert is_dict(a)
            expected_keys = ["component", "type"]

            assert is_string(a["component"], e["component"])
            assert is_string(a["type"], e["type"])

            if e["destination"] is not None:
                expected_keys.append("destination")
                assert is_string(a["destination"], e["destination"])

            if e["paths"] is not None:
                expected_keys.append("paths")
                assert is_list(a["paths"])
                assert len(a["paths"]) == len(e["paths"])

                for ap, ep in zip(a["paths"], e["paths"]):
                    if is_string(ep):
                        assert matches(ap, ep)
                    else:
                        assert is_dict(ap)
                        assert sorted(ap.keys()) == ["from", "to"]
                        assert matches(ap["from"], ep["from"])
                        assert matches(ap["to"], ep["to"])

            if e["isExcludeFromAll"] is not None:
                expected_keys.append("isExcludeFromAll")
                assert is_bool(a["isExcludeFromAll"], e["isExcludeFromAll"])

            if e["isForAllComponents"] is not None:
                expected_keys.append("isForAllComponents")
                assert is_bool(a["isForAllComponents"], e["isForAllComponents"])

            if e["isOptional"] is not None:
                expected_keys.append("isOptional")
                assert is_bool(a["isOptional"], e["isOptional"])

            if e["targetId"] is not None:
                expected_keys.append("targetId")
                assert matches(a["targetId"], e["targetId"])

            if e["targetIndex"] is not None:
                expected_keys.append("targetIndex")
                assert is_int(a["targetIndex"])
                assert c["targets"][a["targetIndex"]]["name"] == e["targetIndex"]

            if e["targetIsImportLibrary"] is not None:
                expected_keys.append("targetIsImportLibrary")
                assert is_bool(a["targetIsImportLibrary"], e["targetIsImportLibrary"])

            if e["targetInstallNamelink"] is not None:
                expected_keys.append("targetInstallNamelink")
                assert is_string(a["targetInstallNamelink"], e["targetInstallNamelink"])

            if e["exportName"] is not None:
                expected_keys.append("exportName")
                assert is_string(a["exportName"], e["exportName"])

            if e["exportTargets"] is not None:
                expected_keys.append("exportTargets")
                assert is_list(a["exportTargets"])
                assert len(a["exportTargets"]) == len(e["exportTargets"])
                for at, et in zip(a["exportTargets"], e["exportTargets"]):
                    assert is_dict(at)
                    assert sorted(at.keys()) == ["id", "index"]
                    assert matches(at["id"], et["id"])
                    assert is_int(at["index"])
                    assert c["targets"][at["index"]]["name"] == et["index"]

            if e["scriptFile"] is not None:
                expected_keys.append("scriptFile")
                assert is_string(a["scriptFile"], e["scriptFile"])

            if e.get("runtimeDependencySetName", None) is not None:
                expected_keys.append("runtimeDependencySetName")
                assert is_string(a["runtimeDependencySetName"], e["runtimeDependencySetName"])

            if e.get("runtimeDependencySetType", None) is not None:
                expected_keys.append("runtimeDependencySetType")
                assert is_string(a["runtimeDependencySetType"], e["runtimeDependencySetType"])

            if e.get("fileSetName", None) is not None:
                expected_keys.append("fileSetName")
                assert is_string(a["fileSetName"], e["fileSetName"])

            if e.get("fileSetType", None) is not None:
                expected_keys.append("fileSetType")
                assert is_string(a["fileSetType"], e["fileSetType"])

            if e.get("fileSetDirectories", None) is not None:
                expected_keys.append("fileSetDirectories")
                assert is_list(a["fileSetDirectories"])
                assert len(a["fileSetDirectories"]) == len(e["fileSetDirectories"])
                for ad, ed in zip(a["fileSetDirectories"], e["fileSetDirectories"]):
                    assert matches(ad, ed)

            if e.get("fileSetTarget", None) is not None:
                expected_keys.append("fileSetTarget")
                et = e["fileSetTarget"]
                at = a["fileSetTarget"]
                assert is_dict(at)
                assert sorted(at.keys()) == ["id", "index"]
                assert matches(at["id"], et["id"])
                assert is_int(at["index"])
                assert c["targets"][at["index"]]["name"] == et["index"]

            if e["backtrace"] is not None:
                expected_keys.append("backtrace")
                check_backtrace(d, a["backtrace"], e["backtrace"])

            assert sorted(a.keys()) == sorted(expected_keys)

    return _check

def check_backtrace_graph(btg):
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
        check_backtrace_graph(obj["backtraceGraph"])

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

                if "precompileHeaders" in expected:
                    expected_keys.append("precompileHeaders")

                    def check_precompile_header(actual, expected):
                        assert is_dict(actual)
                        expected_keys = ["backtrace", "header"]
                        check_backtrace(obj, actual["backtrace"], expected["backtrace"])

                        assert sorted(actual.keys()) == sorted(expected_keys)

                    check_list_match(lambda a, e: matches(a["header"], e["header"]),
                                     actual["precompileHeaders"], expected["precompileHeaders"],
                                     check=check_precompile_header,
                                     check_exception=lambda a, e: "Precompile header: %s" % a["header"],
                                     missing_exception=lambda e: "Precompile header: %s" % e["header"],
                                     extra_exception=lambda a: "Precompile header: %s" % a["header"])

                if "languageStandard" in expected:
                    expected_keys.append("languageStandard")

                    def check_language_standard(actual, expected):
                        assert is_dict(actual)
                        expected_keys = ["backtraces", "standard"]
                        assert actual["standard"] == expected["standard"]
                        check_backtraces(obj, actual["backtraces"], expected["backtraces"])

                        assert sorted(actual.keys()) == sorted(expected_keys)

                    check_language_standard(actual["languageStandard"], expected["languageStandard"])

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
        read_codemodel_json_data("directories/top.json"),
        read_codemodel_json_data("directories/alias.json"),
        read_codemodel_json_data("directories/custom.json"),
        read_codemodel_json_data("directories/cxx.json"),
        read_codemodel_json_data("directories/imported.json"),
        read_codemodel_json_data("directories/interface.json"),
        read_codemodel_json_data("directories/object.json"),
        read_codemodel_json_data("directories/dir.json"),
        read_codemodel_json_data("directories/dir_dir.json"),
        read_codemodel_json_data("directories/external.json"),
        read_codemodel_json_data("directories/fileset.json"),
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

    if sys.platform in ("win32", "cygwin", "msys") or "aix" in sys.platform:
        for e in expected:
            e["installers"] = list(filter(lambda i: i["targetInstallNamelink"] is None or i["targetInstallNamelink"] == "skip", e["installers"]))
            for i in e["installers"]:
                i["targetInstallNamelink"] = None

    if sys.platform not in ("win32", "cygwin", "msys"):
        for e in expected:
            e["installers"] = list(filter(lambda i: not i.get("_dllExtra", False), e["installers"]))
            if "aix" not in sys.platform:
                for i in e["installers"]:
                    if "pathsNamelink" in i:
                        i["paths"] = i["pathsNamelink"]

    if sys.platform not in ("win32", "darwin") and "linux" not in sys.platform:
        for e in expected:
            e["installers"] = list(filter(lambda i: i["type"] != "runtimeDependencySet", e["installers"]))

    if sys.platform != "darwin":
        for e in expected:
            e["installers"] = list(filter(lambda i: i.get("runtimeDependencySetType", None) != "framework", e["installers"]))

    return expected

def check_directories(c, g):
    check_list_match(lambda a, e: matches(a["source"], e["source"]), c["directories"], gen_check_directories(c, g),
                     check=check_directory(c),
                     check_exception=lambda a, e: "Directory source: %s" % a["source"],
                     missing_exception=lambda e: "Directory source: %s" % e["source"],
                     extra_exception=lambda a: "Directory source: %s" % a["source"])

def gen_check_targets(c, g, inSource):
    expected = [
        read_codemodel_json_data("targets/all_build_top.json"),
        read_codemodel_json_data("targets/zero_check_top.json"),
        read_codemodel_json_data("targets/interface_exe.json"),
        read_codemodel_json_data("targets/c_lib.json"),
        read_codemodel_json_data("targets/c_exe.json"),
        read_codemodel_json_data("targets/c_shared_lib.json"),
        read_codemodel_json_data("targets/c_shared_exe.json"),
        read_codemodel_json_data("targets/c_static_lib.json"),
        read_codemodel_json_data("targets/c_static_exe.json"),

        read_codemodel_json_data("targets/all_build_cxx.json"),
        read_codemodel_json_data("targets/zero_check_cxx.json"),
        read_codemodel_json_data("targets/cxx_lib.json"),
        read_codemodel_json_data("targets/cxx_exe.json"),
        read_codemodel_json_data("targets/cxx_standard_compile_feature_exe.json"),
        read_codemodel_json_data("targets/cxx_standard_exe.json"),
        read_codemodel_json_data("targets/cxx_shared_lib.json"),
        read_codemodel_json_data("targets/cxx_shared_exe.json"),
        read_codemodel_json_data("targets/cxx_static_lib.json"),
        read_codemodel_json_data("targets/cxx_static_exe.json"),

        read_codemodel_json_data("targets/all_build_alias.json"),
        read_codemodel_json_data("targets/zero_check_alias.json"),
        read_codemodel_json_data("targets/c_alias_exe.json"),
        read_codemodel_json_data("targets/cxx_alias_exe.json"),

        read_codemodel_json_data("targets/all_build_object.json"),
        read_codemodel_json_data("targets/zero_check_object.json"),
        read_codemodel_json_data("targets/c_object_lib.json"),
        read_codemodel_json_data("targets/c_object_exe.json"),
        read_codemodel_json_data("targets/cxx_object_lib.json"),
        read_codemodel_json_data("targets/cxx_object_exe.json"),

        read_codemodel_json_data("targets/all_build_imported.json"),
        read_codemodel_json_data("targets/zero_check_imported.json"),
        read_codemodel_json_data("targets/link_imported_exe.json"),
        read_codemodel_json_data("targets/link_imported_shared_exe.json"),
        read_codemodel_json_data("targets/link_imported_static_exe.json"),
        read_codemodel_json_data("targets/link_imported_object_exe.json"),
        read_codemodel_json_data("targets/link_imported_interface_exe.json"),

        read_codemodel_json_data("targets/all_build_interface.json"),
        read_codemodel_json_data("targets/zero_check_interface.json"),
        read_codemodel_json_data("targets/iface_srcs.json"),

        read_codemodel_json_data("targets/all_build_custom.json"),
        read_codemodel_json_data("targets/zero_check_custom.json"),
        read_codemodel_json_data("targets/custom_tgt.json"),
        read_codemodel_json_data("targets/custom_exe.json"),
        read_codemodel_json_data("targets/all_build_external.json"),
        read_codemodel_json_data("targets/zero_check_external.json"),
        read_codemodel_json_data("targets/generated_exe.json"),

        read_codemodel_json_data("targets/c_headers_1.json"),
        read_codemodel_json_data("targets/c_headers_2.json"),
    ]

    if cxx_compiler_id in ['Clang', 'AppleClang', 'LCC', 'GNU', 'Intel', 'IntelLLVM', 'MSVC', 'Embarcadero', 'IBMClang'] and g["name"] != "Xcode":
        for e in expected:
            if e["name"] == "cxx_exe":
                if matches(g["name"], "^(Visual Studio |Ninja Multi-Config)"):
                    precompile_header_data = read_codemodel_json_data("targets/cxx_exe_precompileheader_multigen.json")
                else:
                    if ';' in os.environ.get("CMAKE_OSX_ARCHITECTURES", ""):
                        precompile_header_data = read_codemodel_json_data("targets/cxx_exe_precompileheader_2arch.json")
                    else:
                        precompile_header_data = read_codemodel_json_data("targets/cxx_exe_precompileheader.json")
                e["compileGroups"] = precompile_header_data["compileGroups"]
                e["sources"] = precompile_header_data["sources"]
                e["sourceGroups"] = precompile_header_data["sourceGroups"]

    if os.path.exists(os.path.join(reply_dir, "..", "..", "..", "..", "cxx", "cxx_std_11.txt")):
        for e in expected:
            if e["name"] == "cxx_standard_compile_feature_exe":
                language_standard_data = read_codemodel_json_data("targets/cxx_standard_compile_feature_exe_languagestandard.json")
                e["compileGroups"][0]["languageStandard"] = language_standard_data["languageStandard"]

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
            if e["install"] is not None:
                e["install"]["destinations"] = filter_list(lambda d: "_dllExtra" not in d or not d["_dllExtra"], e["install"]["destinations"])

    else:
        for e in expected:
            if e["install"] is not None:
                e["install"]["destinations"] = filter_list(lambda d: "_namelink" not in d or not d["_namelink"], e["install"]["destinations"])

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
        read_codemodel_json_data("projects/codemodel-v2.json"),
        read_codemodel_json_data("projects/cxx.json"),
        read_codemodel_json_data("projects/alias.json"),
        read_codemodel_json_data("projects/object.json"),
        read_codemodel_json_data("projects/imported.json"),
        read_codemodel_json_data("projects/interface.json"),
        read_codemodel_json_data("projects/custom.json"),
        read_codemodel_json_data("projects/external.json"),
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

cxx_compiler_id = sys.argv[2]
assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_objects(index["objects"], index["cmake"]["generator"])
