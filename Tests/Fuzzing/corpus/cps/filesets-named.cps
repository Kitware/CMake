{
  "cps_version": "0.15.0",
  "name": "FileSets-Named",
  "cps_path": "@prefix@/cps",
  "components": {
    "Target": {
      "file_sets":
      [
        {
          "extensions":
          {
            "cmake":
            {
              "name@v1": "foo"
            }
          },
          "type": "includes",
          "root": "@prefix@/include/a",
          "files": [ "foo1.h", "foo2.h" ]
        },
        {
          "extensions":
          {
            "cmake":
            {
              "name@v1": "foo"
            }
          },
          "type": "includes",
          "root": "@prefix@/include/b",
          "files": [ "foo3.h" ]
        },
        {
          "extensions":
          {
            "cmake":
            {
              "name@v1": "bar"
            }
          },
          "type": "includes",
          "root": "@prefix@/include",
          "files": [ "bar.h" ]
        }
      ],
      "type": "interface"
    }
  }
}
