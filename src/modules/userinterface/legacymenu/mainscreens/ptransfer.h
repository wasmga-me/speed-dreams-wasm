/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef PTRANSFER_H
#define PTRANSFER_H

#include "transferbase.h"
#include <curl/curl.h>
#include <string>
#include <vector>

class ptransfer : public transferbase
{
public:
    typedef int (*cb)(CURLcode, CURL *, std::string &, void *,
        const transferbase::headers &headers);
    struct form
    {
        std::string key, value;
    };

    ptransfer(cb f, void *args);
    ~ptransfer();

    int start(const char *url, const char *path,
        const transferbase::headers &headers = transferbase::headers());
    int start(const char *url, const std::vector<form> &forms,
        const transferbase::headers &headers = transferbase::headers());
    int exec(CURLcode result, std::string &error) const;

private:
    const cb f;
    void *const args;
    struct curl_slist *headers;
    struct curl_slist *get_headers(const transferbase::headers &headers) const;
};

#endif
