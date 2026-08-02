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
#include "transfer.h"
#include <tgf.h>

int ptransfer::start(const char *url, long max, const void *data, size_t n,
    const transfer::headers &headers)
{
    CURLcode code;
    std::string form;

    if (transfer::start(url, max, headers))
    {
        GfLogError("transfer::start failed\n");
        return -1;
    }
    else if ((code = curl_easy_setopt(handle(), CURLOPT_POST, 1L)) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt post: %s\n", curl_easy_strerror(code));
        return -1;
    }
    else if ((code = curl_easy_setopt(handle(), CURLOPT_POSTFIELDSIZE, n))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt postfield size: %s\n",
            curl_easy_strerror(code));
        return -1;
    }
    else if (data
        && (code = curl_easy_setopt(handle(), CURLOPT_COPYPOSTFIELDS, data))
            != CURLE_OK)
    {
        GfLogError("curl_easy_setopt copypostfields: %s\n",
            curl_easy_strerror(code));
        return -1;
    }

    return 0;
}

int ptransfer::escape(const std::string &s, std::string &out) const
{
    char *escaped = curl_easy_escape(handle(), s.c_str(), 0);

    if (!escaped)
    {
        GfLogError("curl_easy_escape failed\n");
        return -1;
    }

    out = escaped;
    curl_free(escaped);
    return 0;
}

int ptransfer::start(const char *url, long max, const std::vector<form> &forms,
    const transfer::headers &headers)
{
    CURLcode code;
    std::string form;

    if (transfer::start(url, max, headers))
    {
        GfLogError("transfer::start failed\n");
        return -1;
    }

    for (size_t i = 0; i < forms.size(); i++)
    {
        const auto &f = forms.at(i);
        std::string key, value;

        if (escape(f.key, key) || escape(f.value, value))
        {
            GfLogError("Failed to escape key/value pair\n");
            return -1;
        }

        form += key + '=' + value;

        if (i < forms.size() - 1)
            form += '&';
    }

    if ((code = curl_easy_setopt(handle(), CURLOPT_COPYPOSTFIELDS,
        form.c_str()))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt copypostfields: %s\n",
            curl_easy_strerror(code));
        return -1;
    }

    return 0;
}

int ptransfer::start(const char *url, long max, const char *path,
    const transfer::headers &headers)
{
    CURLcode code;
    curl_mime *mime = nullptr;
    curl_mimepart *part;

    if (transfer::start(url, max, headers))
    {
        GfLogError("transfer::start failed\n");
        goto failure;
    }
    else if (!(mime = curl_mime_init(handle())))
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
    else if ((code = curl_easy_setopt(handle(), CURLOPT_MIMEPOST, mime))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt mimepost: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }

    return 0;

failure:
    curl_mime_free(mime);
    return -1;
}

ptransfer::ptransfer(sink *s, cb f, void *args) :
    transfer(s, f, args)
{
}
