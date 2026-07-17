/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2015 MadBad (madbad82@gmail.com)
 * Copyright (C) 2026 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "notification.h"
#include <guimenu.h>
#include <tgfclient.h>
#include <tgf.h>
#include <sstream>

const char *NotificationManager::path = "notifications.xml";

static std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim))
        elems.push_back(item);

    return elems;
}

void NotificationManager::reset()
{
    x = -w;
    ui_status = Wait;
    t = GfTimeClock();
}

void NotificationManager::clear()
{
    size_t n = msgs.size();

    for (size_t i = 0; i < n; i++)
        msgs.pop();

    reset();
}

void NotificationManager::release(void *screen)
{
    auto it = scr_ids.find(screen);

    if (it == scr_ids.end())
        return;

    scr_ids.erase(it);
}

void NotificationManager::set(const std::string &msg)
{
    size_t n = msgs.size();

    for (size_t i = 0; i < n; i++)
        msgs.pop();

    push(msg);
    t = GfTimeClock();
}

void NotificationManager::push(const std::string &msg)
{
    msgs.push(msg);
}

int NotificationManager::init()
{
    static const char slidebg[] = GFMNU_SECT_DYNAMIC_CONTROLS "/slidebg",
        text[] = GFMNU_SECT_TEMPLATE_CONTROLS "/text",
        icon[] = GFMNU_SECT_DYNAMIC_CONTROLS "/busyicon0";
    void *h = GfuiMenuLoad(path);

    if (!h)
    {
        GfLogError("GfuiMenuLoad %s failed\n", path);
        return -1;
    }

    w = GfParmGetNum(h, slidebg, "width", NULL, 0);
    x = GfParmGetNum(h, slidebg, "x", NULL, 0);
    icon_x = GfParmGetNum(h, icon, "x", NULL, 0);
    lbl_x = GfParmGetNum(h, text, "x", NULL, 0) - x;
    GfParmReleaseHandle(h);
    ui_status = Wait;
    return 0;
}

int NotificationManager::setup_ui(void *screen)
{
    static const char section[] = GFMNU_SECT_TEMPLATE_CONTROLS "/text";
    struct Ids ids;
    int y, h, font;
    void *param = GfuiMenuLoad(path);

    if (!param)
    {
        GfLogError("GfuiMenuLoad %s failed\n", path);
        return -1;
    }

    ids.bg = GfuiMenuCreateStaticImageControl(screen, param, "slidebg");

    if (ids.bg < 0)
    {
        GfLogError("GfuiMenuCreateStaticImageControl failed\n");
        return -1;
    }

    for (size_t i = 0; i < sizeof ids.icons / sizeof *ids.icons; i++)
    {
        std::string s = "busyicon" + std::to_string(i);
        int id = GfuiMenuCreateStaticImageControl(screen, param, s.c_str());

        if (id < 0)
        {
            GfLogError("GfuiMenuCreateStaticImageControl %zu failed\n", i);
            return -1;
        }

        ids.icons[i] = id;
    }

    font = gfuiMenuGetFontId(GfParmGetStr(param, section, "font", ""));
    y = GfParmGetNum(param, section, "y", NULL, 0);
    h = GfuiFontHeight(font);

    for (int &label : ids.labels)
    {
        int id = GfuiMenuCreateLabelControl(screen, param, "text", true,
            GFUI_TPL_TEXT, GFUI_TPL_X, y);

        if (id < 0)
        {
            GfLogError("GfuiMenuCreateLabelControl failed\n");
            return -1;
        }

        label = id;
        y -= h;
    }

    scr_ids[screen] = ids;
    return 0;
}

int NotificationManager::update_ui()
{
    std::vector<std::string> tokens;
    void *screen = GfuiGetScreen();

    if (!screen)
        return 0;
    else if (scr_ids.find(screen) == scr_ids.end() && setup_ui(screen))
        return -1;

    const struct Ids &ids = scr_ids.at(screen);

    if (ui_status == Wait)
    {
        GfuiVisibilitySet(screen, ids.bg, GFUI_INVISIBLE);

        for (int id : ids.icons)
            GfuiVisibilitySet(screen, id, GFUI_INVISIBLE);

        for (int id : ids.labels)
            GfuiVisibilitySet(screen, id, GFUI_INVISIBLE);

        return 0;
    }

    GfuiVisibilitySet(screen, ids.bg, GFUI_VISIBLE);

    for (size_t i = 0; i < sizeof ids.icons / sizeof *ids.icons; i++)
    {
        int visible = status == (int)i ? GFUI_VISIBLE : GFUI_INVISIBLE,
            id = ids.icons[i];

        GfuiSetPosition(screen, id, x + icon_x);
        GfuiVisibilitySet(screen, id, visible);
    }

    tokens = split(msgs.front(), '\n');

    for (size_t i = 0; i < sizeof ids.labels / sizeof *ids.labels; i++)
    {
        const char *text = i < tokens.size() ? tokens.at(i).c_str() : "";
        int id = ids.labels[i];

        GfuiLabelSetText(screen, id, text);
        GfuiSetPosition(screen, id, x + lbl_x);
        GfuiVisibilitySet(screen, id, GFUI_VISIBLE);
    }

    GfuiSetPosition(screen, ids.bg, x);
    return 0;
}

int NotificationManager::wait()
{
    std::string msg;

    if (!msgs.empty())
    {
        t = GfTimeClock();
        ui_status = Show;
    }

    return update_ui();
}

int NotificationManager::show()
{
    const double rate = 300.0;
    double now = GfTimeClock(), x = (now - t) * rate;

    if ((this->x += x) >= 0)
    {
        this->x = 0;
        ui_status = Rest;
    }

    t = now;
    return update_ui();
}

int NotificationManager::rest()
{
    double now = GfTimeClock();

    if (now - t >= 4)
    {
        t = now;
        ui_status = Hide;
    }

    return update_ui();
}

int NotificationManager::hide()
{
    const double rate = 300.0;
    double now = GfTimeClock(), x = (now - t) * rate;

    if ((this->x -= x) <= -w)
    {
        msgs.pop();
        reset();
    }

    t = now;
    return update_ui();
}

int NotificationManager::updateStatus()
{
    switch (ui_status)
    {
        case Init:
            return init();
        case Wait:
            return wait();
        case Show:
            return show();
        case Rest:
            return rest();
        case Hide:
            return hide();
    }

    GfLogError("updateStatus(): invalid status %d", ui_status);
    return -1;
}

NotificationManager::NotificationManager() :
    status(Idle),
    ui_status(Init)
{
}
