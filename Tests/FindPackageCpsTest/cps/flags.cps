{
  "cps_version": "0.13.0",
  "name": "Flags",
  "cps_path": "@prefix@/cps",
  "components": {
    "plain": {
      "type": "archive",
      "location": "@prefix@/lib/libfoo.a",
      "compile_flags": [ "-compile-flag", "-second-compile-flag" ],
      "link_flags": [ "-link-flag", "-second-link-flag" ]
    },
    "languages": {
      "type": "interface",
      "compile_flags": {
        "*": [ "-DFLAGS_ALL_LANGUAGES" ],
        "c": [ "-DFLAGS_ONLY_IN_C" ],
        "cpp": [ "-DFLAGS_ONLY_IN_CXX" ]
      }
    },
    "configurations": {
      "type": "interface",
      "compile_flags": [ "-base-compile" ],
      "link_flags": [ "-base-link" ],
      "configurations": {
        "Debug": {
          "compile_flags": [ "-debug-compile" ],
          "link_flags": [ "-debug-link" ]
        }
      }
    },
    "configured_languages": {
      "type": "interface",
      "configurations": {
        "Debug": {
          "compile_flags": {
            "cpp": [ "-debug-cxx" ]
          }
        }
      }
    }
  }
}
