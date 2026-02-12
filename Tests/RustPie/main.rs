
extern "C" {
    fn static_foo();
    fn shared_foo();
}


extern crate object;
use object::object_foo;


fn main() {
    unsafe {
        static_foo();
        shared_foo();
    }
    object_foo();
}
