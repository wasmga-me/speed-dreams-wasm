/***************************************************************************

    file        : MuFactors.cpp
    created     : 29 Apr 2017
    copyright   : (C) 2017 D.Schellhammer

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MuFactors.h"
//#include "MyParam.h"

#include <fstream>
#include <tgf.h>

extern GfLogger* PLogAXIOM;
#define LogAXIOM (*PLogAXIOM)

MuFactors::MuFactors()
{
}

MuFactors::~MuFactors()
{
}


void MuFactors::read(const MyParam &param, tdble temp)
{
    mSect.clear();
    MuFactorsSect sect;
    double mufactor8;
    double mufactor15;
    double mufactor20;
    double mufactor25;
    double brakemufactor8;
    double brakemufactor15;
    double brakemufactor20;
    double brakemufactor25;

    char path[256];
    int i;

    for (i = 0; i < 200; i++)
    {
        snprintf(path, sizeof(path), "%i%s", i+1, " from");
        sect.fromstart = (double)param.getNum("private", path);
        LogAXIOM.debug("Path = %s - Value = %.2f\n", path, sect.fromstart);
        snprintf(path, sizeof(path), "%i%s", i+1, " mufactor");
        sect.mufactor = (double)param.getNum("private", path);
        snprintf(path, sizeof(path), "%i%s", i+1, " mufactor 8");
        mufactor8 = (double)param.getNum("private", path);
        snprintf(path, sizeof(path), "%i%s", i+1, " mufactor 15");
        mufactor15 = (double)param.getNum("private", path);
        snprintf(path, sizeof(path), "%i%s", i+1, " mufactor 20");
        mufactor20 = (double)param.getNum("private", path);
        snprintf(path, sizeof(path), "%i%s", i+1, " mufactor 25");
        mufactor25 = (double)param.getNum("private", path);
        snprintf(path, sizeof(path), "%i%s", i+1, " brakemufactor");
        sect.brakemufactor = (double)param.getNum("private", path);
        snprintf(path, sizeof(path), "%i%s", i+1, " brakemufactor 8");
        brakemufactor8 = (double)param.getNum("private", path);
        snprintf(path, sizeof(path), "%i%s", i+1, " brakemufactor 15");
        brakemufactor15 = (double)param.getNum("private", path);
        snprintf(path, sizeof(path), "%i%s", i+1, " brakemufactor 20");
        brakemufactor20 = (double)param.getNum("private", path);
        snprintf(path, sizeof(path), "%i%s", i+1, " brakemufactor 25");
        brakemufactor25 = (double)param.getNum("private", path);


        if (mufactor8 == 0)
            mufactor8 = sect.mufactor;

        if (mufactor15 == 0)
            mufactor15 = sect.mufactor;

        if (mufactor20 == 0)
            mufactor20 = sect.mufactor;

        if (mufactor25 == 0)
            mufactor25 = sect.mufactor;

        if (brakemufactor8 == 0)
            brakemufactor8 = sect.brakemufactor;

        if (brakemufactor15 == 0)
            brakemufactor15 = sect.brakemufactor;

        if (brakemufactor20 == 0)
            brakemufactor20 = sect.brakemufactor;

        if (brakemufactor25 == 0)
            brakemufactor25 = sect.brakemufactor;

        if(temp <= 8.0)
        {
            sect.mufactor = mufactor8;
            sect.brakemufactor = brakemufactor8;
        }
        else  if (temp <= 15.0)
        {
            sect.mufactor = mufactor15;
            sect.brakemufactor = brakemufactor15;
        }
        else  if (temp <= 20.0)
        {
            sect.mufactor = mufactor20;
            sect.brakemufactor = brakemufactor20;
        }
        else  if (temp <= 25.0)
        {
            sect.mufactor = mufactor25;
            sect.brakemufactor = brakemufactor25;
        }

        if(sect.mufactor > 0)
            mSect.push_back(sect);
        else
            break;
    }

    if(i > 0)
    {
        mMinMuFactor = calcMinMuFactor();
        mMinBrakeMuFactor = calcMinBrakeMuFactor();
        printMuFactors();
    }
    else
    {
        sect.fromstart = 0.0;
        sect.mufactor = 1.0;
        sect.brakemufactor = 0.85;
        mSect.push_back(sect);
        mMinMuFactor = 1.0;
        mMinBrakeMuFactor = 1.0;
        printMuFactors();
        LogAXIOM.debug("Couldn't load using default factor\n");
    }
}

double MuFactors::muFactor(double fromstart) const
{
    return mSect[sectIdx(fromstart)].mufactor;
}

double MuFactors::brakemuFactor(double fromstart) const
{
    return mSect[sectIdx(fromstart)].brakemufactor;
}

int MuFactors::sectIdx(double fromstart) const
{
    int idx = -1;

    for (int i = 0; i < (int)mSect.size() - 1; i++)
    {
        if (fromstart > mSect[i].fromstart && fromstart < mSect[i+1].fromstart)
        {
            idx = i;
        }
    }

    if (idx == -1)
    {
        idx = (int)mSect.size() - 1;
    }

    return idx;
}

double MuFactors::calcMinMuFactor() const
{
    double minfactor = 1000.0;

    for (int i = 0; i < (int)mSect.size(); i++)
    {
        if (mSect[i].mufactor < minfactor)
        {
            minfactor = mSect[i].mufactor;
        }
    }

    return minfactor;
}

double MuFactors::calcMinBrakeMuFactor() const
{
    double brakeminfactor = 1000.0;

    for (int i = 0; i < (int)mSect.size(); i++)
    {
        if (mSect[i].brakemufactor < brakeminfactor)
        {
            brakeminfactor = mSect[i].brakemufactor;
        }
    }

    return brakeminfactor;
}

void MuFactors::printMuFactors() const
{
    LogAXIOM.debug("Mu factors:\n");

    for (size_t i = 0; i < mSect.size(); i++)
    {
        LogAXIOM.debug("fs %f mu factor %f - brake mu factor %f\n",
                      mSect[i].fromstart, mSect[i].mufactor, mSect[i].brakemufactor);
    }
}
