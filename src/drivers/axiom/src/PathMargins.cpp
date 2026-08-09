/***************************************************************************

    file        : PathMargins.cpp
    created     : 17 Dec 2018
    copyright   : (C) 2018 D.Schellhammer

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "PathMargins.h"

#include <fstream>
#include <tgf.h>

extern GfLogger* PLogAXIOM;
#define LogAXIOM (*PLogAXIOM)

PathMargins::PathMargins()
{
}

PathMargins::~PathMargins()
{
}

void PathMargins::read(const MyParam &params)
{
    PathMarginsSect sect;

    char path[256];
    int i;

    for (i = 0; i < 200; i++)
    {
        snprintf(path, sizeof(path), "%i%s", i+1, " from margins");
        sect.fromstart = (double)params.getNum("private", path);
        LogAXIOM.debug("Path = %s - Value = %.2f\n", path, sect.fromstart);
        snprintf(path, sizeof(path), "%i%s", i+1, " left");
        sect.left = (double)params.getNum("private", path);
        LogAXIOM.debug("Path = %s - Value = %.2f\n", path, sect.left);
        snprintf(path, sizeof(path), "%i%s", i+1, " right");
        sect.right = (double)params.getNum("private", path);
        LogAXIOM.debug("Path = %s - Value = %.2f\n", path, sect.right);

        if(sect.left > 0 && sect.right > 0)
            mPathMargin.push_back(sect);
        else
            break;
    }

    if(i == 0)
    {
        sect.fromstart = 0.0;
        sect.left = 1.2;
        sect.right = 1.2;
        mPathMargin.push_back(sect);
        LogAXIOM.debug("Couldn't load, using default margins\n");
    }
}

PathMarginsSect PathMargins::margins(double fromstart) const
{
    return mPathMargin[sectIdx(fromstart)];
}

int PathMargins::sectIdx(double fromstart) const
{
    int idx = -1;

    for (int i = 0; i < (int)mPathMargin.size() - 1; i++)
    {
        if (fromstart >= mPathMargin[i].fromstart && fromstart < mPathMargin[i + 1].fromstart)
        {
            idx = i;
        }
    }

    if (idx == -1)
    {
        idx = (int)mPathMargin.size() - 1;
    }

    return idx;
}
