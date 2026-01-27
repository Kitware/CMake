import json
import jsonschema
import sys


# First argument is a file containing the list of files to check
with open(sys.argv[1], "r", encoding="utf-8") as file_list:
    files_to_check = [line.strip() for line in file_list if line.strip()]

# Second argument is the schema file
with open(sys.argv[2], "r", encoding="utf-8-sig") as f:
    schema = json.load(f)

# Check each file against the schema
for file_path in files_to_check:
    try:
        with open(file_path, "r", encoding="utf-8-sig") as f:
            contents = json.load(f)
        # The following raises an exception if validation fails
        jsonschema.validate(contents, schema)
    except Exception as e:
        print(f"Failed to validate file {file_path}: {e}")
        raise
