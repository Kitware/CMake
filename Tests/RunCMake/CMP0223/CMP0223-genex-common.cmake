file(GENERATE OUTPUT "result.txt" CONTENT
  "empty=$<PATH:IS_PREFIX,,/a/b> normalize=$<PATH:IS_PREFIX,NORMALIZE,,/a/b> nonempty=$<PATH:IS_PREFIX,/a,/a/b>")
