from check_index import *

import json
import os

def read_json(filename):
    with open(os.path.join(reply_dir, filename)) as f:
        return json.load(f)

def check_target_id(target_ids, std_target_ids, state, target_file, context, target_id):
    assert target_id in target_ids, \
        "Target '%s' has dangling %s target id '%s'." % (target_file, context, target_id)

    if target_id in std_target_ids:
        state["referenced_std_target"] = True

def collect_target_ids(codemodel):
    target_ids = set()
    target_json_files = set()
    std_target_ids = set()
    seen_main_target = False
    seen_std_target = False

    assert is_list(codemodel["configurations"])
    assert len(codemodel["configurations"]) > 0

    for configuration in codemodel["configurations"]:
        for target_list_name in ("targets", "abstractTargets"):
            for target in configuration[target_list_name]:
                assert is_dict(target)
                target_id = target["id"]
                target_ids.add(target_id)
                target_json_files.add(target["jsonFile"])

                if target["name"] == "main":
                    seen_main_target = True
                if target["name"] == "@cmake_cxx_std":
                    seen_std_target = True
                    std_target_ids.add(target_id)

    assert seen_main_target, "The File API codemodel did not report the user target 'main'."
    assert seen_std_target, \
        "The File API codemodel did not report the import std target '@cmake_cxx_std'."

    return target_ids, target_json_files, std_target_ids

def check_target_references(codemodel):
    target_ids, target_json_files, std_target_ids = collect_target_ids(codemodel)
    target_id_reference_fields = (
        "dependencies",
        "compileDependencies",
        "interfaceCompileDependencies",
        "objectDependencies",
        "orderDependencies",
    )
    link_reference_fields = (
        "linkLibraries",
        "interfaceLinkLibraries",
    )
    state = {
        "referenced_std_target": False,
    }

    for target_json_file in sorted(target_json_files):
        target = read_json(target_json_file)
        for target_reference_field in target_id_reference_fields:
            if target_reference_field not in target:
                continue

            assert is_list(target[target_reference_field])
            for reference in target[target_reference_field]:
                assert is_dict(reference)
                check_target_id(
                    target_ids,
                    std_target_ids,
                    state,
                    target_json_file,
                    target_reference_field,
                    reference["id"])

                if "fromDependency" in reference:
                    assert is_dict(reference["fromDependency"])
                    assert sorted(reference["fromDependency"].keys()) == ["id"]
                    check_target_id(
                        target_ids,
                        std_target_ids,
                        state,
                        target_json_file,
                        target_reference_field + ".fromDependency",
                        reference["fromDependency"]["id"])

        for target_reference_field in link_reference_fields:
            if target_reference_field not in target:
                continue

            assert is_list(target[target_reference_field])
            for reference in target[target_reference_field]:
                assert is_dict(reference)
                has_id = "id" in reference
                has_fragment = "fragment" in reference
                assert has_id != has_fragment

                if has_id:
                    check_target_id(
                        target_ids,
                        std_target_ids,
                        state,
                        target_json_file,
                        target_reference_field,
                        reference["id"])

                if "fromDependency" in reference:
                    assert is_dict(reference["fromDependency"])
                    assert sorted(reference["fromDependency"].keys()) == ["id"]
                    check_target_id(
                        target_ids,
                        std_target_ids,
                        state,
                        target_json_file,
                        target_reference_field + ".fromDependency",
                        reference["fromDependency"]["id"])

    assert state["referenced_std_target"], \
        "The File API codemodel did not reference '@cmake_cxx_std' from any target relationship."

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
assert is_list(index["objects"])

codemodel_index = None
for index_object in index["objects"]:
    if index_object["kind"] == "codemodel":
        codemodel_index = index_object
        break

assert codemodel_index is not None, "No codemodel object found in File API index."
assert codemodel_index["version"]["major"] == 2

codemodel = read_json(codemodel_index["jsonFile"])
check_target_references(codemodel)
