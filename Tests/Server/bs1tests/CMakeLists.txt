
do_build(buildsystem1)

set(request
  {\"type\":\"parse\",\"file_path\":\"${CMAKE_SOURCE_DIR}/buildsystem1/CMakeLists.txt\"}
)

set(response
  [==[{"parsed":{"tokens":[{"column":1,"length":22,"line":1,"type":"command"},{"column":23,"length":1,"line":1,"type":"left paren"},{"column":24,"length":7,"line":1,"type":"keyword"},{"column":32,"length":3,"line":1,"type":"unquoted argument"},{"column":35,"length":1,"line":1,"type":"right paren"},{"column":1,"length":7,"line":3,"type":"command"},{"column":8,"length":1,"line":3,"type":"left paren"},{"column":9,"length":12,"line":3,"type":"identifier"},{"column":21,"length":1,"line":3,"type":"right paren"},{"column":1,"length":3,"line":5,"type":"command"},{"column":4,"length":1,"line":5,"type":"left paren"},{"column":5,"length":4,"line":5,"type":"identifier"},{"column":10,"length":3,"line":5,"type":"unquoted argument"},{"column":13,"length":1,"line":5,"type":"right paren"},{"column":1,"length":3,"line":7,"type":"command"},{"column":4,"length":1,"line":7,"type":"left paren"},{"column":5,"length":4,"line":7,"type":"identifier"},{"column":10,"length":3,"line":7,"type":"unquoted argument"},{"column":13,"length":1,"line":7,"type":"right paren"},{"column":1,"length":3,"line":9,"type":"command"},{"column":4,"length":1,"line":9,"type":"left paren"},{"column":5,"length":4,"line":9,"type":"identifier"},{"column":10,"length":3,"line":9,"type":"unquoted argument"},{"column":13,"length":1,"line":9,"type":"right paren"},{"column":1,"length":3,"line":11,"type":"command"},{"column":4,"length":1,"line":11,"type":"left paren"},{"column":5,"length":4,"line":11,"type":"identifier"},{"column":10,"length":3,"line":11,"type":"unquoted argument"},{"column":13,"length":1,"line":11,"type":"right paren"},{"column":1,"length":3,"line":13,"type":"command"},{"column":4,"length":1,"line":13,"type":"left paren"},{"column":5,"length":4,"line":13,"type":"identifier"},{"column":10,"length":3,"line":13,"type":"unquoted argument"},{"column":13,"length":1,"line":13,"type":"right paren"}],"unreachable":[]}}]==]
)

do_test(buildsystem1 "${request}" "${response}")

set(content [==[cmake_minimum_required(VERSION 3.4)\n\nproject(buildsystem1)\n\nset(var1 123)\n\n\nset(var2 345)\n\nset(var4 987)\nset(var5 987)\n]==])

set(request
  {\"type\":\"parse\",\"file_path\":\"${CMAKE_SOURCE_DIR}/buildsystem1/CMakeLists.txt\",\"file_content\":\"${content}\"}
)

set(response
  [==[{"parsed":{"tokens":[{"column":1,"length":22,"line":1,"type":"command"},{"column":23,"length":1,"line":1,"type":"left paren"},{"column":24,"length":7,"line":1,"type":"keyword"},{"column":32,"length":3,"line":1,"type":"unquoted argument"},{"column":35,"length":1,"line":1,"type":"right paren"},{"column":1,"length":7,"line":3,"type":"command"},{"column":8,"length":1,"line":3,"type":"left paren"},{"column":9,"length":12,"line":3,"type":"identifier"},{"column":21,"length":1,"line":3,"type":"right paren"},{"column":1,"length":3,"line":5,"type":"command"},{"column":4,"length":1,"line":5,"type":"left paren"},{"column":5,"length":4,"line":5,"type":"identifier"},{"column":10,"length":3,"line":5,"type":"unquoted argument"},{"column":13,"length":1,"line":5,"type":"right paren"},{"column":1,"length":3,"line":8,"type":"command"},{"column":4,"length":1,"line":8,"type":"left paren"},{"column":5,"length":4,"line":8,"type":"identifier"},{"column":10,"length":3,"line":8,"type":"unquoted argument"},{"column":13,"length":1,"line":8,"type":"right paren"},{"column":1,"length":3,"line":10,"type":"command"},{"column":4,"length":1,"line":10,"type":"left paren"},{"column":5,"length":4,"line":10,"type":"identifier"},{"column":10,"length":3,"line":10,"type":"unquoted argument"},{"column":13,"length":1,"line":10,"type":"right paren"},{"column":1,"length":3,"line":11,"type":"command"},{"column":4,"length":1,"line":11,"type":"left paren"},{"column":5,"length":4,"line":11,"type":"identifier"},{"column":10,"length":3,"line":11,"type":"unquoted argument"},{"column":13,"length":1,"line":11,"type":"right paren"}],"unreachable":[]}}]==]
)

do_test(buildsystem1 "${request}" "${response}")

set(content [==[cmake_minimum_required(VERSION 3.4)\n\nproject(buildsystem1)\n\nset(var1 666)\n\n\nset(var2 345)\n\nset(var4 987)\nset(var5 987)\n]==])

set(request
  {\"type\":\"content\",\"file_path\":\"${CMAKE_SOURCE_DIR}/buildsystem1/CMakeLists.txt\",\"file_content\":\"${content}\",\"matcher\":\"var\",\"file_line\":10}
)

set(response
  [==[{"content":{"var1":"666", "var2":"345"}}
  ]==]
)

do_test(buildsystem1 "${request}" "${response}")
