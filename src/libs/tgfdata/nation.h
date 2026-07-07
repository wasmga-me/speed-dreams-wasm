/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef NATION_H
#define NATION_H

#include <tgfdata.h>
#include <string>

TGFDATA_API std::string NationPath(const char *nation, const char *dir = "4x3png");

#endif
