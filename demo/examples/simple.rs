extern crate webview_gui;

use webview_gui::WebView;

fn main() {
    match WebView::new() {
        Ok(mut wv) => {
            wv.draw();
        },
        Err(_) => (),
    }
}
