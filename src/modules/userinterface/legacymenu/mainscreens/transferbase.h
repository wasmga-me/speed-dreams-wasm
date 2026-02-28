/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef TRANSFERBASE_H
#define TRANSFERBASE_H

#include <curl/curl.h>
#include <cstddef>
#include <string>
#include <vector>

class transferbase
{
public:
    struct header
    {
        header() = default;
        header(const std::string &field, const std::string & value) :
            field(field), value(value) {}
        std::string field, value;
    };

    typedef std::vector<header> headers;
    transferbase();
    virtual int exec(CURLcode result, std::string &error) const;
    virtual ~transferbase();
    CURL *handle() const;

protected:
    static size_t on_header(char *buffer, size_t size, size_t nitems,
        void *userdata);
    headers resp_headers;
    CURL *h;
    int start(const char *url, const headers &headers);
    int check(CURLcode result, std::string &error) const;

private:
    struct curl_slist *hlist;
    int get_headers(const transferbase::headers &headers,
        struct curl_slist **out) const;
};

#endif
