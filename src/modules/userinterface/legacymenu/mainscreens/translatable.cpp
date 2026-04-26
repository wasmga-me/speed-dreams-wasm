/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "translatable.h"
#include <tgfclient.h>
#include <translation.h>
#include <stdexcept>
#include <string>

const char *Translatable::tr(const char *key) const
{
    std::string s = std::string("translatable/") + key;

    return GfuiGetTranslatedText(param, s.c_str(), "text", "");
}

Translatable::~Translatable()
{
    GfParmReleaseHandle(param);
    GfuiScreenRelease(hscr);
}

Translatable::Translatable(void *hscr, void *param) :
    hscr(hscr),
    param(param)
{
    if (!hscr)
        throw std::runtime_error("Invalid screen handle");
    else if (!param)
        throw std::runtime_error("Invalid params handle");
}
