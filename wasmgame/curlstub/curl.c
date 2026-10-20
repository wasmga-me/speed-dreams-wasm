/* See curl/curl.h: every call fails, so nothing reaches the network. */
#include <stdlib.h>
#include "curl/curl.h"

CURLcode curl_global_init(long flags) { (void)flags; return CURLE_OK; }
void curl_global_cleanup(void) {}
CURL *curl_easy_init(void) { return NULL; }
void curl_easy_cleanup(CURL *h) { (void)h; }
CURLcode curl_easy_setopt(CURL *h, CURLoption opt, ...) { (void)h; (void)opt; return CURLE_FAILED_INIT; }
CURLcode curl_easy_getinfo(CURL *h, CURLINFO info, ...) { (void)h; (void)info; return CURLE_FAILED_INIT; }
CURLcode curl_easy_perform(CURL *h) { (void)h; return CURLE_FAILED_INIT; }
const char *curl_easy_strerror(CURLcode c) { return c == CURLE_OK ? "No error" : "No network in the browser build"; }
char *curl_easy_escape(CURL *h, const char *s, int len) { (void)h; (void)s; (void)len; return NULL; }
void curl_free(void *p) { free(p); }
CURLM *curl_multi_init(void) { return NULL; }
CURLMcode curl_multi_cleanup(CURLM *m) { (void)m; return CURLM_OK; }
CURLMcode curl_multi_add_handle(CURLM *m, CURL *h) { (void)m; (void)h; return CURLM_INTERNAL_ERROR; }
CURLMcode curl_multi_remove_handle(CURLM *m, CURL *h) { (void)m; (void)h; return CURLM_OK; }
CURLMcode curl_multi_perform(CURLM *m, int *running) { (void)m; if (running) *running = 0; return CURLM_OK; }
CURLMcode curl_multi_poll(CURLM *m, void *fds, unsigned n, int timeout_ms, int *numfds) { (void)m; (void)fds; (void)n; (void)timeout_ms; if (numfds) *numfds = 0; return CURLM_OK; }
CURLMsg *curl_multi_info_read(CURLM *m, int *left) { (void)m; if (left) *left = 0; return NULL; }
const char *curl_multi_strerror(CURLMcode c) { return c == CURLM_OK ? "No error" : "No network in the browser build"; }
struct curl_slist *curl_slist_append(struct curl_slist *l, const char *s) { (void)s; return l; }
void curl_slist_free_all(struct curl_slist *l) { (void)l; }
curl_mime *curl_mime_init(CURL *h) { (void)h; return NULL; }
void curl_mime_free(curl_mime *m) { (void)m; }
curl_mimepart *curl_mime_addpart(curl_mime *m) { (void)m; return NULL; }
CURLcode curl_mime_name(curl_mimepart *p, const char *name) { (void)p; (void)name; return CURLE_FAILED_INIT; }
CURLcode curl_mime_data(curl_mimepart *p, const char *data, size_t size) { (void)p; (void)data; (void)size; return CURLE_FAILED_INIT; }
CURLcode curl_mime_filedata(curl_mimepart *p, const char *file) { (void)p; (void)file; return CURLE_FAILED_INIT; }
