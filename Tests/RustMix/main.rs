extern "C" {
  // Functions defined in C
  fn c_obj_greet();
  fn c_static_greet();
  fn c_shared_greet();

  // Functions defined in C++
  fn cpp_shared_greet();

  // Functions defined in Rust through a C-style ABI
  fn rs_staticlib_greet();
  fn rs_cdylib_greet();
}

// Functions defined in Rust, using native Rust ABI
extern crate rs_rlib;
use rs_rlib::rs_rlib_greet;

fn main() {
  println!("Hello from main.rs");
  unsafe {
    c_obj_greet();
    c_static_greet();
    c_shared_greet();
    cpp_shared_greet();
    rs_staticlib_greet();
    rs_cdylib_greet();
  }
  rs_rlib_greet();
}
