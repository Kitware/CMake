mod moda;

extern "C" {
    fn liba_greet();
    fn libb_greet();
    fn libc_greet();
}

fn main() {
    println!("Hello from main.rs");
    moda::moda_greet();
    unsafe { liba_greet() };
    unsafe { libb_greet() };
    unsafe { libc_greet() };
}
