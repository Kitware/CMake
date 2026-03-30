
use edition_2015;
use edition_2018;
use edition_2021;
use edition_2024;

fn main() {
    edition_2015::r#dyn();
    let _ = edition_2018::make_foo(1337);
    let _ = edition_2021::r#gen();
    let _ = edition_2024::add_options(None, None);
}
