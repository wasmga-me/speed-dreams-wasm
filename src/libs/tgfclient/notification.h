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

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <tgfclient.h>
#include <map>
#include <queue>
#include <string>

class TGFCLIENT_API NotificationManager
{
public:
    NotificationManager();
    int updateStatus();
    void set(const std::string &msg);
    void push(const std::string &msg);
    void release(void *screen);
    void clear();

    enum
    {
        Idle,
        Sending,
        Receiving
    } status;

private:
    int init();
    int wait();
    int show();
    int rest();
    int hide();
    int update_ui();
    int setup_ui(void *screen);
    void reset();

    struct Ids
    {
        int	bg, icons[3];
        int labels[8];
    };

    enum
    {
        Init,
        Wait,
        Show,
        Rest,
        Hide
    } ui_status;

    static const char *path;
    std::queue<std::string> msgs;
    std::map<void *, Ids> scr_ids;
    double t, x, w, icon_x, lbl_x;
};

#endif
