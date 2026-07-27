import argparse
import json
import pathlib
import sys

import jsonschema


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description="Validate CMake JSON schemas")
    parser.add_argument(
        "schema",
        type=pathlib.Path,
        help="the path to the schema file",
    )
    parser.add_argument(
        "--input-files",
        nargs="+",
        type=pathlib.Path,
        help="the path to an input file to validate",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="enable verbose output",
    )
    args = parser.parse_args(argv)

    with open(args.schema, "r", encoding="utf-8") as f:
        schema = json.load(f)

    for file in args.input_files:
        try:
            with open(file, "r", encoding="utf-8-sig") as f:
                contents = json.load(f)
            jsonschema.validate(contents, schema)
        except jsonschema.ValidationError as e:
            message = str(e) if args.verbose else str(e.message)
            print(f"Failed to validate file {file}: {message}", file=sys.stderr)
            return 1
        except jsonschema.SchemaError as e:
            message = str(e) if args.verbose else str(e.message)
            print(f"Failed to validate schema: {message}", file=sys.stderr)
            return 2
        except Exception as e:
            print(f"Unknown exception: {e}", file=sys.stderr)
            return 3

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
