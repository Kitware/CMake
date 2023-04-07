import json
import jsonschema
import os.path
import sys


with open(sys.argv[1], "r", encoding="utf-8-sig") as f:
    contents = json.load(f)

schema_file = os.path.join(
        os.path.dirname(__file__),
        "..", "..", "..", "Help", "manual", "presets", "schema.json")
with open(schema_file, "r", encoding="utf-8") as f:
    schema = json.load(f)

jsonschema.validate(contents, schema)
