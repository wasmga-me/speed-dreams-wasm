/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef TRANSLATABLE_H
#define TRANSLATABLE_H

class Translatable
{
protected:
    Translatable(void *hscr, void *param);
    ~Translatable();
    const char *tr(const char *key) const;
    void *const hscr, *const param;
};

#endif
