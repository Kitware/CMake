import json
import jsonschema
import os.path
import sys


with open(sys.argv[1], "rb") as f:
    contents = json.loads(f.read().decode("utf-8-sig"))

schema_file = os.path.join(
        os.path.dirname(__file__),
        "..", "..", "..", "Help", "manual", "presets", "schema.json")
with open(schema_file) as f:
    schema = json.load(f)

jsonschema.validate(contents, schema)
