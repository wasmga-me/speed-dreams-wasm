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
#include <tgf.h>

int transfer::exec(CURLcode result, std::string &error) const
{
    s->flush();
    return check(result, error) || f(result, h, s, error, args);
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

int transfer::start(const char *url, long max, const headers &headers)
{
    CURLcode code;

    if (transferbase::start(url, headers))
    {
        GfLogError("transferbase::start failed\n");
        return -1;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write))
        != CURLE_OK)
    {
        GfLogError("curl_easy_setopt writefunction: %s\n",
            curl_easy_strerror(code));
        return -1;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_MAXFILESIZE, max)))
    {
        GfLogError("curl_easy_setopt maxfilesize: %s\n",
            curl_easy_strerror(code));
        return -1;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_FAILONERROR, 1L)))
    {
        GfLogError("curl_easy_setopt failonerror: %s\n",
            curl_easy_strerror(code));
        return -1;
    }
    else if ((code = curl_easy_setopt(h, CURLOPT_WRITEDATA,
        static_cast<void *>(s))) != CURLE_OK)
    {
        GfLogError("curl_easy_setopt writedata: %s\n",
            curl_easy_strerror(code));
        return -1;
    }

    return 0;
}

transfer::~transfer()
{
    delete s;
}

transfer::transfer(sink *s, cb f, void *args) :
    transferbase(),
    s(s),
    f(f),
    args(args)
{
}
