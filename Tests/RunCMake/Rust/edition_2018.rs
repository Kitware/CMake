#![allow(rust_2021_compatibility)]

pub trait Foo {}

struct FooA {}
impl Foo for FooA {}

struct FooB {}
impl Foo for FooB {}

// dyn keyword is available in 2018+
pub fn make_foo(value: i32) -> Box<dyn Foo> {
    match value {
        // Unsupported in 2021+, should be 0..=42
        0...42 => Box::new(FooA{}),
        _ => Box::new(FooB{}),
    }
}
