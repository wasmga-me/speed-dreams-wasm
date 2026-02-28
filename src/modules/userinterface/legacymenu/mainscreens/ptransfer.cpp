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
    return check(result, error) || f(result, h, error, args, resp_headers);
}

int ptransfer::start(const char *url, const void *data, size_t n,
    const transferbase::headers &headers)
{
    CURLcode code;
    std::string form;

    if (transferbase::start(url, headers))
    {
        GfLogError("transferbase::start failed\n");
        return -1;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_POST, 1L)) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt post: %s\n", curl_easy_strerror(code));
        return -1;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_POSTFIELDSIZE, n))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt postfield size: %s\n",
            curl_easy_strerror(code));
        return -1;
    }
    else if (data
        && (code = curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, data))
            != CURLE_OK)
    {
        GfLogError("curl_easy_setopt copypostfields: %s\n",
            curl_easy_strerror(code));
        return -1;
    }

    return 0;
}

int ptransfer::start(const char *url, const std::vector<form> &forms,
    const transferbase::headers &headers)
{
    int ret = -1;
    CURLcode code;
    std::string form;
    char *escaped = nullptr;

    if (transferbase::start(url, headers))
    {
        GfLogError("transferbase::start failed\n");
        goto end;
    }
    else if (!(escaped = curl_easy_escape(h, form.c_str(), 0)))
    {
        GfLogError("curl_easy_escape failed\n");
        goto end;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_COPYPOSTFIELDS, escaped))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt copypostfields: %s\n",
            curl_easy_strerror(code));
        goto end;
    }

    ret = 0;

end:
    curl_free(escaped);
    return ret;
}

int ptransfer::start(const char *url, const char *path,
    const transferbase::headers &headers)
{
    CURLcode code;
    struct curl_slist *list = nullptr;
    curl_mime *mime;
    curl_mimepart *part;

    if (transferbase::start(url, headers))
    {
        GfLogError("transferbase::start failed\n");
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
    else if ((code = curl_easy_setopt(h, CURLOPT_MIMEPOST, mime)) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt mimepost: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }

    return 0;

failure:
    curl_slist_free_all(list);
    curl_easy_cleanup(h);
    return -1;
}

ptransfer::ptransfer(cb f, void *args) :
    transferbase(),
    f(f),
    args(args)
{
}
