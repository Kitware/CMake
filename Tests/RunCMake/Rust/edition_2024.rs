pub fn add_options(a: Option<i32>, b: Option<i32>) -> Option<i32> {
    // let chains only allowed in Rust 2024
    if let Some(a) = a && let Some(b) = b {
        Some(a + b)
    } else {
        None
    }
}
