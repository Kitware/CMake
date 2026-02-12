
project(EnabledRustPie LANGUAGES Rust)
setup(enabled)

set_property(
    TARGET
        enabled_static
        enabled_shared
        enabled_object
        enabled_RustPie
    PROPERTY POSITION_INDEPENDENT_CODE TRUE
)
