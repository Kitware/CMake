set(CMAKE_EXPERIMENTAL_RUST "efaed83b-d73a-48af-999a-bd0a6172c313")
enable_language(Rust)

# No Edition set, rustc defaults to 2015.
add_library(Edition.2015 OBJECT edition_2015.rs)
set_target_properties(Edition.2015 PROPERTIES Rust_EDITION 2015)

# Explicitly set edition on target, without default value from variable.
add_library(Edition.2018 OBJECT edition_2018.rs)
set_target_properties(Edition.2018 PROPERTIES Rust_EDITION 2018)

# Implicitly set edition on target from variable
set(CMAKE_Rust_EDITION 2021)
add_library(Edition.2021 OBJECT edition_2021.rs)

# Explicitly override the edition.
add_library(Edition.2024 OBJECT edition_2024.rs)
set_target_properties(Edition.2024 PROPERTIES Rust_EDITION 2024)

# Rust guarantees that we can link multiple crates with different editions
# together. We specify explicitly the edition to make sure CMake properly pass
# the edition in the linking step too.
unset(CMAKE_Rust_EDITION)
add_executable(Editions editions.rs)
set_target_properties(Editions PROPERTIES Rust_EDITION 2018)
target_link_libraries(
    Editions PRIVATE
    Edition.2015
    Edition.2018
    Edition.2021
    Edition.2024
)
