/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2025 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef TRANSFER_H
#define TRANSFER_H

#include <curl/curl.h>
#include "sink.h"
#include <string>

class transfer
{
public:
    typedef int (*cb)(CURLcode, CURL *, const sink *, std::string &, void *);
    transfer(sink *s, cb f, void *args);
    ~transfer();

    int start(const char *url, long max);
    int exec(CURLcode result, std::string &error) const;
    void flush() const;
    CURL *handle() const;

private:
    static int write(char *ptr, size_t size, size_t nmemb, void *userdata);
    int check(CURLcode result, std::string &error) const;

    CURL *h;
    sink *const s;
    const cb f;
    void *const args;
};

#endif
