export CC=/usr/bin/clang
export CXX=/usr/bin/clazy
export CLANGXX=/usr/bin/clang++

export CLAZY_CHECKS="level2\
,no-base-class-event\
,no-connect-3arg-lambda\
,no-connect-by-name\
,no-container-inside-loop\
,no-copyable-polymorphic\
,no-ctor-missing-parent-argument\
,no-function-args-by-ref\
,no-missing-qobject-macro\
,no-non-pod-global-static\
,no-old-style-connect\
,no-qproperty-without-notify\
,no-qstring-allocations\
,no-range-loop-detach\
,no-range-loop-reference\
,no-reserve-candidates\
,no-rule-of-three\
,no-rule-of-two-soft\
,no-static-pmf\
,no-strict-iterators\
"
