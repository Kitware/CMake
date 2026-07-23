import sys

from show_only_json_check import *

def check_kind(k):
    assert is_string(k)
    assert k == "ctestInfo"

def check_version(v):
    assert is_dict(v)
    assert sorted(v.keys()) == ["major", "minor"]
    assert is_int(v["major"])
    assert is_int(v["minor"])
    assert v["major"] == 1
    assert v["minor"] == 0

def check_backtracegraph(b):
    assert is_dict(b)
    assert sorted(b.keys()) == ["commands", "files", "nodes"]
    check_backtracegraph_commands(b["commands"])
    check_backtracegraph_files(b["files"])
    check_backtracegraph_nodes(b["nodes"])

def check_backtracegraph_commands(c):
    assert is_list(c)
    assert len(c) == 1
    assert is_string(c[0])
    assert c[0] == "add_test"

def check_backtracegraph_files(f):
    assert is_list(f)
    assert len(f) == 2
    assert is_string(f[0])
    assert is_string(f[1])
    assert f[0] == "file1"
    assert f[1] == "file0"

def check_backtracegraph_nodes(n):
    assert is_list(n)
    assert len(n) == 2
    node = n[0]
    assert is_dict(node)
    assert sorted(node.keys()) == ["file"]
    assert is_int(node["file"])
    assert node["file"] == 1
    node = n[1]
    assert is_dict(node)
    assert sorted(node.keys()) == ["command", "file", "line", "parent"]
    assert is_int(node["command"])
    assert is_int(node["file"])
    assert is_int(node["line"])
    assert is_int(node["parent"])
    assert node["command"] == 0
    assert node["file"] == 0
    assert node["line"] == 1
    assert node["parent"] == 0

def check_command(c):
    assert is_list(c)
    assert len(c) == 3
    assert is_string(c[0])
    check_re(c[0], r"/cmake(\.exe)?$")
    assert is_string(c[1])
    assert c[1] == "-E"
    assert is_string(c[2])
    assert c[2] == "echo"

def check_generated_resource_spec_file_property(p):
    assert is_dict(p)
    assert sorted(p.keys()) == ["name", "value"]
    assert is_string(p["name"])
    assert is_string(p["value"])
    assert p["name"] == "GENERATED_RESOURCE_SPEC_FILE"
    assert p["value"] == "/Path/Does/Not/Exist"

def check_labels_property(p):
    assert is_dict(p)
    assert sorted(p.keys()) == ["name", "value"]
    assert is_string(p["name"])
    assert is_string(p["value"])
    assert p["name"] == "LABELS"
    assert p["value"] == "TestLabel;DirectoryLabel"

def check_reqfiles_property(p):
    assert is_dict(p)
    assert sorted(p.keys()) == ["name", "value"]
    assert is_string(p["name"])
    assert is_string(p["value"])
    assert p["name"] == "REQUIRED_FILES"
    assert p["value"] == \
        "RequiredFileDoesNotExist;AnotherRequiredFileDoesNotExist"

def check_timeout_property(p):
    assert is_dict(p)
    assert sorted(p.keys()) == ["name", "value"]
    assert is_string(p["name"])
    assert is_string(p["value"])
    assert p["name"] == "TIMEOUT"
    assert p["value"] == "1234.5"

def check_timeout_signal_name_property(p):
    assert is_dict(p)
    assert sorted(p.keys()) == ["name", "value"]
    assert is_string(p["name"])
    assert is_string(p["value"])
    assert p["name"] == "TIMEOUT_SIGNAL_NAME"
    assert p["value"] == "SIGINT"

def check_timeout_signal_grace_property(p):
    assert is_dict(p)
    assert sorted(p.keys()) == ["name", "value"]
    assert is_string(p["name"])
    assert is_string(p["value"])
    assert p["name"] == "TIMEOUT_SIGNAL_GRACE_PERIOD"
    assert p["value"] == "2.1"

def check_willfail_property(p):
    assert is_dict(p)
    assert sorted(p.keys()) == ["name", "value"]
    assert is_string(p["name"])
    assert is_string(p["value"])
    assert p["name"] == "WILL_FAIL"
    assert p["value"] == "true"

def check_resource_groups_property(p):
    assert is_dict(p)
    assert sorted(p.keys()) == ["name", "value"]
    assert is_string(p["name"])
    assert is_string(p["value"])
    assert p["name"] == "RESOURCE_GROUPS"
    assert p["value"] == "2,threads:2,gpus:4;gpus:2,threads:4"

def check_workingdir_property(p):
    assert is_dict(p)
    assert sorted(p.keys()) == ["name", "value"]
    assert is_string(p["name"])
    assert is_string(p["value"])
    assert p["name"] == "WORKING_DIRECTORY"
    assert p["value"].endswith("Tests/RunCMake/CTestCommandLine/ShowOnly")

def check_defined_properties(p_list):
    for property_id, p in zip(["A", "B"], p_list):
        assert is_dict(p)
        assert sorted(p.keys()) == ["name", "value"]
        assert is_string(p["name"])
        assert is_string(p["value"])
        assert p["name"] == "USER_DEFINED_" + property_id
        assert p["value"] == "User defined property " + property_id + " value"

def check_properties(p):
    assert is_list(p)
    if sys.platform in ("win32"):
      assert len(p) == 9
      check_generated_resource_spec_file_property(p[0])
      check_labels_property(p[1])
      check_resource_groups_property(p[2])
      check_reqfiles_property(p[3])
      check_timeout_property(p[4])
      check_willfail_property(p[5])
      check_workingdir_property(p[6])
      check_defined_properties(p[7:9])
    else:
      assert len(p) == 11
      check_generated_resource_spec_file_property(p[0])
      check_labels_property(p[1])
      check_resource_groups_property(p[2])
      check_reqfiles_property(p[3])
      check_timeout_property(p[4])
      check_timeout_signal_name_property(p[5])
      check_timeout_signal_grace_property(p[6])
      check_willfail_property(p[7])
      check_workingdir_property(p[8])
      check_defined_properties(p[9:11])

def check_tests(t):
    assert is_list(t)
    assert len(t) == 1
    test = t[0]
    assert is_dict(test)
    assert sorted(test.keys()) == ["backtrace", "command", "name", "properties"]
    assert is_int(test["backtrace"])
    assert test["backtrace"] == 1
    check_command(test["command"])
    assert is_string(test["name"])
    assert test["name"] == "ShowOnly"
    check_properties(test["properties"])

assert is_dict(ctest_json)
assert sorted(ctest_json.keys()) == ["backtraceGraph", "kind", "tests", "version"]
check_backtracegraph(ctest_json["backtraceGraph"])
check_kind(ctest_json["kind"])
check_version(ctest_json["version"])
check_tests(ctest_json["tests"])
