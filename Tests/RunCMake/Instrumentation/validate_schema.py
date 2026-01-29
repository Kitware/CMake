import json
import jsonschema
import sys

with open(sys.argv[1], "r", encoding="utf-8-sig") as f:
    contents = json.load(f)

with open(sys.argv[2], "r", encoding="utf-8") as f:
    schema = json.load(f)

try:
    jsonschema.validate(contents, schema)
except jsonschema.ValidationError as e:
    print(e)
    sys.exit(1)
