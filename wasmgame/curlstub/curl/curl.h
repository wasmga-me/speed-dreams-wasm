/* Stand-in for libcurl in the browser build of Speed Dreams: the game has no
 * network access there, so every transfer fails at once (the downloads menu,
 * the web server lap times and the online weather stay off). */
#ifndef WG_CURL_STUB_H
#define WG_CURL_STUB_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#define LIBCURL_VERSION_MAJOR 8
#define LIBCURL_VERSION_MINOR 0

typedef void CURL;
typedef void CURLM;
typedef struct curl_mime curl_mime;
typedef struct curl_mimepart curl_mimepart;
struct curl_slist { char *data; struct curl_slist *next; };

typedef enum { CURLE_OK = 0, CURLE_FAILED_INIT = 2, CURLE_HTTP_RETURNED_ERROR = 22 } CURLcode;
typedef enum { CURLM_OK = 0, CURLM_INTERNAL_ERROR = 4 } CURLMcode;
typedef enum { CURLMSG_NONE, CURLMSG_DONE } CURLMSG;
typedef struct CURLMsg { CURLMSG msg; CURL *easy_handle; union { void *whatever; CURLcode result; } data; } CURLMsg;

typedef enum {
  CURLOPT_WRITEDATA = 10001, CURLOPT_URL = 10002, CURLOPT_USERAGENT = 10018,
  CURLOPT_HTTPHEADER = 10023, CURLOPT_HEADERDATA = 10029, CURLOPT_COPYPOSTFIELDS = 10165,
  CURLOPT_MIMEPOST = 10269, CURLOPT_PROTOCOLS_STR = 10318,
  CURLOPT_LOW_SPEED_LIMIT = 19, CURLOPT_LOW_SPEED_TIME = 20, CURLOPT_VERBOSE = 41,
  CURLOPT_FAILONERROR = 45, CURLOPT_POST = 47, CURLOPT_FOLLOWLOCATION = 52,
  CURLOPT_POSTFIELDSIZE = 60, CURLOPT_MAXREDIRS = 68, CURLOPT_MAXFILESIZE = 114,
  CURLOPT_PROTOCOLS = 181,
  CURLOPT_WRITEFUNCTION = 20011, CURLOPT_HEADERFUNCTION = 20079
} CURLoption;
typedef enum {
  CURLINFO_EFFECTIVE_URL = 0x100001, CURLINFO_RESPONSE_CODE = 0x200002,
  CURLINFO_CONTENT_TYPE = 0x100012
} CURLINFO;

#define CURL_GLOBAL_DEFAULT 3
#define CURL_ZERO_TERMINATED ((size_t)-1)
#define CURLPROTO_HTTP (1 << 0)
#define CURLPROTO_HTTPS (1 << 1)

CURLcode curl_global_init(long flags);
void curl_global_cleanup(void);
CURL *curl_easy_init(void);
void curl_easy_cleanup(CURL *h);
CURLcode curl_easy_setopt(CURL *h, CURLoption opt, ...);
CURLcode curl_easy_getinfo(CURL *h, CURLINFO info, ...);
CURLcode curl_easy_perform(CURL *h);
const char *curl_easy_strerror(CURLcode c);
char *curl_easy_escape(CURL *h, const char *s, int len);
void curl_free(void *p);
CURLM *curl_multi_init(void);
CURLMcode curl_multi_cleanup(CURLM *m);
CURLMcode curl_multi_add_handle(CURLM *m, CURL *h);
CURLMcode curl_multi_remove_handle(CURLM *m, CURL *h);
CURLMcode curl_multi_perform(CURLM *m, int *running);
CURLMcode curl_multi_poll(CURLM *m, void *fds, unsigned n, int timeout_ms, int *numfds);
CURLMsg *curl_multi_info_read(CURLM *m, int *left);
const char *curl_multi_strerror(CURLMcode c);
struct curl_slist *curl_slist_append(struct curl_slist *l, const char *s);
void curl_slist_free_all(struct curl_slist *l);
curl_mime *curl_mime_init(CURL *h);
void curl_mime_free(curl_mime *m);
curl_mimepart *curl_mime_addpart(curl_mime *m);
CURLcode curl_mime_name(curl_mimepart *p, const char *name);
CURLcode curl_mime_data(curl_mimepart *p, const char *data, size_t size);
CURLcode curl_mime_filedata(curl_mimepart *p, const char *file);

#ifdef __cplusplus
}
#endif
#endif
