#[no_mangle]
pub extern "C" fn rs_staticlib_greet() {
    println!("Hello from a Rust staticlib!");
}
