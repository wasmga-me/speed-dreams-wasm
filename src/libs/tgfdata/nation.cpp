/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "nation.h"
#include <cctype>
#include <cstring>

std::string NationPath(const char *nation, const char *dir)
{
    std::string nationname;

    for (size_t i = 0; i < strlen(nation); i++)
        nationname += std::tolower(nation[i]);
    std::string uk = "uk";
    if (nationname == uk){
        nationname = "gb";
    }
    return "data/img/flags/" + std::string(dir) + '/' + nationname + ".png";
}
