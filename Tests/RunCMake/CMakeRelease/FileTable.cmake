set(version "@version@")
set(version_major "1")
set(version_minor "2")
set(version_patch "3")
set(maybe_version_suffix "\"suffix\": \"rc4\",")
configure_file("${CMAKE_CURRENT_LIST_DIR}/../../../Utilities/Release/files-v1.json.in" "files-v1.json" @ONLY)

foreach(query
    ".version | .major , .minor , .patch , .suffix, .string"
    ".files[].name"
    ".files[] | select(.os[] | . == \"source\") | .name"
    ".files[] | select((.os[] | . == \"macOS\") and (.class == \"volume\")) | .name"
    ".files[] | select((.os[] | . == \"macos10.10\") and (.class == \"archive\")) | .name"
    ".files[] | select((.os[] | . == \"windows\") and (.architecture[] | . == \"i386\") and (.class == \"installer\")) | .name"
    ".files[] | select(.architecture[] | . == \"x86_64\") | .name"
    ".files[] | select([.macOSmin] | inside([\"10.10\", \"10.11\", \"10.12\"])) | .name"
    ".hashFiles[] | select(.algorithm[] | . == \"SHA-256\") | .name"
    )
  message(STATUS "query: ${query}")
  execute_process(COMMAND ${JQ} "${query}" files-v1.json)
endforeach()
