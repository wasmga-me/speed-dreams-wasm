/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Anri Dellal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <string>
#include <vector>

int servers_set();
extern void *ServerConfigInit(void *prevMenu);
#endif