#[no_mangle]
pub extern "C" fn rs_cdylib_greet() {
    println!("Hello from a Rust cdylib!");
}
