ctresalloc_verify_log(
[[begin test1
alloc widgets 0 1
dealloc widgets 0 1
end test1
begin ctresalloc-write-proc-good1
alloc transmogrifiers calvin 1
alloc widgets 0 2
alloc widgets 0 1
alloc widgets 2 2
alloc widgets 0 1
alloc widgets 2 2
dealloc transmogrifiers calvin 1
dealloc widgets 0 2
dealloc widgets 0 1
dealloc widgets 2 2
dealloc widgets 0 1
dealloc widgets 2 2
end ctresalloc-write-proc-good1
]])
