#include "webview.h"

#include <objc/objc-runtime.h>
#include <CoreGraphics/CoreGraphics.h>
#include <limits.h>

#define NSAlertStyleWarning 0
#define NSAlertStyleCritical 2
#define NSWindowStyleMaskResizable 8
#define NSWindowStyleMaskMiniaturizable 4
#define NSWindowStyleMaskTitled 1
#define NSWindowStyleMaskClosable 2
#define NSWindowStyleMaskFullScreen (1 << 14)
#define NSViewWidthSizable 2
#define NSViewHeightSizable 16
#define NSBackingStoreBuffered 2
#define NSEventMaskAny ULONG_MAX
#define NSEventModifierFlagCommand (1 << 20)
#define NSEventModifierFlagOption (1 << 19)
#define NSAlertStyleInformational 1
#define NSAlertFirstButtonReturn 1000
#define WKNavigationActionPolicyDownload 2
#define NSModalResponseOK 1
#define WKNavigationActionPolicyDownload 2
#define WKNavigationResponsePolicyAllow 1
#define WKUserScriptInjectionTimeAtDocumentStart 0
#define NSApplicationActivationPolicyRegular 0
#define NSApplicationDefinedEvent 15
#define NSWindowStyleMaskBorderless 0

struct webview_priv
{
    id pool;
    id window;
    id webview;
    id windowDelegate;
    int should_exit;
};

struct cocoa_webview
{
    const char *title;
    int width;
    int height;
    int min_width;
    int min_height;
    int resizable;
    int frameless;
    int visible;
    struct webview_priv priv;
};

static void webview_window_will_close(id self, SEL cmd, id notification)
{
}

static bool webview_window_should_close(id self, SEL cmd, id sender)
{
    return false;
}

WEBVIEW_API int webview_init(webview_t w)
{
    struct cocoa_webview *webview = (struct cocoa_webview *)w;

    // autorelease pool
    webview->priv.pool = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSAutoreleasePool"),
                                                        sel_registerName("new"));

    ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
                                   sel_registerName("sharedApplication"));

    static Class __NSWindowDelegate;
    if (__NSWindowDelegate == NULL)
    {
        __NSWindowDelegate = objc_allocateClassPair(objc_getClass("NSObject"),
                                                    "__NSWindowDelegate", 0);
        class_addProtocol(__NSWindowDelegate, objc_getProtocol("__NSWindowDelegate"));
        class_replaceMethod(__NSWindowDelegate, sel_registerName("windowWillClose:"),
                            (IMP)webview_window_will_close, "v@:@");
        class_replaceMethod(__NSWindowDelegate, sel_registerName("windowShouldClose:"),
                            (IMP)webview_window_should_close, "B@:@");
        objc_registerClassPair(__NSWindowDelegate);
    }

    webview->priv.windowDelegate =
        ((id(*)(id, SEL))objc_msgSend)((id)__NSWindowDelegate, sel_registerName("new"));

    objc_setAssociatedObject(webview->priv.windowDelegate, "webview", (id)(w),
                             OBJC_ASSOCIATION_ASSIGN);

    id nsTitle =
        ((id(*)(id, SEL, id))objc_msgSend)((id)objc_getClass("NSString"),
                                           sel_registerName("stringWithUTF8String:"), webview->title);

    CGRect r = CGRectMake(0, 0, webview->width, webview->height);

    unsigned int style;
    if (webview->frameless)
    {
        style = NSWindowStyleMaskBorderless | NSWindowStyleMaskMiniaturizable;
    }
    else
    {
        style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                NSWindowStyleMaskMiniaturizable;
    }
    if (webview->resizable)
    {
        style = style | NSWindowStyleMaskResizable;
    }

    webview->priv.window =
        ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSWindow"), sel_registerName("alloc"));

    ((id(*)(id, SEL, CGRect, id, id, id))objc_msgSend)(webview->priv.window,
                                                       sel_registerName("initWithContentRect:styleMask:backing:defer:"),
                                                       r, style, NSBackingStoreBuffered, 0);

    ((id(*)(id, SEL))objc_msgSend)(webview->priv.window, sel_registerName("autorelease"));
    ((id(*)(id, SEL, id))objc_msgSend)(webview->priv.window, sel_registerName("setTitle:"), nsTitle);
    ((id(*)(id, SEL, id))objc_msgSend)(webview->priv.window, sel_registerName("setDelegate:"),
                                       webview->priv.windowDelegate);
    ((id(*)(id, SEL))objc_msgSend)(webview->priv.window, sel_registerName("center"));

    if (webview->visible)
    {
        ((id(*)(id, SEL))objc_msgSend)(webview->priv.window, sel_registerName("orderFrontRegardless"));
    }

    ((id(*)(id, SEL, CGSize))objc_msgSend)(webview->priv.window, sel_registerName("setMinSize:"), CGSizeMake(webview->min_width, webview->min_height));

    ((id(*)(id, SEL))objc_msgSend)(((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
                                                                  sel_registerName("sharedApplication")),
                                   sel_registerName("finishLaunching"));

    ((id(*)(id, SEL, id))objc_msgSend)(((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
                                                                      sel_registerName("sharedApplication")),
                                       sel_registerName("activateIgnoringOtherApps:"), 1);

    webview->priv.should_exit = 0;

    return 0;
}

WEBVIEW_API int webview_loop(webview_t w, int blocking)
{
    struct cocoa_webview *webview = (struct cocoa_webview *)w;

    id until = (blocking ? ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSDate"),
                                                          sel_registerName("distantFuture"))
                         : ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSDate"),
                                                          sel_registerName("distantPast")));

    id app = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
                                            sel_registerName("sharedApplication"));

    id event = ((id(*)(id, SEL, id, id, id, id))objc_msgSend)(
        app,
        sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"),
        ULONG_MAX, until,
        ((id(*)(id, SEL, const char *))objc_msgSend)((id)objc_getClass("NSString"),
                                                     sel_registerName("stringWithUTF8String:"),
                                                     "kCFRunLoopDefaultMode"),
        true);

    if (event)
    {
        ((id(*)(id, SEL, id))objc_msgSend)(((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
                                                                          sel_registerName("sharedApplication")),
                                           sel_registerName("sendEvent:"), event);
    }

    return webview->priv.should_exit;
}

WEBVIEW_API webview_t webview_new(
    const char *title,
    int width,
    int height,
    int min_width,
    int min_height,
    int resizable,
    int frameless,
    int visible)
{
    struct cocoa_webview *instance = (struct cocoa_webview *)calloc(1, sizeof(*instance));
    instance->title = title;
    instance->width = width;
    instance->height = height;
    instance->min_width = min_width;
    instance->min_height = min_height;
    instance->resizable = resizable;
    instance->frameless = frameless;
    instance->visible = visible;

    if (webview_init(instance) != 0)
    {
        // error handler
    }

    return instance;
}
