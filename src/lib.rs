extern crate webview_gui_sys as ffi;
use std::ffi::CString;

mod error;

pub use error::{Error, WVResult};
use ffi::CWebView;

pub struct WebView {
    inner: Option<*mut ffi::CWebView>,
}

impl WebView {
    pub fn new() -> WVResult<WebView> {
        unsafe {
            let title = CString::new("Test").unwrap();
            let inner = ffi::webview_new(
                title.as_ptr(),
                480,
                800,
                480,
                800,
                0,
                0,
                1
            );

            if inner.is_null() {
                Err(Error::Initialization)
            } else {
                Ok(WebView::from_ptr(inner))
            }
        }
    }

    unsafe fn from_ptr(inner: *mut CWebView) -> WebView {
        WebView {
            inner: Some(inner),
        }
    }

    fn step(&mut self) -> Option<i32> {
        unsafe {
            match ffi::webview_loop(self.inner.unwrap(), 1) {
                0 => Some(0),
                _ => None,
            }
        }
    }

    pub fn draw(&mut self) {
        loop {
            match self.step() {
                Some(num) => num,
                None => 0,
            };
        }
    }

    pub fn erase(&mut self) {}
}
