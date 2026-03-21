#[no_mangle]
pub extern "C" fn shared_foo() {
    println!("shared_foo");
}
