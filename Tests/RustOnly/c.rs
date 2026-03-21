// We can expose a Rust API, as this is compiled into a rlib.
pub fn libc_greet() {
    println!("Hello from libc");
}
