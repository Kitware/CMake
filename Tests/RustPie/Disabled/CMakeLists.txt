
project(DisabledRustPie LANGUAGES Rust)
setup(disabled)

set_property(
    TARGET
        disabled_static
        disabled_shared
        disabled_object
        disabled_RustPie
    PROPERTY POSITION_INDEPENDENT_CODE OFF
)
