mod moda;

extern "C" {
    fn liba_greet();
    fn libb_greet();
    fn libe_greet();
}

extern crate c;
extern crate d;
use c::libc_greet;
use d::libd_greet;

fn main() {
    println!("Hello from main.rs");
    moda::moda_greet();
    unsafe { liba_greet() };
    unsafe { libb_greet() };
    libc_greet();
    libd_greet();
    unsafe { libe_greet() };
}
