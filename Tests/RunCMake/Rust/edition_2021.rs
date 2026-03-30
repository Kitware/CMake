// gen is becoming a keyword in 2024
pub fn gen() -> i32 {
    // This does not compile before Rust 2021
    [1, 2, 3].into_iter().reduce(|acc, n| acc + n).unwrap_or(0)
}
