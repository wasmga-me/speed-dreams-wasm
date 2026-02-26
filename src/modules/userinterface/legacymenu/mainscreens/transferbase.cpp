/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "transferbase.h"
#include <tgf.h>
#include <curl/curl.h>
#include <string>

int transferbase::check(CURLcode result, std::string &error) const
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
    else if (result != CURLE_OK && result != CURLE_HTTP_RETURNED_ERROR)
    {
        const char *e = curl_easy_strerror(result);

        error = url;
        error += ": ";
        error += e;
    }
    else if (response != 200)
    {
        error = url;
        error += ": unexpected HTTP status ";
        error += std::to_string(response);
    }
    else
        ret = 0;

    if (ret)
        GfLogError("%s\n", error.c_str());

    return ret;
}

CURL *transferbase::handle() const
{
    return h;
}

transferbase::~transferbase()
{
    curl_easy_cleanup(h);
}

int transferbase::exec(CURLcode result, std::string &error) const
{
    return 0;
}

transferbase::transferbase() :
    h(nullptr)
{
}
