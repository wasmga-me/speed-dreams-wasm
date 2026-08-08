/***************************************************************************

    file                 : Wheels.h
    created              : 2017-03-21 07:39:00 UTC
    copyright            : (C) Daniel Schellhammer
                           (C) Andrew Sumner
                           (C) Xavier Bertaux

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _WHEELS_H_
#define _WHEELS_H_

#include <car.h>

class Wheels
{
public:
    Wheels();
    void      init(const tCarElt* car, double muFactor, double length, int rain);
    void      update();
    double    TyreConditionFront() const;
    double    TyreConditionRear() const;
    double    TyreCondition() const;
    double    TyreTreadDepthFront() const;
    double    TyreTreadDepthRear() const;
    double    TyreTreadDepth() const;
    double    frictionBalanceLR() const;
    double    gripFactor() const { return mGripFactor; }
    double    distLeft() const { return mDistLeft; }
    double    wear() const { return mWear; }
    double    avgWearPerMeter() const { return mAvgWearPerMeter; }

    virtual   ~Wheels();

private:
    double updateGripFactor() const;

    const tCarElt*    mCar;
    double            mGripFactor;
    double            mDistWhenNew;
    double            mDistLeft;
    double            mWear;
    double            mAvgWearPerMeter;
    double            mAvgGrain;
    double            mMuScale;
    double            mMuScaleCold;
    double            mTrackLength;
};

#endif // _WHEELS_H_
