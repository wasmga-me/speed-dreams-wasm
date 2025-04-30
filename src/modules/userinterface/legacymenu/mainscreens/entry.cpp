/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2024 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "entry.h"
#include "asset.h"
#include <tgf.h>
#include <tgfclient.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

int entry::trypng(const std::string &path)
{
    std::ifstream f(path, std::ios::binary);
    static const unsigned char header[] = {0x89, 'P', 'N', 'G'};
    char buf[sizeof header] = {0};

    f.read(buf, sizeof buf);
    return memcmp(buf, header, sizeof buf);
}

int entry::tryjpeg(const std::string &path)
{
    std::ifstream f(path, std::ios::binary);
    static const unsigned char header[] = {0xff, 0xd8, 0xff};
    char buf[sizeof header] = {0};

    f.read(buf, sizeof buf);
    return memcmp(buf, header, sizeof buf);
}

int entry::get(const std::string &path, enum thumbnail_type &type)
{
    static const struct ttype
    {
        enum thumbnail_type type;
        int (*fn)(const std::string &path);
    } f[] =
    {
        {thumbnail_type::png, trypng},
        {thumbnail_type::jpeg, tryjpeg}
    };

    for (size_t i = 0; i < sizeof f / sizeof *f; i++)
    {
        const struct ttype &t = f[i];

        if (!t.fn(path))
        {
            type = t.type;
            return 0;
        }
    }

    GfLogError("Unable to determine image type for %s\n", path.c_str());
    return -1;
}

int entry::rename()
{
    enum thumbnail_type type;

    if (get(thumbnail, type))
    {
        GfLogError("Failed to get thumbnail type\n");
        return -1;
    }

    static const char *const ext[] = {"jpg", "png"};
    std::string path = thumbnail + "." + ext[type];
    const char *old = thumbnail.c_str(), *newp = path.c_str();

    if (::rename(old, newp))
    {
        GfLogError("rename(3) %s -> %s failed: %s\n", old, newp,
            strerror(errno));
        return -1;
    }

    thumbnail = path;
    return 0;
}

int entry::on_thumbnail()
{
    bool update;

    if (rename())
    {
        GfLogError("Failed to rename thumbnail\n");
        return -1;
    }
    else if (a.needs_update(update))
        state = entry::download;
    else if (update)
        state = entry::update;
    else
        state = entry::done;

    thumbnail_fetched = true;
    return 0;
}

entry::entry(const Asset &a) :
    a(a),
    state(init),
    thumbnail_fetched(false),
    progress(0.0f)
{
}

entry::~entry()
{
    if (!data.empty() && remove(data.c_str()))
        GfLogError("remove(3) %s: %s\n", data.c_str(), strerror(errno));

    if (!thumbnail.empty() && remove(thumbnail.c_str()))
        GfLogError("remove(3) %s: %s\n", thumbnail.c_str(), strerror(errno));
}
