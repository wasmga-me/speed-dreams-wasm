/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2025 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "transfer.h"
#include <curl/curl.h>
#include <tgfclient.h>

int transfer::check(CURLcode result, std::string &error) const
{
    int ret = -1;
    long response;
    CURLcode code;
    char *url;

    if ((code = curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE,
        &response)) != CURLE_OK)
    {
        const char *e = curl_easy_strerror(code);

        error = "Failed to retrieve response code: ";
        error += e;
    }
    else if ((code = curl_easy_getinfo(h, CURLINFO_EFFECTIVE_URL, &url))
        != CURLE_OK)
    {
        const char *e = curl_easy_strerror(code);

        error = "Failed to retrieve effective URL: ";
        error += e;
    }
    else if (response != 200)
    {
        error = url;
        error += ": unexpected HTTP status ";
        error += std::to_string(response);
    }
    else if (result != CURLE_OK)
    {
        const char *e = curl_easy_strerror(result);

        error = "Fetch failed: ";
        error += e;
    }
    else
        ret = 0;

    if (ret)
        GfLogError("%s\n", error.c_str());

    return ret;
}

int transfer::exec(CURLcode result, std::string &error) const
{
    return check(result, error) || f(result, h, s, error, args);
}

void transfer::flush() const
{
    s->flush();
}

int transfer::write(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    sink *s = static_cast<sink *>(userdata);
    size_t total = size * nmemb;

    if (size != 0 && total / size != nmemb)
    {
        GfLogError("size calculation wrapped around\n");
        return !size;
    }
    else if (s->append(ptr, total))
    {
        GfLogError("append failed\n");
        return !size;
    }

    return total;
}

int transfer::start(const char *url, long max)
{
    CURLcode code;
    CURL *h = curl_easy_init();

    if (!h)
    {
        GfLogError("curl_easy_init failed\n");
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
    else if ((code = curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 0L))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt follow location: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt writefunction: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_MAXFILESIZE, max)))
    {
        GfLogError("curl_easy_setopt maxfilesize: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_FAILONERROR, 1L)))
    {
        GfLogError("curl_easy_setopt failonerror: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_WRITEDATA,
        static_cast<void *>(s))) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt writedata: %s\n",
            curl_easy_strerror(code));
        goto failure;
    }

    this->h = h;
    return 0;

failure:
    curl_easy_cleanup(h);
    return -1;
}

CURL *transfer::handle() const
{
    return h;
}

transfer::~transfer()
{
    curl_easy_cleanup(h);
    delete s;
}

transfer::transfer(sink *s, cb f, void *args) :
    h(nullptr),
    s(s),
    f(f),
    args(args)
{
}
