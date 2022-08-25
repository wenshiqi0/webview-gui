#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef WEBVIEW_STATIC
#define WEBVIEW_API static
#else
#define WEBVIEW_API extern
#endif

typedef void *webview_t;

#ifdef __cplusplus
}
#endif
