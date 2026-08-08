/***************************************************************************

    file        : MuFactors.h
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

#ifndef _MUFACTORS_H_
#define _MUFACTORS_H_

#include "MuFactorsSect.h"
#include "MyParam.h"
#include "track.h"

#include <vector>

class MuFactors
{
public:
    MuFactors();
    void read(const MyParam& param, tdble temp);
    double muFactor(double fromstart) const;
    double minMuFactor() const { return mMinMuFactor; }
    double brakemuFactor(double fromstart) const;
    double minBrakeMuFactor() const { return mMinBrakeMuFactor; }

    virtual ~MuFactors();

private:
    int sectIdx(double fromstart) const ;
    double calcMinMuFactor() const;
    double calcMinBrakeMuFactor() const;
    void printMuFactors() const;

    std::vector<MuFactorsSect> mSect;
    double mMinMuFactor;
    double mMinBrakeMuFactor;
};

#endif // _MUFACTORS_H_
