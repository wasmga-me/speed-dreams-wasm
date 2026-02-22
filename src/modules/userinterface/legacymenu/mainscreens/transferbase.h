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
#include <string>

class transferbase
{
public:
    transferbase();
    virtual int exec(CURLcode result, std::string &error) const;
    virtual ~transferbase();
    CURL *handle() const;

protected:
    int check(CURLcode result, std::string &error) const;
    CURL *h;
};

#endif
