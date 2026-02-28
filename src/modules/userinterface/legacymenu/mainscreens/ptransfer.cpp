/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "ptransfer.h"
#include <tgf.h>

int ptransfer::exec(CURLcode result, std::string &error) const
{
    return check(result, error) || f(result, h, error, args);
}

struct curl_slist *ptransfer::get_headers(const transferbase::headers &headers)
    const
{
    struct curl_slist *ret = nullptr;

    for (const auto &header : headers)
    {
        std::string s = header.field + ": " + header.value;
        struct curl_slist *sl = curl_slist_append(ret, s.c_str());

        if (!sl)
        {
            GfLogError("curl_slist_append failed\n");
            goto failure;
        }
    }

    return ret;

failure:
    curl_slist_free_all(ret);
    return nullptr;
}

int ptransfer::start(const char *url, const std::vector<form> &forms,
    const transferbase::headers &headers)
{
    int ret = -1;
    CURLcode code;
    char *escaped = nullptr;
    CURL *h = curl_easy_init();
    struct curl_slist *list = nullptr;
    std::string form;

    if (!h)
    {
        GfLogError("curl_easy_init failed\n");
        goto end;
    }
    else if (!(list = get_headers(headers)))
    {
        GfLogError("get_headers failed\n");
        goto end;
    }

    for (size_t i = 0; i < forms.size(); i++)
    {
        const auto &f = forms.at(i);

        form += f.key + '=' +f.value;

        if (i < forms.size() - 1)
            form += '&';
    }

    if (!(escaped = curl_easy_escape(h, form.c_str(), 0)))
    {
        GfLogError("curl_easy_escape failed\n");
        goto end;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_URL, url)) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt url: %s\n", curl_easy_strerror(code));
        goto end;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_PROTOCOLS,
        CURLPROTO_HTTP | CURLPROTO_HTTPS)) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt protocols: %s\n",
            curl_easy_strerror(code));
        goto end;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt follow location: %s\n",
            curl_easy_strerror(code));
        goto end;
    }
    /* This value is the documented default since cURL 8.3.0.
     * Make it explicit to follow the same behaviour on older versions. */
    else if ((code = curl_easy_setopt(h, CURLOPT_MAXREDIRS, 30L))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt max redirs: %s\n",
            curl_easy_strerror(code));
        goto end;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, escaped))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt copypostfields: %s\n",
            curl_easy_strerror(code));
        goto end;
    }

    this->h = h;
    this->headers = list;
    ret = 0;

end:
    if (ret)
    {
        curl_slist_free_all(list);
        curl_easy_cleanup(h);
    }

    curl_free(escaped);
    return ret;
}

int ptransfer::start(const char *url, const char *path,
    const transferbase::headers &headers)
{
    CURLcode code;
    CURL *h = curl_easy_init();
    curl_mime *mime;
    curl_mimepart *part;

    if (!h)
    {
        GfLogError("curl_easy_init failed\n");
        goto failure;
    }
    else if (!(mime = curl_mime_init(h)))
    {
        GfLogError("curl_mime_init failed\n");
        goto failure;
    }
    else if (!(part = curl_mime_addpart(mime)))
    {
        GfLogError("curl_mime_addpart failed\n");
        goto failure;
    }
    else if ((code = curl_mime_name(part, "filename")) != CURLE_OK)
    {
        GfLogError("curl_mime_name: %s\n", curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_mime_filedata(part, path)) != CURLE_OK)
    {
        GfLogError("curl_mime_filedata: %s\n", curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_URL, url)) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt url: %s\n", curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_PROTOCOLS,
        CURLPROTO_HTTP | CURLPROTO_HTTPS)) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt protocols: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_MIMEPOST, mime)) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt mimepost: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 0L))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt follow location: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }

    this->h = h;
    return 0;

failure:
    curl_easy_cleanup(h);
    return -1;
}

ptransfer::~ptransfer()
{
    curl_slist_free_all(headers);
}

ptransfer::ptransfer(cb f, void *args) :
    transferbase(),
    f(f),
    args(args),
    headers(nullptr)
{
}
