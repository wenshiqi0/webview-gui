extern crate cc;
extern crate pkg_config;

use std::env;

fn main() {
    let target = env::var("TARGET").unwrap();

    let mut build = cc::Build::new();

    if target.contains("apple") {
        build.file("src/osx_cocoa.c").flag("-x").flag("objective-c");
        println!("cargo:rustc-link-lib=framework=Cocoa");
        println!("cargo:rustc-link-lib=framework=WebKit");
    }

    build.compile("webview");
}
