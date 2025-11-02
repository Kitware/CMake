#[no_mangle]
pub extern "C" fn libc_greet() {
    println!("Hello from libc");
}
