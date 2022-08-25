#[cfg(all(target_family = "unix", not(target_os = "macos")))]
mod gtk;

use std::os::raw::*;

pub enum CWebView {} // opaque type, only used in ffi pointers

extern "C" {
    pub fn webview_new(
        title: *const c_char,
        width: c_int,
        height: c_int,
        min_width: c_int,
        min_height: c_int,
        resizable: c_int,
        frameless: c_int,
        visible: c_int,
    ) -> *mut CWebView;

    pub fn webview_loop(
        this: *mut CWebView,
        blocking: c_int
    ) -> c_int;
}
